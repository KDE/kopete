/***************************************************************************
                          irccontact.cpp  -  description
                             -------------------
    begin                : Thu Feb 20 2003
    copyright            : (C) 2003 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <klocale.h>
#include <qstringlist.h>
#include <qregexp.h>

#include "ircchannelcontact.h"
#include "ircidentity.h"

#include "kirc.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "irccontact.h"

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

IRCContact::IRCContact(IRCIdentity *identity, const QString &nick, KopeteMetaContact *metac) :
	KopeteContact((KopeteProtocol *)identity->protocol(), nick, metac )
{
	mIdentity = identity;
	mEngine = mIdentity->engine();
	mMetaContact = metac;
	mMsgManager = 0L;
	mNickName = nick;

	// KopeteMessageManagerFactory stuff
	mContact.append((KopeteContact *)this);
	mMyself.append((KopeteContact *)identity->mySelf());

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
}

KopeteMessageManager* IRCContact::manager(bool)
{
	if (!mMsgManager)
	{
		kdDebug(14120) << k_funcinfo << "Creating new KMM" << endl;

		KopeteContactPtrList initialContact;
		initialContact.append((KopeteContact *)mIdentity->mySelf());
		mMsgManager = KopeteMessageManagerFactory::factory()->create( (KopeteContact *)mIdentity->mySelf(), initialContact, (KopeteProtocol *)mIdentity->protocol());
		mMsgManager->setDisplayName( caption() );
		QObject::connect( mMsgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)), this, SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));
		QObject::connect( mMsgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
		if( mEngine->isLoggedIn() )
			mEngine->joinChannel(mNickName);
	}
	return mMsgManager;
}

void IRCContact::slotMessageManagerDestroyed()
{
	emit( endSession() );
	mMsgManager = 0L;
}

bool IRCContact::processMessage( const KopeteMessage &msg )
{
	QStringList commandLine = QStringList::split( QRegExp( QString::fromLatin1("\\s+") ), msg.plainBody() );
	QString commandArgs = msg.plainBody().section( QRegExp( QString::fromLatin1("\\s+") ), 1 );

	if( commandLine.first().startsWith( QString::fromLatin1("/") ) )
	{
		QString command = commandLine.first().right( commandLine.first().length() - 1 );

		if( mEngine->isLoggedIn() )
		{
			// These commands only work when we are connected
			if( command == QString::fromLatin1("nick") && commandLine.count() > 1 )
				mIdentity->successfullyChangedNick( QString::null, *commandLine.at(1) );

			else if( command == QString::fromLatin1("me") && commandLine.count() > 1 )
				mEngine->actionContact( displayName(), commandArgs );
		}

		// A /command returns false to stop further processing
		return false;
	}

	return true;
}

void IRCContact::slotUserDisconnected( const QString &user, const QString &reason)
{
	QString nickname = user.section('!', 0, 0);
	if ( nickname.lower() == mEngine->nickName().lower() )
	{
		mMsgManager->setCanBeDeleted(true);
		setOnlineStatus( KopeteContact::Offline ); // We parted the channel, change status
	}
	else
	{
		KopeteContact *user = locateUser( nickname );
		if ( user )
		{
			manager()->removeContact( user );
			delete user;
		}
		KopeteMessage msg((KopeteContact *)this, mContact, i18n(QString("User %1 has quit (\"%2\")").arg(nickname).arg(reason)), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCContact::slotNewMessage(const QString &originating, const QString &target, const QString &message)
{
	//kdDebug(14120) << k_funcinfo << "originating is " << originating << " target is " << target << endl;
	if (target.lower() == mNickName.lower())
	{
		QString nickname = originating.section('!', 0, 0);
		KopeteContact *user = locateUser( nickname );
		if ( user )
		{
			KopeteMessage msg( user, mContact, message, KopeteMessage::Inbound );
			manager()->appendMessage(msg);
		}
	}
}

void IRCContact::slotNewAction(const QString &originating, const QString &target, const QString &message)
{
	//kdDebug(14120) << k_funcinfo << "originating is " << originating << " target is " << target << endl;
	if (target.lower() == mNickName.lower())
	{
		QString nickname = originating.section('!', 0, 0);
		KopeteContact *user = locateUser( nickname );
		if ( user || mIdentity->mySelf()->nickName().lower() == originating.lower() )
		{
			KopeteMessage msg( user, mContact, message, KopeteMessage::Action );
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
		mWhoisMap[nickname]->userName = username;
		mWhoisMap[nickname]->hostName = hostname;
		mWhoisMap[nickname]->realName = realname;
	}
}

void IRCContact::slotNewWhoIsServer(const QString &nickname, const QString &servername, const QString &serverinfo)
{
	if( mWhoisMap.contains(nickname) )
	{
		mWhoisMap[nickname]->serverName = servername;
		mWhoisMap[nickname]->serverInfo = serverinfo;
	}
}

void IRCContact::slotNewWhoIsIdle(const QString &nickname, unsigned long idle)
{
	if( mWhoisMap.contains(nickname) )
		mWhoisMap[nickname]->idle = idle;
}

void IRCContact::slotNewWhoIsOperator(const QString &nickname)
{
	if( mWhoisMap.contains(nickname) )
		mWhoisMap[nickname]->isOperator = true;
}

void IRCContact::slotNewWhoIsChannels(const QString &nickname, const QString &channel)
{
	if( mWhoisMap.contains(nickname) )
		mWhoisMap[nickname]->channels.append( channel );
}

void IRCContact::slotWhoIsComplete(const QString &nickname)
{
	if( mWhoisMap.contains(nickname) )
	{
		whoIsInfo *w = mWhoisMap[nickname];
		KopeteMessage msg;
		KopeteContact *c = locateUser( nickname );

		//User info
		msg = KopeteMessage( c, mContact, QString::fromLatin1("[%1] (%2@%3) : %4\n").arg(nickname).arg(w->userName).arg(w->hostName).arg(w->realName), KopeteMessage::Internal );
		manager()->appendMessage(msg);

		//Channels
		QString channelText;
		for(QStringList::Iterator it = w->channels.begin(); it != w->channels.end(); ++it)
			channelText += *it + QString::fromLatin1(" \n");
		msg = KopeteMessage( c, mContact, QString::fromLatin1("[%1] %2").arg(nickname).arg(channelText), KopeteMessage::Internal );
		manager()->appendMessage(msg);

		//Server
		msg = KopeteMessage( c, mContact, QString::fromLatin1("[%1] %2 : %3\n").arg(nickname).arg(w->serverName).arg(w->serverInfo), KopeteMessage::Internal );
		manager()->appendMessage(msg);

		//Idle
		msg = KopeteMessage( c, mContact, i18n("[%1] idle %2\n").arg(nickname).arg( QString::number(w->idle) ), KopeteMessage::Internal );
		manager()->appendMessage(msg);

		//End
		msg = KopeteMessage( c, mContact,  i18n("[%1] End of WHOIS list.").arg(nickname), KopeteMessage::Internal );
		manager()->appendMessage(msg);

		delete w;
	}
}

void IRCContact::slotNewNickChange( const QString &oldnickname, const QString &newnickname)
{
	IRCContact *user = static_cast<IRCContact*>( locateUser( oldnickname) );
	if( user )
	{
		user->setNickName( newnickname );
		//If we are tracking name changes...
		user->setDisplayName( newnickname );

		KopeteMessage msg((KopeteContact *)this, mContact, i18n("%1 is now known as %2").arg(oldnickname).arg(newnickname), KopeteMessage::Internal);
		manager()->appendMessage(msg);
	}
}

void IRCContact::slotSendMsg(KopeteMessage &message, KopeteMessageManager *)
{
	if( onlineStatus() != KopeteContact::Online )
		mEngine->joinChannel(mNickName);

	if( processMessage( message ) )
	{
		// If the above was false, there was a server command
		mEngine->messageContact(mNickName, message.plainBody());
		manager()->appendMessage(message);
	}

	manager()->messageSucceeded();
}

KopeteContact *IRCContact::locateUser( const QString &nick )
{
	kdDebug(14120) << k_funcinfo << "Find nick " << nick << endl;
	KopeteContactPtrList mMembers = manager()->members();

	for( KopeteContact *it = mMembers.first(); it; it = mMembers.next() )
	{
		if( static_cast<IRCContact*>(it)->nickName() == nick )
			return it;
	}

	return 0L;
}

const QString IRCContact::caption() const
{
	return mNickName;
}

#include "irccontact.moc"
