/*
    ircchannelcontact.cpp - IRC Channel Contact

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

#include "ircchannelcontact.h"
#include "ircusercontact.h"
#include "ircaccount.h"
#include "ircprotocol.h"

#include "kopeteview.h"
#include "kopetestdaction.h"

#include <kdebug.h>
#include <klineeditdlg.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <qtimer.h>

IRCChannelContact::IRCChannelContact(IRCAccount *account, const QString &channel, KopeteMetaContact *metac) :
		IRCContact( account, channel, metac, QString::fromLatin1( "irc_channel" ) )
{
	// Variable assignments
	mNickName = channel;

	// KIRC Engine stuff
	QObject::connect(account->engine(), SIGNAL(userJoinedChannel(const QString &, const QString &)), this, SLOT(slotUserJoinedChannel(const QString &, const QString &)));
	QObject::connect(account->engine(), SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), this, SLOT(slotUserPartedChannel(const QString &, const QString &, const QString &)));
	QObject::connect(account->engine(), SIGNAL(incomingNamesList(const QString &, const QStringList &)), this, SLOT(slotNamesList(const QString &, const QStringList &)));
	QObject::connect(account->engine(), SIGNAL(incomingExistingTopic(const QString &, const QString &)), this, SLOT( slotChannelTopic(const QString&, const QString &)));
	QObject::connect(account->engine(), SIGNAL(incomingTopicChange(const QString &, const QString &, const QString &)), this, SLOT( slotTopicChanged(const QString&,const QString&,const QString&)));
	QObject::connect(account->engine(), SIGNAL(incomingModeChange(const QString&, const QString&, const QString&)), this, SLOT(slotIncomingModeChange(const QString&,const QString&, const QString&)));
	QObject::connect(account->engine(), SIGNAL(incomingChannelMode(const QString&, const QString&, const QString&)), this, SLOT(slotIncomingChannelMode(const QString&,const QString&, const QString&)));
	QObject::connect(account->engine(), SIGNAL(connectedToServer()), this, SLOT(slotConnectedToServer()));

	isConnected = false;

	setOnlineStatus( IRCProtocol::IRCChannelOffline() );
}

IRCChannelContact::~IRCChannelContact()
{
}

KopeteMessageManager* IRCChannelContact::manager(bool)
{
	if( !mEngine->isLoggedIn() )
		mAccount->connect();

	if ( !mMsgManager && mEngine->isLoggedIn() )
	{
		mMsgManager = KopeteMessageManagerFactory::factory()->create( mAccount->myself(), mMyself, (KopeteProtocol *)mAccount->protocol());
		mMsgManager->setDisplayName( caption() );
		QObject::connect( mMsgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)), this, SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));
		QObject::connect( mMsgManager, SIGNAL(closing(KopeteMessageManager*)), this, SLOT(slotMessageManagerDestroyed()));
		isConnected = true;
		QObject::connect( KopeteMessageManagerFactory::factory(), SIGNAL(viewCreated(KopeteView*)),this, SLOT( slotJoinChannel(KopeteView*) ) );
	}
	return mMsgManager;
}

void IRCChannelContact::slotMessageManagerDestroyed()
{
	KopeteContactPtrList contacts = mMsgManager->members();
	for( KopeteContact *c = contacts.first(); c; c = contacts.next() )
		mAccount->unregisterUser( static_cast<IRCContact*>(c)->nickName() );

	mAccount->unregisterChannel( mNickName );
	slotPart();
	isConnected = false;
	mMsgManager = 0L;
}

void IRCChannelContact::slotJoinChannel( KopeteView *view )
{
	if( view->msgManager() == mMsgManager )
	{
		mEngine->joinChannel(mNickName);
		QObject::disconnect( KopeteMessageManagerFactory::factory(), SIGNAL(viewCreated(KopeteView*)),this, SLOT( slotJoinChannel(KopeteView*) ) );
	}
}

void IRCChannelContact::slotConnectedToServer()
{
	kdDebug(14120) << k_funcinfo << endl;
	setOnlineStatus( IRCProtocol::IRCChannelOnline() );
}

void IRCChannelContact::slotNamesList(const QString &channel, const QStringList &nicknames)
{
	if ( isConnected && channel.lower() == mNickName.lower() )
	{
		kdDebug(14120) << k_funcinfo << "Names List:" << channel << endl;

		mJoinedNicks += nicknames;
		if( mJoinedNicks.count() == nicknames.count() )
			slotAddNicknames();
	}
}

void IRCChannelContact::slotAddNicknames()
{
	if( !isConnected || mJoinedNicks.isEmpty() )
		return;

	QString nickToAdd = mJoinedNicks.front();
	mJoinedNicks.pop_front();

	if (nickToAdd.lower() != mNickName.lower())
	{
		QChar firstChar = nickToAdd[0];
		if( firstChar == '@' || firstChar == '+' )
			nickToAdd = nickToAdd.remove(0, 1);

		IRCContact *user = mAccount->findUser( nickToAdd );
		user->setOnlineStatus( IRCProtocol::IRCUserOnline() );

		if ( firstChar == '@' )
			manager()->setContactOnlineStatus( static_cast<KopeteContact*>(user), IRCProtocol::IRCUserOp() );
		else if( firstChar == '+')
			manager()->setContactOnlineStatus( static_cast<KopeteContact*>(user), IRCProtocol::IRCUserVoice() );

		manager()->addContact( static_cast<KopeteContact*>(user), true );
	}
	QTimer::singleShot(0, this, SLOT( slotAddNicknames() ) );
}

void IRCChannelContact::slotChannelTopic(const QString &channel, const QString &topic)
{
	if( isConnected && mNickName.lower() == channel.lower() )
	{
		mTopic = topic;
		manager()->setDisplayName( caption() );
		KopeteMessage msg((KopeteContact*)this, mMyself, i18n("Topic for %1 is %2").arg(mNickName).arg(mTopic), KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::slotJoin()
{
	if ( !isConnected && onlineStatus().status() == KopeteOnlineStatus::Online )
		execute();
}

void IRCChannelContact::slotPart()
{
	if( isConnected )
		mEngine->partChannel(mNickName, QString("Kopete %1 : http://kopete.kde.org").arg(kapp->aboutData()->version()) );
}

void IRCChannelContact::slotUserJoinedChannel(const QString &user, const QString &channel)
{
	if( isConnected && (channel.lower() == mNickName.lower()) )
	{
		QString nickname = user.section('!', 0, 0);
		if ( nickname.lower() == mEngine->nickName().lower() )
		{
			KopeteMessage msg((KopeteContact *)this, mMyself,
			i18n("You have joined channel %1").arg(mNickName), KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat);
			manager()->appendMessage(msg);
			while( !messageQueue.isEmpty() )
			{
				slotSendMsg( messageQueue.front(), manager() );
				messageQueue.pop_front();
			}
			setMode( QString::null );
		}
		else
		{
			IRCUserContact *contact = mAccount->findUser( nickname );
			contact->setOnlineStatus( IRCProtocol::IRCUserOnline() );
			manager()->addContact((KopeteContact *)contact, true);

			KopeteMessage msg((KopeteContact *)this, mMyself,
			i18n("User <b>%1</b> [%2] joined channel %3").arg(nickname).arg(user.section('!', 1)).arg(mNickName), KopeteMessage::Internal, KopeteMessage::RichText, KopeteMessage::Chat);
			manager()->appendMessage(msg);
		}
	}
}

void IRCChannelContact::slotUserPartedChannel(const QString &user, const QString &channel, const QString &reason)
{
	QString nickname = user.section('!', 0, 0);
	if ( isConnected && channel.lower() == mNickName.lower() && nickname.lower() != mEngine->nickName().lower() )
	{
		KopeteContact *c = locateUser( nickname );
		if ( c )
		{
			manager()->removeContact( c, true );
			mAccount->unregisterUser( nickname );
		}
		KopeteMessage msg((KopeteContact *)this, mMyself,
		i18n("User <b>%1</b> parted channel %2 (%3)").arg(nickname).arg(mNickName).arg(reason), KopeteMessage::Internal, KopeteMessage::RichText, KopeteMessage::Chat);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::setTopic( const QString &topic )
{
	if ( isConnected )
	{
		bool okPressed = true;
		QString newTopic = topic;
		if( newTopic.isNull() )
			newTopic = KLineEditDlg::getText( i18n("New Topic"), i18n("Enter the new topic:"), mTopic, &okPressed, 0L );

		if( okPressed )
		{
			mTopic = newTopic;
			mEngine->setTopic( mNickName, newTopic );
		}
	}
}

void IRCChannelContact::slotTopicChanged( const QString &channel, const QString &nick, const QString &newtopic )
{
	if( isConnected && mNickName.lower() == channel.lower() )
	{
		mTopic = newtopic;
		mMsgManager->setDisplayName( caption() );
		KopeteMessage msg((KopeteContact *)this, mMyself, i18n("%1 has changed the topic to %2").arg(nick).arg(newtopic), KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat);
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::slotIncomingModeChange( const QString &nick, const QString &channel, const QString &mode )
{
	if( isConnected && mNickName.lower() == channel.lower() )
	{
		KopeteMessage msg((KopeteContact *)this, mMyself, i18n("%1 sets mode %2 %3").arg(nick).arg(mode).arg(mNickName), KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat);
		manager()->appendMessage(msg);
		bool inParams = false;
		bool modeEnabled = false;
		QString params = QString::null;
		for( uint i=0; i < mode.length(); i++ )
		{
			switch( mode[i] )
			{
				case '+':
					modeEnabled = true;
					break;

				case '-':
					modeEnabled = false;
					break;

				case ' ':
					inParams = true;
					break;
				default:
					if( inParams )
						params.append( mode[i] );
					else
						toggleMode( mode[i], modeEnabled, false );
					break;
			}
		}
	}
}

void IRCChannelContact::slotIncomingChannelMode( const QString &channel, const QString &mode, const QString &params )
{
	if( isConnected && channel.lower() == mNickName.lower() )
	{
		kdDebug(14120) << k_funcinfo << channel << " : " << mode << " : " << params << endl;
		for( uint i=1; i < mode.length(); i++ )
		{
			if( mode[i] != 'l' && mode[i] != 'k' )
				toggleMode( mode[i], true, false );
		}
	}
}

void IRCChannelContact::setMode( const QString &mode )
{
	if( isConnected )
		mEngine->changeMode( mNickName, mode );
}

void IRCChannelContact::slotModeChanged()
{
	/*toggleMode( 't', actionModeT->isChecked(), true );
	toggleMode( 'n', actionModeN->isChecked(), true );
	toggleMode( 's', actionModeS->isChecked(), true );
	toggleMode( 'm', actionModeM->isChecked(), true );
	toggleMode( 'i', actionModeI->isChecked(), true );*/
}

