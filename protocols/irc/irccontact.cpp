/*
    irccontact.cpp - IRC Contact

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <klocale.h>
#include <qregexp.h>

#include <qtimer.h>
#include <qtextcodec.h>

#include "ircaccount.h"
#include "kopeteglobal.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
#include "kopeteview.h"
#include "ircusercontact.h"
#include "irccontact.h"
#include "ircprotocol.h"
#include "ircservercontact.h"
#include "irccontactmanager.h"
#include "ksparser.h"

IRCContact::IRCContact(IRCContactManager *contactManager, const QString &nick, Kopete::MetaContact *metac, const QString& icon)
	: Kopete::Contact(contactManager->account(), nick, metac, icon),
	  m_nickName(nick),
	  m_msgManager(0L)
{
	// Contact list display name
	setProperty( Kopete::Global::Properties::self()->nickName(), m_nickName );

	// IRCContactManager stuff
	QObject::connect(contactManager, SIGNAL(privateMessage(IRCContact *, IRCContact *, const QString &)),
			this, SLOT(privateMessage(IRCContact *, IRCContact *, const QString &)));

	// Kopete::MessageManagerFactory stuff
	mMyself.append( static_cast<Kopete::Contact*>( this ) );

	// KIRC stuff
	QObject::connect(MYACCOUNT->engine(), SIGNAL(incomingNickChange(const QString &, const QString &)),
			this, SLOT( slotNewNickChange(const QString&, const QString&)));
	QObject::connect(MYACCOUNT->engine(), SIGNAL(successfullyChangedNick(const QString &, const QString &)),
			this, SLOT(slotNewNickChange(const QString &, const QString &)));
	QObject::connect(MYACCOUNT->engine(), SIGNAL(incomingQuitIRC(const QString &, const QString &)),
			this, SLOT( slotUserDisconnected(const QString&, const QString&)));

	QObject::connect(MYACCOUNT->engine(), SIGNAL(statusChanged(KIRC::Engine::Status)),
			this, SLOT(updateStatus()));

	MYACCOUNT->engine()->setCodec( m_nickName, codec() );
}

IRCContact::~IRCContact()
{
//	kdDebug(14120) << k_funcinfo << mNickName << endl;
	if( metaContact() && metaContact()->isTemporary() && !isChatting(m_msgManager) )
		metaContact()->deleteLater();
}

bool IRCContact::isReachable()
{
	if ( onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown )
		return true;

	return false;
}

void IRCContact::privateMessage(IRCContact *, IRCContact *, const QString &)
{
}

void IRCContact::setCodec(const QTextCodec *codec)
{
	MYACCOUNT->engine()->setCodec(m_nickName, codec);
	metaContact()->setPluginData(m_protocol, QString::fromLatin1("Codec"), QString::number(codec->mibEnum()));
}

const QTextCodec *IRCContact::codec()
{
	QString codecId = metaContact()->pluginData(m_protocol, QString::fromLatin1("Codec"));
	QTextCodec *codec = MYACCOUNT->codec();

	if( !codecId.isEmpty() )
	{
		bool test = true;
		uint mib = codecId.toInt(&test);
		if (test)
			codec = QTextCodec::codecForMib(mib);
		else
			codec = QTextCodec::codecForName(codecId.latin1());
	}

	if( !codec )
		return MYACCOUNT->engine()->codec();

	return codec;
}

Kopete::MessageManager *IRCContact::manager(bool canCreate)
{
	if( canCreate && !m_msgManager )
	{
		if(MYACCOUNT->engine()->status() == KIRC::Engine::Disconnected)
			MYACCOUNT->connect();

		m_msgManager = Kopete::MessageManagerFactory::factory()->create(MYACCOUNT->myself(), mMyself, MYACCOUNT->protocol());
		m_msgManager->setDisplayName(caption());

		QObject::connect( m_msgManager, SIGNAL(messageSent(Kopete::Message&, Kopete::MessageManager *)),
			this, SLOT(slotSendMsg(Kopete::Message&, Kopete::MessageManager *)));
		QObject::connect( m_msgManager, SIGNAL(closing(Kopete::MessageManager*)),
			this, SLOT(messageManagerDestroyed()));

		QTimer::singleShot( 0, this, SLOT( initConversation() ) );
	}

	return m_msgManager;
}

void IRCContact::messageManagerDestroyed()
{
	m_msgManager = 0L;

	if( metaContact()->isTemporary() && !isChatting() )
		deleteLater();
}

void IRCContact::slotUserDisconnected(const QString &user, const QString &reason)
{
	if( manager(false) )
	{
		QString nickname = user.section('!', 0, 0);
		Kopete::Contact *c = locateUser( nickname );
		if ( c )
		{
			manager()->removeContact(c, i18n("Quit: \"%1\" ").arg(reason), Kopete::Message::RichText);
			c->setOnlineStatus( m_protocol->m_UserStatusOffline );
		}
	}
}

void IRCContact::setNickName( const QString &nickname )
{
	kdDebug(14120) << k_funcinfo << m_nickName << " changed to " << nickname << endl;
	m_nickName = nickname;
	setProperty( Kopete::Global::Properties::self()->nickName(), nickname);
}

void IRCContact::slotNewNickChange(const QString &oldnickname, const QString &newnickname)
{
	//kdDebug(14120) << k_funcinfo << oldnickname << " >> " << newnickname << ", " << m_nickName << endl;

	IRCContact *user = static_cast<IRCContact*>( locateUser(oldnickname) );
	if( user )
	{
		user->setNickName( newnickname );

		//If the user is in our contact list, then change the notify list nickname
		if( !user->metaContact()->isTemporary() )
		{
			MYACCOUNT->contactManager()->removeFromNotifyList( oldnickname );
			MYACCOUNT->contactManager()->addToNotifyList( newnickname );
		}
	}
}

void IRCContact::slotSendMsg(Kopete::Message &message, Kopete::MessageManager *)
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

				QRegExp rx( QString::fromLatin1("<span style=\"%1\">.*</span>" ).arg( styleHTML ) );
				rx.setMinimal( true );
				htmlString.replace( rx, replacement );
			}
		}
	}

	htmlString = Kopete::Message::unescape(htmlString);

	if (htmlString.find('\n') > -1)
	{
		QStringList messages = QStringList::split( '\n', htmlString );

		for( QStringList::Iterator it = messages.begin(); it != messages.end(); ++it )
		{
			Kopete::Message msg(message.from(), message.to(), *it, message.direction(),
				Kopete::Message::RichText, message.type());

			MYACCOUNT->engine()->messageContact(m_nickName, *it );

			msg.setBg(QColor());
			msg.setFg(QColor());

			appendMessage(msg);
			manager()->messageSucceeded();
		}
	}
	else
	{
		MYACCOUNT->engine()->messageContact(m_nickName, htmlString );

		message.setBg( QColor() );
		message.setFg( QColor() );

		appendMessage(message);
		manager()->messageSucceeded();
	}
}

Kopete::Contact *IRCContact::locateUser( const QString &nick )
{
	//kdDebug(14120) << k_funcinfo << "Find nick " << nick << endl;
	if( manager(false) )
	{
		if( nick == MYACCOUNT->mySelf()->nickName() )
			return MYACCOUNT->mySelf();
		else
		{
			Kopete::ContactPtrList mMembers = manager()->members();
			for (Kopete::Contact *it = mMembers.first(); it; it = mMembers.next())
			{
				if (static_cast<IRCContact*>(it)->nickName() == nick)
					return it;
			}
		}
	}
	return 0L;
}

bool IRCContact::isChatting(Kopete::MessageManager *avoid) const
{
	if (!MYACCOUNT)
		return false;

	QIntDict<Kopete::MessageManager> sessions = Kopete::MessageManagerFactory::factory()->sessions();
	for (QIntDictIterator<Kopete::MessageManager> it( sessions ); it.current() ; ++it)
	{
		if( it.current() != avoid && it.current()->account() == MYACCOUNT &&
			it.current()->members().contains(this) )
		{
			return true;
		}
	}
	return false;
}

void IRCContact::slotDeleteContact()
{
	kdDebug(14120) << k_funcinfo << m_nickName << endl;

	if (manager(false))
		delete manager();

	if (!isChatting())
	{
		kdDebug(14120) << k_funcinfo << "will delete " << m_nickName << endl;
		Kopete::Contact::slotDeleteContact();
	}
	else
	{
		metaContact()->removeContact(this);
		Kopete::MetaContact *m = new Kopete::MetaContact();
		m->setTemporary(true);
		setMetaContact(m);
	}
}

void IRCContact::appendMessage(Kopete::Message &msg)
{
	manager()->appendMessage(msg);
}

KopeteView *IRCContact::view()
{
	if (m_msgManager)
		return manager()->view(false);
	return 0L;
}
void IRCContact::serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData )
{
	// write the
	addressBookData[ protocol()->addressBookIndexField() ] = ( contactId() + QChar(0xE120) + account()->accountId() );
}

#include "irccontact.moc"
