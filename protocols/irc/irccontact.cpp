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

#include "ircaccount.h"

#include "ksparser.h"
#include "kopetemetacontact.h"
#include "ircusercontact.h"
#include "irccontact.h"
#include "ircprotocol.h"

struct whoIsInfo
{
	QString userName;
	QString hostName;
	QString realName;
	QString serverName;
	QString serverInfo;
	QStringList channels;
	unsigned long idle;
	bool isOperator;
};

IRCContact::IRCContact(IRCAccount *account, const QString &nick, KopeteMetaContact *metac, const QString& icon ) : KopeteContact(account, nick, metac, icon )
{
	mAccount = account;
	mEngine = mAccount->engine();
	mMetaContact = metac;
	mMsgManager = 0L;
	mNickName = nick;

	// Contact list display name
	setDisplayName(mNickName);

	// KopeteMessageManagerFactory stuff
	mMyself.append( static_cast<KopeteContact*>( this ) );

	QObject::connect(mEngine, SIGNAL(incomingAction(const QString &, const QString &, const QString &)), this, SLOT(slotNewAction(const QString &, const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingMessage(const QString &, const QString &, const QString &)), this, SLOT(slotNewMessage(const QString &, const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingWhoIsUser(const QString &, const QString &, const QString &, const QString &)) ,
		this, SLOT( slotNewWhoIsUser(const QString &, const QString &, const QString &, const QString &) ) );

	QObject::connect(mEngine, SIGNAL(incomingWhoIsServer(const QString &, const QString &, const QString &)), this, SLOT(slotNewWhoIsServer(const QString &, const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingWhoIsOperator(const QString &)), this, SLOT(slotNewWhoIsOperator(const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingWhoIsIdle(const QString &, unsigned long )), this, SLOT(slotNewWhoIsIdle(const QString &, unsigned long )));
	QObject::connect(mEngine, SIGNAL(incomingWhoIsChannels(const QString &, const QString &)), this, SLOT(slotNewWhoIsChannels(const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingEndOfWhois(const QString &)), this, SLOT( slotWhoIsComplete(const QString &)));
	QObject::connect(mEngine, SIGNAL(incomingNickChange(const QString &, const QString &)), this, SLOT( slotNewNickChange(const QString&, const QString&)));
	QObject::connect(mEngine, SIGNAL(incomingQuitIRC(const QString &, const QString &)), this, SLOT( slotUserDisconnected(const QString&, const QString&)));
	QObject::connect(mEngine, SIGNAL(incomingCtcpReply(const QString &, const QString &, const QString &)), this, SLOT( slotNewCtcpReply(const QString&, const QString &, const QString &)));
	QObject::connect(mEngine, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()) );
	isConnected = false;
}

bool IRCContact::isReachable()
{
	if ( onlineStatus().status() != KopeteOnlineStatus::Offline && onlineStatus().status() != KopeteOnlineStatus::Unknown )
		return true;

	return false;
}

void IRCContact::slotConnectionClosed()
{
	//FIXME: why not reimplement this slot in every custom IRCContacts, looks bette (Olivier)
	if( inherits("IRCChannelContact") )
		setOnlineStatus( IRCProtocol::IRCChannelOffline() );
	else
		setOnlineStatus( IRCProtocol::IRCUserOffline() );
}

void IRCContact::slotUserDisconnected( const QString &user, const QString &reason)
{
	if( isConnected )
	{
		QString nickname = user.section('!', 0, 0);
		KopeteContact *c = locateUser( nickname );
		if ( c )
		{
			manager()->removeContact( c, i18n("Quit: \"%1\" ").arg(reason) );
			c->setOnlineStatus( IRCProtocol::IRCUserOffline() );
			mAccount->unregisterUser( nickname );
		}
	}
}

void IRCContact::slotNewMessage(const QString &originating, const QString &target, const QString &message)
{
	//kdDebug(14120) << k_funcinfo << "originating is " << originating << " target is " << target << endl;
	if ( isConnected && target.lower() == mNickName.lower() )
	{
		QString nickname = originating.section('!', 0, 0);
		KopeteContact *user = locateUser( nickname );
		if ( user )
		{
			KopeteMessage msg( user, manager()->members(), message, KopeteMessage::Inbound, KopeteMessage::PlainText, KopeteMessage::Chat );
			msg.setBody( mAccount->protocol()->parser()->parse( msg.escapedBody() ), KopeteMessage::RichText );
			manager()->appendMessage(msg);
		}
	}
}

void IRCContact::slotNewAction(const QString &originating, const QString &target, const QString &message)
{
	//kdDebug(14120) << k_funcinfo << "originating is " << originating << " target is " << target << endl;
	if ( isConnected && target.lower() == mNickName.lower())
	{
		QString nickname = originating.section('!', 0, 0);
		KopeteContact *user = locateUser( nickname );
		if ( user )
		{
			KopeteMessage msg( user, manager()->members(), message, KopeteMessage::Action, KopeteMessage::PlainText, KopeteMessage::Chat );
			manager()->appendMessage(msg);
		}
		else if( mAccount->mySelf()->nickName().lower() == originating.lower() )
		{
			KopeteMessage msg( (KopeteContact*)mAccount->mySelf(), manager()->members(), message, KopeteMessage::Action, KopeteMessage::PlainText, KopeteMessage::Chat );
			manager()->appendMessage(msg);
		}

	}
}

void IRCContact::slotNewWhoIsUser(const QString &nickname, const QString &username, const QString &hostname, const QString &realname)
{
	KopeteContact *user = locateUser( nickname );
	if( user )
	{
		kdDebug(14120) << k_funcinfo << endl;
		mWhoisMap[nickname] = new whoIsInfo;
		mWhoisMap[nickname]->isOperator = false;
		mWhoisMap[nickname]->userName = username;
		mWhoisMap[nickname]->hostName = hostname;
		mWhoisMap[nickname]->realName = realname;
	}
}

void IRCContact::slotNewWhoIsServer(const QString &nickname, const QString &servername, const QString &serverinfo)
{
	if( isConnected && mWhoisMap.contains(nickname) )
	{
		mWhoisMap[nickname]->serverName = servername;
		mWhoisMap[nickname]->serverInfo = serverinfo;
	}
}

void IRCContact::slotNewWhoIsIdle(const QString &nickname, unsigned long idle)
{
	if( isConnected && mWhoisMap.contains(nickname) )
		mWhoisMap[nickname]->idle = idle;
}

void IRCContact::slotNewWhoIsOperator(const QString &nickname)
{
	if( isConnected && mWhoisMap.contains(nickname) )
		mWhoisMap[nickname]->isOperator = true;
}

void IRCContact::slotNewWhoIsChannels(const QString &nickname, const QString &channel)
{
	if( isConnected && mWhoisMap.contains(nickname) )
		mWhoisMap[nickname]->channels.append( channel );
}

void IRCContact::slotWhoIsComplete(const QString &nickname)
{
	if( isConnected && mWhoisMap.contains(nickname) )
	{
		whoIsInfo *w = mWhoisMap[nickname];

		//User info
		QString msg=i18n("%1 is (%2@%3): %4\n").arg(nickname).arg(w->userName).arg(w->hostName).arg(w->realName);
		if( w->isOperator )
			msg += i18n("%1 is an IRC operator\n").arg(nickname);

		//Channels
		msg += i18n("on channels %1\n").arg(w->channels.join(" ; "));

		//Server
		msg += i18n("on IRC via server %1 ( %2 )\n").arg(w->serverName).arg(w->serverInfo);

		//Idle
		msg += i18n("idle: %2\n").arg( QString::number(w->idle) );

		//End
		KopeteMessage m( locateUser(nickname), mMyself, msg, KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat );
		manager()->appendMessage(m);

		delete w;
		mWhoisMap.remove(nickname);
	}
}

void IRCContact::slotNewNickChange( const QString &oldnickname, const QString &newnickname)
{
	IRCContact *user = static_cast<IRCContact*>( locateUser( oldnickname) );
	if( user )
	{
		QString oldName;
		if( oldnickname == mAccount->mySelf()->nickName() )
			oldName = i18n("You are");
		else
			oldName = i18n("%1 is").arg(oldnickname);

		user->setNickName( newnickname );
		//If tracking name changes....
		user->setDisplayName( newnickname );

		//If the user is in our contact list, then change the notify list nickname
		if( !user->metaContact()->isTemporary() )
		{
			mEngine->removeFromNotifyList( oldnickname );
			mEngine->addToNotifyList( newnickname );
		}
	}
}

void IRCContact::slotNewCtcpReply(const QString &type, const QString &target, const QString &messageReceived)
{
	if( isConnected && locateUser( target ) )
	{
		KopeteMessage msg((KopeteContact *)this, mMyself, i18n("CTCP %1 REPLY: %2").arg(type).arg(messageReceived), KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat);
		manager()->appendMessage(msg);
	}
}

void IRCContact::slotSendMsg(KopeteMessage &message, KopeteMessageManager *)
{
	QString htmlString = message.escapedBody();
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
				kdDebug() << k_funcinfo << "Attribute:" << attribute << ", Value:" << value << endl;
				if( attribute == QString::fromLatin1("color") )
				{
					int ircColor = mAccount->protocol()->parser()->colorForHTML( value );
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

	htmlString.replace( QRegExp( QString::fromLatin1( "<p.*>(.*)</p>" ) ), QString::fromLatin1("\\1\n") );
	htmlString.replace( QRegExp( QString::fromLatin1( "<[^>]*>" ) ), QString::null );

	QStringList messages = QStringList::split( QRegExp( QString::fromLatin1("\n") ), KopeteMessage::unescape( htmlString ) );
	for(QStringList::Iterator it = messages.begin(); it != messages.end(); ++it)
		mEngine->messageContact(mNickName, *it );

	message.setBg( QColor() );
	message.setFg( QColor() );

	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

KopeteContact *IRCContact::locateUser( const QString &nick )
{
	//kdDebug(14120) << k_funcinfo << "Find nick " << nick << endl;
	if( isConnected )
	{
		if( nick == mAccount->mySelf()->nickName() )
			return mAccount->mySelf();
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

bool IRCContact::isChatting() const
{
	bool result=false;
	QIntDict<KopeteMessageManager> sessions = KopeteMessageManagerFactory::factory()->sessions();

	QIntDictIterator<KopeteMessageManager> it( sessions );
	for ( ; it.current() ; ++it )
	{
		result |= it.current()->members().contains(this);
	}
	return result;
}

#include "irccontact.moc"