void IRCChannelContact::toggleMode( QChar mode, bool enabled, bool update )
{
	if( isConnected )
	{
		/*switch( mode )
		{
			case 't':
   				actionModeT->setChecked( enabled );
				break;
			case 'n':
				actionModeN->setChecked( enabled );
				break;
			case 's':
				actionModeS->setChecked( enabled );
				break;
			case 'm':
				actionModeM->setChecked( enabled );
				break;
			case 'i':
				actionModeI->setChecked( enabled );
				break;
		}*/
	}

	if( update )
	{
		if( modeMap[mode] != enabled )
		{
			if( enabled )
				setMode( QString::fromLatin1("+") + mode );
			else
				setMode( QString::fromLatin1("-") + mode );
		}
	}

	modeMap[mode] = enabled;
}

bool IRCChannelContact::modeEnabled( QChar mode, QString *value )
{
	if( !value )
		return modeMap[mode];
	return false;
}

KActionCollection *IRCChannelContact::customContextMenuActions()
{
	// KAction stuff
	mCustomActions = new KActionCollection(this);
	actionJoin = new KAction(i18n("&Join"), 0, this, SLOT(slotJoin()), mCustomActions, "actionJoin");
	actionPart = new KAction(i18n("&Part"), 0, this, SLOT(slotPart()), mCustomActions, "actionPart");
	actionTopic = new KAction(i18n("Change &Topic..."), 0, this, SLOT(setTopic()), mCustomActions, "actionTopic");
	actionModeMenu = new KActionMenu(i18n("Channel Modes"), 0, mCustomActions, "actionModeMenu");

	actionModeT = new KToggleAction(i18n("Only Operators Can Change &Topic"), 0, this, SLOT(slotModeChanged()), actionModeMenu );
	actionModeN = new KToggleAction(i18n("&No Outside Messages"), 0, this, SLOT(slotModeChanged()), actionModeMenu );
	actionModeS = new KToggleAction(i18n("&Secret"), 0, this, SLOT(slotModeChanged()), actionModeMenu );
	actionModeM = new KToggleAction(i18n("&Moderated"), 0, this, SLOT(slotModeChanged()), actionModeMenu );
	actionModeI = new KToggleAction(i18n("&Invite Only"), 0, this, SLOT(slotModeChanged()), actionModeMenu );

	actionModeMenu->insert( actionModeT );
	actionModeMenu->insert( actionModeN );
	actionModeMenu->insert( actionModeS );
	actionModeMenu->insert( actionModeM );
	actionModeMenu->insert( actionModeI );
	actionModeMenu->setEnabled( true );

	bool isOperator = isConnected && ( manager()->contactOnlineStatus( mAccount->mySelf() ) == IRCProtocol::IRCUserOp() );

	actionJoin->setEnabled( !isConnected );
	actionPart->setEnabled( isConnected );

	actionTopic->setEnabled( isConnected && ( !modeEnabled('t') || isOperator ) );

	actionModeT->setEnabled(isOperator);
	actionModeN->setEnabled(isOperator);
	actionModeS->setEnabled(isOperator);
	actionModeM->setEnabled(isOperator);
	actionModeI->setEnabled(isOperator);

	return mCustomActions;
}

const QString IRCChannelContact::caption() const
{
	QString cap;
	if ( isConnected )
	{
		cap = QString::fromLatin1("%1 @ %2").arg(mNickName).arg(mEngine->host());
		if( !mTopic.isNull() && !mTopic.isEmpty() )
			cap.append( QString::fromLatin1(" - %1").arg(mTopic) );
	}
	else
		cap = QString::null;

	return cap;
}

#include "ircchannelcontact.moc"
