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

#include "kopetemetacontact.h"
#include "kopeteview.h"
#include "ircusercontact.h"
#include "irccontact.h"
#include "ircprotocol.h"
#include "ircservercontact.h"
#include "irccontactmanager.h"
#include "ksparser.h"

IRCContact::IRCContact(IRCContactManager *contactManager, const QString &nick, KopeteMetaContact *metac, const QString& icon)
	: KopeteContact(contactManager->account(), nick, metac, icon),
	  m_protocol(static_cast<IRCProtocol *>(protocol())),
	  m_account(contactManager->account()),
	  m_engine(contactManager->engine()),
	  m_metaContact(metac),
	  m_nickName(nick),
	  m_msgManager(0L),
	  m_isConnected(false)
{
	// Contact list display name
	setDisplayName(m_nickName);

	// IRCContactManager stuff
	QObject::connect(contactManager, SIGNAL(privateMessage(IRCContact *, IRCContact *, const QString &)),
			this, SLOT(privateMessage(IRCContact *, IRCContact *, const QString &)));

	QObject::connect(contactManager, SIGNAL(action(IRCContact *, IRCContact *, const QString &)),
			this, SLOT(action(IRCContact *, IRCContact *, const QString &)));

	// KopeteMessageManagerFactory stuff
	mMyself.append( static_cast<KopeteContact*>( this ) );

	// KIRC stuff
	QObject::connect(m_engine, SIGNAL(incomingNickChange(const QString &, const QString &)),
			this, SLOT( slotNewNickChange(const QString&, const QString&)));
	QObject::connect(m_engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)),
			this, SLOT(slotNewNickChange(const QString &, const QString &)));
	QObject::connect(m_engine, SIGNAL(incomingQuitIRC(const QString &, const QString &)),
			this, SLOT( slotUserDisconnected(const QString&, const QString&)));

	QObject::connect(m_engine, SIGNAL(statusChanged(KIRC::EngineStatus)),
			this, SLOT(updateStatus()));

	m_engine->setCodec( m_nickName, codec() );
}

IRCContact::~IRCContact()
{
//	kdDebug(14120) << k_funcinfo << mNickName << endl;
	if(metaContact() && metaContact()->isTemporary() && !isChatting())
		metaContact()->deleteLater();
}

bool IRCContact::isReachable()
{
	if ( onlineStatus().status() != KopeteOnlineStatus::Offline && onlineStatus().status() != KopeteOnlineStatus::Unknown )
		return true;

	return false;
}

void IRCContact::privateMessage(IRCContact *, IRCContact *, const QString &)
{
}

void IRCContact::action(IRCContact *, IRCContact *, const QString &)
{
}

void IRCContact::setCodec( const QTextCodec *codec )
{
	m_engine->setCodec( m_nickName, codec );
	metaContact()->setPluginData( m_protocol, QString::fromLatin1("Codec"), QString::number(codec->mibEnum()) );
}

const QTextCodec *IRCContact::codec()
{
	QString codecId = metaContact()->pluginData( m_protocol, QString::fromLatin1("Codec") );
	QTextCodec *codec = m_account->codec();

	if( !codecId.isEmpty() )
	{
		bool test = true;
		uint mib = codecId.toInt(&test);
		if( test )
			codec = QTextCodec::codecForMib( mib );
		else
			codec = QTextCodec::codecForName( codecId.latin1() );
	}

	if( !codec )
		return m_engine->codec();

	return codec;
}

KopeteMessageManager *IRCContact::manager(bool canCreate)
{
	if( canCreate && !m_msgManager )
	{
		if(m_engine->status() == KIRC::Disconnected)
			m_account->connect();

		m_msgManager = KopeteMessageManagerFactory::factory()->create( m_account->myself(), mMyself, m_account->protocol());
		m_msgManager->setDisplayName(caption());
		m_isConnected = true;
		QObject::connect( m_msgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)),
			this, SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));
		QObject::connect( m_msgManager, SIGNAL(closing(KopeteMessageManager*)),
			this, SLOT(messageManagerDestroyed()));

		QTimer::singleShot( 0, this, SLOT( initConversation() ) );
	}

	return m_msgManager;
}

void IRCContact::messageManagerDestroyed()
{
	m_msgManager = 0L;
	m_isConnected = false;

	if( metaContact()->isTemporary() && !isChatting() )
		deleteLater();
}

