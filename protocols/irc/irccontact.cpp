/*
    irccontact.cpp - IRC Contact

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2004-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "ircaccount.h"
#include "ircprotocol.h"
#include "ksparser.h"

#include "kircclientsocket.h"
#include "kircentity.h"
#include "kircmessage.h"
#include "kircstdmessages.h"

#include "kopetechatsessionmanager.h"
#include "kopeteglobal.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
#include "kopeteview.h"

#include <kdebug.h>
#include <klocale.h>
#include <qregexp.h>

#include <qtimer.h>
#include <qtextcodec.h>

using namespace IRC;
using namespace Kopete;

class IRCContact::Private
{
public:
	KIrc::EntityPtr entity;

	QMap<ChatSessionType, ChatSession *> chatSessions;

//	QList<IRContact *> members;

//	QList<Kopete::Contact *> mMyself;
	Kopete::Message::MessageDirection execDir;

	QList<QAction *> actions;
	QList<QAction *> serverActions;
	QList<QAction *> channelActions;
	QList<QAction *> userActions;
};

IRCContact::IRCContact(IRCAccount *account, const KIrc::EntityPtr &entity, MetaContact *metac, const QString& icon)
	: Contact(account, entity->name(), metac, icon)
	, d (new IRCContact::Private)
{
	Q_ASSERT(entity);

	kDebug(14120) <<"Entity name: "<< entity->name();

	d->entity = entity;

	if (!metac)
	{
		metac = new MetaContact();
		metac->setTemporary(true);
		setMetaContact(metac);
	}

	KIrc::ClientSocket *client = kircClient();

	// ChatSessionManager stuff
//	mMyself.append( static_cast<Contact*>( this ) );

	// KIRC stuff
	connect(client, SIGNAL(connectionStateChanged(KIrc::Socket::ConnectionState)),
		this, SLOT(updateStatus()));
/*
	connect(entity, SIGNAL(updated()),
		this, SLOT(entityUpdated()));
*/
	entityUpdated();
}

IRCContact::~IRCContact()
{
//	kDebug(14120) << entity->name();
//	if (metaContact() && metaContact()->isTemporary() && !isChatting(m_chatSession))
//		metaContact()->deleteLater();

	emit destroyed(this);

	delete d;
}

void IRCContact::deleteContact()
{
//	delete m_chatSession; // qDeleteAll(d->chatSessions) ?

	if (!isChatting())
	{
		Contact::deleteContact();
	}
	else
	{
		metaContact()->removeContact(this);
		MetaContact *m = new MetaContact();
		m->setTemporary(true);
		setMetaContact(m);
	}
}

IRCAccount *IRCContact::ircAccount() const
{
	return static_cast<IRCAccount *>(account());
}

KIrc::ClientSocket *IRCContact::kircClient() const
{
	return ircAccount()->client();
}

KIrc::EntityPtr IRCContact::entity() const
{
	return d->entity;
}

void IRCContact::entityUpdated()
{
	Global::Properties *prop = Global::Properties::self();

	// Basic entity properties.
	setProperty(prop->nickName(), d->entity->name());
//	setProperty(???->serverName(), d->entity->server());
//	setProperty(???->type(), d->entity->type());

	// Server properties

	// Channel properties
//	setProperty(???->topic(), d->entity->topic());

	// Contact properties
/*
	// Update Icon properties
	switch(m_entity->type())
	{
//	case KIrc::Entity::Unknown: // Use default
	case KIrc::Entity::Server:
		setIcon("irc_server");
		break;
	case KIrc::Entity::Channel:
		setIcon("irc_channel");
		break;
//	case KIrc::Entity::Service: // Use default for now
//		setIcon("irc_service");
//		break;
	case KIrc::Entity::User:
		setIcon("irc_user");
		break;
	default:
//		setIcon("irc_unknown");
		setIcon(QString::null);	//krazy:exclude=nullstrassign for old broken gcc
		break;
	}
*/
	updateStatus();
}

QString IRCContact::caption() const
{
	QString caption = QString::fromLatin1("%1 @ %2")
		.arg(QString(d->entity->name())) // nickName
		.arg(QString(ircAccount()->networkName()));

	QString topic = d->entity->topic();
	if(!topic.isEmpty())
		caption.append( QString::fromLatin1(" - %1").arg(Kopete::Message::unescape(topic)) );

	return caption;
}

