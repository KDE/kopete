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
#include <qstringlist.h>
#include <qregexp.h>

#include "ircchannelcontact.h"
#include "ircidentity.h"

#include "kirc.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "irccontact.h"

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
		this, SLOT( slotNewWhois(const QString &, const QString &, const QString &, const QString &) ) );
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
			QString msgText = QString::fromLatin1("* ") + QString::fromLatin1(" ") + message;
			KopeteMessage msg( user, mContact, msgText, KopeteMessage::Inbound );
			manager()->appendMessage(msg);
		}
	}
}

void IRCContact::slotNewWhois(const QString &nickname, const QString &username, const QString &hostname, const QString &realname)
{
	kdDebug(14120) << k_funcinfo << endl;
	KopeteContact *user = locateUser( nickname );
	if( user )
	{
		QString msgText = QString::fromLatin1("[%1] (%2@%3) : %4").arg(nickname).arg(username).arg(hostname).arg(realname);
		KopeteMessage msg( user, mContact, msgText, KopeteMessage::Internal );
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

const QString &IRCContact::caption() const
{
	return mNickName;
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

#include "irccontact.moc"