void IRCContact::slotUserDisconnected(const QString &user, const QString &reason)
{
	if( m_isConnected )
	{
		QString nickname = user.section('!', 0, 0);
		KopeteContact *c = locateUser( nickname );
		if ( c )
		{
			manager()->removeContact( c, i18n("Quit: \"%1\" ").arg(reason) );
			c->setOnlineStatus( m_protocol->m_UserStatusOffline );
		}
	}
}

void IRCContact::slotNewNickChange(const QString &oldnickname, const QString &newnickname)
{
	//kdDebug(14120) << k_funcinfo << oldnickname << " >> " << newnickname << ", " << m_nickName << endl;

	IRCContact *user = static_cast<IRCContact*>( locateUser(oldnickname) );
	if( user )
	{
		user->setNickName( newnickname );
		//If tracking name changes....
		user->setDisplayName( newnickname );

		//If the user is in our contact list, then change the notify list nickname
		if( !user->metaContact()->isTemporary() )
		{
			m_account->contactManager()->removeFromNotifyList( oldnickname );
			m_account->contactManager()->addToNotifyList( newnickname );
		}
	}
}

void IRCContact::slotSendMsg(KopeteMessage &message, KopeteMessageManager *)
{
	QString htmlString = message.escapedBody();

	if( htmlString.find( QString::fromLatin1("</span") ) > -1 )
	{
		QRegExp findTags( QString::fromLatin1("<span style=\"(.*)\">(.*)</span>") );
		findTags.setMinimal( true );
		int pos = 0;

		while ( pos >= 0 )
		{
			pos = findTags.search( htmlString );
			if ( pos > -1 )
			{
				QString styleHTML = findTags.cap(1);
				QString replacement = findTags.cap(2);
				QStringList styleAttrs = QStringList::split( ';', styleHTML );

				for( QStringList::Iterator attrPair = styleAttrs.begin(); attrPair != styleAttrs.end(); ++attrPair )
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

	htmlString = KopeteMessage::unescape( htmlString );

	if( htmlString.find( '\n' ) > -1 )
	{
		QStringList messages = QStringList::split( '\n', htmlString );

		for( QStringList::Iterator it = messages.begin(); it != messages.end(); ++it )
		{
			KopeteMessage msg(message.from(), message.to(), *it, message.direction(),
				KopeteMessage::RichText, message.type() );

			m_engine->messageContact(m_nickName, *it );

			msg.setBg( QColor() );
			msg.setFg( QColor() );

			appendMessage(msg);
			manager()->messageSucceeded();
		}
	}
	else
	{
		m_engine->messageContact(m_nickName, htmlString );

		message.setBg( QColor() );
		message.setFg( QColor() );

		appendMessage(message);
		manager()->messageSucceeded();
	}
}

KopeteContact *IRCContact::locateUser( const QString &nick )
{
	//kdDebug(14120) << k_funcinfo << "Find nick " << nick << endl;
	if( m_isConnected )
	{
		if( nick == m_account->mySelf()->nickName() )
			return m_account->mySelf();
		else
		{
			KopeteContactPtrList mMembers = manager()->members();
			for( KopeteContact *it = mMembers.first(); it; it = mMembers.next() )
			{
				if( static_cast<IRCContact*>(it)->nickName() == nick )
					return it;
			}
		}
	}

	return 0L;
}

bool IRCContact::isChatting( KopeteMessageManager *avoid ) const
{
	QIntDict<KopeteMessageManager> sessions = KopeteMessageManagerFactory::factory()->sessions();
	for ( QIntDictIterator<KopeteMessageManager> it( sessions ); it.current() ; ++it )
	{
		if( it.current() != avoid && it.current()->account() == m_account &&
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

	if( manager(false) )
		delete manager();

	if( !isChatting() )
	{
		kdDebug(14120) << k_funcinfo << "will delete " << m_nickName << endl;
		KopeteContact::slotDeleteContact();
	}
	else
	{
		metaContact()->removeContact( this );
		KopeteMetaContact *m = new KopeteMetaContact();
		m->setTemporary( true );
		setMetaContact( m );
	}
}

void IRCContact::appendMessage( KopeteMessage &msg )
{
	manager()->appendMessage(msg);
}

KopeteView *IRCContact::view()
{
	if( m_msgManager )
		return manager()->view(false);
	return 0L;
}

#include "irccontact.moc"