void IRCContact::updateStatus()
{
	//setOnlineStatus(Kopete::OnlineStatus::Online);
	//setOnlineStatus(IRCProtocol::self()->onlineStatusFor(d->entity));

	//TODO: temporary until the real status handling is resurrected
	if(account()->isConnected())
		setOnlineStatus(Kopete::OnlineStatus::Online);
	else
		setOnlineStatus(Kopete::OnlineStatus::Offline);
}

bool IRCContact::isReachable()
{
	if (onlineStatus().status() != OnlineStatus::Offline &&
		onlineStatus().status() != OnlineStatus::Unknown)
		return true;

	return false;
}

void IRCContact::setCodec(QTextCodec *codec)
{
	d->entity->setCodec(codec);
	if (codec)
		metaContact()->setPluginData(IRCProtocol::self(), QString::fromLatin1("Codec"), QString::number(codec->mibEnum()));
//	else
//		metaContact()->removePluginData(m_protocol, QString::fromLatin1("Codec"));
}

QTextCodec *IRCContact::codec()
{
	QString codecId = metaContact()->pluginData(IRCProtocol::self(), QString::fromLatin1("Codec"));
	QTextCodec *codec = ircAccount()->codec();

	if( !codecId.isEmpty() )
	{
		bool test = true;
		uint mib = codecId.toInt(&test);
		if (test)
			codec = QTextCodec::codecForMib(mib);
		else
			codec = QTextCodec::codecForName(codecId.toLatin1());
	}

#if 0
	// FIXME: use context defaultCodec
	if (!codec)
		return kircClient()->defaultCodec();
#endif

	return codec;
}

ChatSession *IRCContact::manager(CanCreateFlags create)
{
	return chatSession(IRC::SERVER, create);
}

ChatSession *IRCContact::chatSession(IRC::ChatSessionType type, CanCreateFlags create)
{
	IRCAccount *account = ircAccount();
	KIrc::ClientSocket *engine = kircClient();

	Kopete::ChatSession *chatSession = d->chatSessions.value(type);
	if (!chatSession)
	{
//		if (engine->status() == KIrc::ClientSocket::Idle && dynamic_cast<IRCServerContact*>(this) == 0)
//			account->connect();

		kDebug(14120)<<"creating new ChatSession";

		chatSession = ChatSessionManager::self()->create(account->myself(), ( Kopete::ContactPtrList()<<this ) , account->protocol());
		chatSession->setDisplayName(caption());

		connect(chatSession, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
			this, SLOT(slotSendMsg(Kopete::Message&,Kopete::ChatSession*)));
		connect(chatSession, SIGNAL(closing(ChatSession*)),
			this, SLOT(chatSessionDestroyed(ChatSession*)));

		d->chatSessions.insert(type, chatSession);
	}

	return chatSession;
}

void IRCContact::chatSessionDestroyed(ChatSession *chatSession)
{
//	m_chatSession = 0; // d->chatSessions.remove(chatSession);

	if (metaContact()->isTemporary() && !isChatting())
		deleteLater();
}

