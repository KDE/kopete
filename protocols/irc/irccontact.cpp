/*
    irccontact.cpp - IRC Contact

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2004-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "irccontact.h"

#include "ircaccount.h"
#include "ircprotocol.h"
#include "ksparser.h"

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

using namespace Kopete;

IRCContact::IRCContact(IRCAccount *account, KIRC::EntityPtr entity, MetaContact *metac, const QString& icon)
	: Contact(account, entity->name(), metac, icon),
	  m_entity(entity),
	  m_chatSession(0)
{
	kdDebug(14120) << k_funcinfo << entity->name() << endl;

	if (!metac)
	{
		metac = new MetaContact();
		metac->setTemporary(true);
		setMetaContact(metac);
	}

	KIRC::Engine *engine = kircEngine();

	// ChatSessionManager stuff
	mMyself.append( static_cast<Contact*>( this ) );

	// KIRC stuff

	QObject::connect(engine, SIGNAL(connectionStateChanged(KIRC::ConnectionState)),
			this, SLOT(updateStatus()));

	QObject::connect(m_entity, SIGNAL(updated()),
			this, SLOT(entityUpdated()));

	entityUpdated();
}

IRCContact::~IRCContact()
{
//	kdDebug(14120) << k_funcinfo << m_nickName << endl;
	if (metaContact() && metaContact()->isTemporary() && !isChatting(m_chatSession))
		metaContact()->deleteLater();

	emit destroyed(this);
}

void IRCContact::deleteContact()
{
	delete m_chatSession;

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

KIRC::Engine *IRCContact::kircEngine() const
{
	return ircAccount()->engine();
}

void IRCContact::entityUpdated()
{
	Global::Properties *prop = Global::Properties::self();

	// Basic entity properties.
	setProperty(prop->nickName(), m_entity->name());
//	setProperty(???->serverName(), m_entity->server());
//	setProperty(???->type(), m_entity->type());

	// Server properties

	// Channel properties
//	setProperty(???->topic(), m_entity->topic());

	// Contact properties
/*
	// Update Icon properties
	switch(m_entity->type())
	{
//	case KIRC::Entity::Unknown: // Use default
	case KIRC::Entity::Server:
		setIcon("irc_server");
		break;
	case KIRC::Entity::Channel:
		setIcon("irc_channel");
		break;
//	case KIRC::Entity::Service: // Use default for now
//		setIcon("irc_service");
//		break;
	case KIRC::Entity::User:
		setIcon("irc_user");
		break;
	default:
//		setIcon("irc_unknown");
		setIcon(QString::null);
		break;
	}
*/
	updateStatus();
}

QString IRCContact::caption() const
{
	return QString::null;
}

void IRCContact::updateStatus()
{
	setOnlineStatus(IRCProtocol::self()->onlineStatusFor(m_entity->status()));
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
	m_entity->setCodec(codec);
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
			codec = QTextCodec::codecForName(codecId.latin1());
	}

	if (!codec)
		return kircEngine()->defaultCodec();

	return codec;
}

ChatSession *IRCContact::manager(Contact::CanCreateFlags canCreate)
{
	IRCAccount *account = ircAccount();
	KIRC::Engine *engine = kircEngine();

	if (canCreate == Contact::CanCreate && !m_chatSession)
	{
//		if (engine->status() == KIRC::Engine::Idle && dynamic_cast<IRCServerContact*>(this) == 0)
//			account->connect();

		m_chatSession = ChatSessionManager::self()->create(account->myself(), mMyself, account->protocol());
		m_chatSession->setDisplayName(caption());

		QObject::connect(m_chatSession, SIGNAL(messageSent(Message&, ChatSession *)),
			this, SLOT(slotSendMsg(Message&, ChatSession *)));
		QObject::connect(m_chatSession, SIGNAL(closing(ChatSession *)),
			this, SLOT(chatSessionDestroyed()));

		initConversation();
	}

	return m_chatSession;
}

void IRCContact::chatSessionDestroyed()
{
	m_chatSession = 0;

	if (metaContact()->isTemporary() && !isChatting())
		deleteLater();
}

void IRCContact::slotSendMsg(Message &message, ChatSession *)
{
	QString htmlString = message.escapedBody();

	if (htmlString.find(QString::fromLatin1("</span")) > -1)
	{
		QRegExp findTags( QString::fromLatin1("<span style=\"(.*)\">(.*)</span>") );
		findTags.setMinimal( true );
		int pos = 0;

		while (pos >= 0)
		{
			pos = findTags.search(htmlString);
			if (pos > -1)
			{
				QString styleHTML = findTags.cap(1);
				QString replacement = findTags.cap(2);
				QStringList styleAttrs = QStringList::split(';', styleHTML);

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

	if (htmlString.find('\n') > -1)
	{
		QStringList messages = QStringList::split( '\n', htmlString );

		for( QStringList::Iterator it = messages.begin(); it != messages.end(); ++it )
		{
			Message msg(message.from(), message.to(), Kopete::Message::escape(sendMessage(*it)), message.direction(),
			                    Message::RichText, CHAT_VIEW, message.type());

			msg.setBg(QColor());
			msg.setFg(QColor());

			appendMessage(msg);
			manager(Contact::CanCreate)->messageSucceeded();
		}
	}
	else
	{
		message.setBody( Kopete::Message::escape(sendMessage( htmlString )), Message::RichText );

		message.setBg( QColor() );
		message.setFg( QColor() );

		appendMessage(message);
		manager(Contact::CanCreate)->messageSucceeded();
	}
}

QString IRCContact::sendMessage(const QString &msg)
{
/*
	QString newMessage = msg;
	uint trueLength = msg.length() + m_nickName.length() + 12;
	if( trueLength > 512 )
	{
		//TODO: tell them it is truncated
		kdWarning() << "Message was to long (" << trueLength << "), it has been truncated to 512 characters" << endl;
		newMessage.truncate( 512 - ( m_nickName.length() + 12 ) );
	}

	kircEngine()->privmsg(m_nickName, newMessage );

	return newMessage;
*/
	return QString::null;
}

Contact *IRCContact::locateUser(const QString &nick)
{
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
	return 0;
}

bool IRCContact::isChatting(ChatSession *avoid) const
{
	IRCAccount *account = ircAccount();

	if (!account)
		return false;

	QValueList<ChatSession*> sessions = ChatSessionManager::self()->sessions();
	for (QValueList<ChatSession*>::Iterator it= sessions.begin(); it!=sessions.end() ; ++it)
	{
	  if( (*it) != avoid && (*it)->account() == account &&
			   (*it)->members().contains(this) )
		{
			return true;
		}
	}
	return false;
}

void IRCContact::appendMessage(Message &msg)
{
	manager(Contact::CanCreate)->appendMessage(msg);
}

KopeteView *IRCContact::view()
{
	if (m_chatSession)
		return m_chatSession->view(false);
	return 0L;
}

void IRCContact::serialize(QMap<QString, QString> & /*serializedData*/, QMap<QString, QString> &addressBookData)
{
	addressBookData[protocol()->addressBookIndexField()] = contactId() + QChar(0xE120) + account()->accountId();
}

#include "irccontact.moc"