void IRCContact::slotSendMsg(Message &message, ChatSession *chatSession)
{
	QString htmlString = message.escapedBody();

	if (htmlString.indexOf(QString::fromLatin1("</span")) > -1)
	{
		QRegExp findTags( QString::fromLatin1("<span style=\"(.*)\">(.*)</span>") );
		findTags.setMinimal( true );
		int pos = 0;

		while (pos >= 0)
		{
			pos = findTags.indexIn(htmlString);
			if (pos > -1)
			{
				QString styleHTML = findTags.cap(1);
				QString replacement = findTags.cap(2);
				QStringList styleAttrs = styleHTML.split(';');

				for (QStringList::Iterator attrPair = styleAttrs.begin(); attrPair != styleAttrs.end(); ++attrPair)
				{
					QString attribute = (*attrPair).section(':',0,0);
					QString value = (*attrPair).section(':',1);

					if( attribute == QString::fromLatin1("color") )
					{
						int ircColor = KSParser::colorForHTML( value );
						if( ircColor > -1 )
							replacement.prepend( QString( QChar(0x03) ).append( QString::number(ircColor) ) ).append( QChar( 0x03 ) );
					}
					else if( attribute == QString::fromLatin1("font-weight") && value == QString::fromLatin1("600") )
						replacement.prepend( QChar(0x02) ).append( QChar(0x02) );
					else if( attribute == QString::fromLatin1("text-decoration")  && value == QString::fromLatin1("underline") )
						replacement.prepend( QChar(31) ).append( QChar(31) );
				}

				htmlString = htmlString.left( pos ) + replacement + htmlString.mid( pos + findTags.matchedLength() );
			}
		}
	}

	htmlString = Message::unescape(htmlString);

	if (htmlString.indexOf('\n') > -1)
	{
		QStringList messages = htmlString.trimmed().split('\n');

		for( QStringList::Iterator it = messages.begin(); it != messages.end(); ++it )
		{
#warning FIXME
			Message msg(message.from(), message.to());
			msg.setHtmlBody(Kopete::Message::escape(sendMessage(*it)));
			msg.setDirection(message.direction());
//			                    Message::RichText, CHAT_VIEW, message.type());

//			msg.setBackgroundColor(QColor());
//			msg.setForegroundColor(QColor());

			appendMessage(msg);
			chatSession->messageSucceeded();
		}
	}
	else
	{
		message.setHtmlBody(Kopete::Message::escape(sendMessage(htmlString)));

//		message.setBackgroundColor(QColor());
//		message.setForegroundColor(QColor());

		appendMessage(message);
		chatSession->messageSucceeded();
	}
}

QString IRCContact::sendMessage(const QString &msg)
{
	kDebug(14120)<<"sending "<<msg;

	QString newMessage = msg;
	uint trueLength = msg.length() + entity()->name().length() + 12;
	if( trueLength > 512 )
	{
		//TODO: tell them it is truncated
		kWarning() << "Message was to long (" << trueLength << "), it has been truncated to 512 characters";
		newMessage.truncate( 512 - ( entity()->name().length() + 12 ) );
	}

	if(entity()->type()==KIrc::Entity::Server) //if it is a server, send raw data
		ircAccount()->client()->writeMessage( KIrc::Message::fromLine(codec()->fromUnicode(newMessage)) );
	else
		ircAccount()->client()->writeMessage( KIrc::StdMessages::privmsg(codec()->fromUnicode(entity()->name()),codec()->fromUnicode(newMessage)) );

	return newMessage;
}

Contact *IRCContact::locateUser(const QString &nick)
{
/*
	IRCAccount *account = ircAccount();

	if (m_chatSession)
	{
		if( nick == account->mySelf()->nickName() )
			return account->mySelf();
		else
		{
			ContactPtrList mMembers = m_chatSession->members();
			for (Contact *it = mMembers.first(); it; it = mMembers.next())
			{
				if (static_cast<IRCContact*>(it)->nickName() == nick)
					return it;
			}
		}
	}
*/
	return 0;
}

bool IRCContact::isChatting(ChatSession *avoid) const
{
	IRCAccount *account = ircAccount();

	if (!account)
		return false;

	foreach (ChatSession *chatSession, ChatSessionManager::self()->sessions())
	{
/*
		if( chatSession != avoid &&
			chatSession->account() == account &&
			chatSession->members().contains(this) )
		{
			return true;
		}
*/
	}
	return false;
}

void IRCContact::appendMessage(Message &msg)
{
	manager(Contact::CanCreate)->appendMessage(msg);
}
/*
void IRCServerContact::appendMessage(Kopete::Message &msg)
{
	msg.setImportance( Kopete::Message::Low ); //to don't distrub the user

	if (m_chatSession && m_chatSession->view(false))
		m_chatSession->appendMessage(msg);
	else
		mMsgBuffer.append( msg );
}
*/

void IRCContact::serialize(QMap<QString, QString> & /*serializedData*/, QMap<QString, QString> &addressBookData)
{
	addressBookData[protocol()->addressBookIndexField()] = contactId() + QChar(0xE120) + account()->accountId();
}

