/*
    ircchannelcontact.cpp - IRC Channel Contact

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "irccontactmanager.h"
#include "ircchannelcontact.h"
#include "ircusercontact.h"
#include "ircservercontact.h"
#include "ircaccount.h"
#include "ircprotocol.h"

#include "kopeteview.h"
#include "kopeteuiglobal.h"
#include "kcodecaction.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "kopetemessagemanagerfactory.h"

#include <kdebug.h>
#include <krun.h>
#include <kinputdialog.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include <qtimer.h>

IRCChannelContact::IRCChannelContact(IRCContactManager *contactManager, const QString &channel, Kopete::MetaContact *metac)
	: IRCContact(contactManager, channel, metac, "irc_channel")
{
	mInfoTimer = new QTimer( this );
	QObject::connect(mInfoTimer, SIGNAL(timeout()), this, SLOT( slotUpdateInfo() ) );

	QObject::connect( MYACCOUNT->engine(), SIGNAL(incomingUserIsAway( const QString &, const QString & )),
		this, SLOT(slotIncomingUserIsAway(const QString &, const QString &)));

	QObject::connect( MYACCOUNT->engine(), SIGNAL( incomingListedChan( const QString &, uint, const QString & ) ),
		this, SLOT( slotChannelListed( const QString &, uint, const QString & ) ) );

	actionJoin = 0L;
	actionModeT = new KToggleAction(i18n("Only Operators Can Change &Topic"), 0, this, SLOT(slotModeChanged()), this );
	actionModeN = new KToggleAction(i18n("&No Outside Messages"), 0, this, SLOT(slotModeChanged()), this );
	actionModeS = new KToggleAction(i18n("&Secret"), 0, this, SLOT(slotModeChanged()), this );
	actionModeM = new KToggleAction(i18n("&Moderated"), 0, this, SLOT(slotModeChanged()), this );
	actionModeI = new KToggleAction(i18n("&Invite Only"), 0, this, SLOT(slotModeChanged()), this );
	actionHomePage = 0L;

	updateStatus();
	slotUpdateInfo();
}

IRCChannelContact::~IRCChannelContact()
{
}

void IRCChannelContact::slotUpdateInfo()
{
	/** This woudl be nice, but it generates server errors too often

	if( !manager(false) && onlineStatus() == m_protocol->m_ChannelStatusOnline )
		MYACCOUNT->engine()->writeMessage( QString::fromLatin1("LIST %1").arg(m_nickName) );
	else
		setProperty( QString::fromLatin1("channelMembers"), i18n("Members"), manager()->members().count() );

	*/

	if( manager(false) )
	{
		setProperty( m_protocol->propChannelMembers, manager()->members().count() );
		MYACCOUNT->engine()->writeMessage( QString::fromLatin1("WHO %1").arg(m_nickName) );
	}
	else
	{
		removeProperty( m_protocol->propChannelMembers );
		removeProperty( m_protocol->propChannelTopic );
	}

	mInfoTimer->start( 45000, true );
}

void IRCChannelContact::slotChannelListed( const QString &channel, uint members, const QString &topic )
{
	if( !manager(false) && onlineStatus() == m_protocol->m_ChannelStatusOnline
		&& channel.lower() == m_nickName.lower() )
	{
		mTopic = topic;
		setProperty( m_protocol->propChannelMembers, members );
		setProperty( m_protocol->propChannelTopic, topic );
	}
}

void IRCChannelContact::updateStatus()
{
	KIRC::Engine::Status status = MYACCOUNT->engine()->status();
	switch( status )
	{
		case KIRC::Engine::Disconnected:
		case KIRC::Engine::Connecting:
		case KIRC::Engine::Authentifying:
			setOnlineStatus(m_protocol->m_ChannelStatusOffline);
			break;
		case KIRC::Engine::Connected:
		case KIRC::Engine::Closing:
			setOnlineStatus(m_protocol->m_ChannelStatusOnline);
			break;
		default:
			setOnlineStatus(m_protocol->m_StatusUnknown);
	}
}

void IRCChannelContact::messageManagerDestroyed()
{
	if(manager(false))
	{
		part();
		Kopete::ContactPtrList contacts = manager()->members();

		// remove all the users on the channel
		for( Kopete::Contact *c = contacts.first(); c; c = contacts.next() )
		{
			if( c->metaContact()->isTemporary() && !static_cast<IRCContact*>(c)->isChatting( manager() ) )
				c->deleteLater();
		}
	}

	IRCContact::messageManagerDestroyed();
}

void IRCChannelContact::initConversation()
{
	MYACCOUNT->engine()->join(m_nickName, password());
}

void IRCChannelContact::slotConnectedToServer()
{
	setOnlineStatus( m_protocol->m_ChannelStatusOnline );
	if( manager(false) )
		MYACCOUNT->engine()->join(m_nickName, password());
}

void IRCChannelContact::namesList(const QStringList &nicknames)
{
	mJoinedNicks += nicknames;
	if( mJoinedNicks.count() == nicknames.count() )
		slotAddNicknames();
}

void IRCChannelContact::slotAddNicknames()
{
	if( !manager(false) || mJoinedNicks.isEmpty() )
	{
		slotUpdateInfo();
		setMode( QString::null );
		return;
	}

	QString nickToAdd = mJoinedNicks.front();
	QChar firstChar = nickToAdd[0];
	if( firstChar == '@' || firstChar == '+' )
		nickToAdd = nickToAdd.remove(0, 1);

	mJoinedNicks.pop_front();
	IRCContact *user;

	if ( nickToAdd.lower() != MYACCOUNT->mySelf()->nickName().lower() )
	{
		//kdDebug(14120) << k_funcinfo << m_nickName << " NICK: " << nickToAdd << endl;
		user = MYACCOUNT->contactManager()->findUser( nickToAdd );
		user->setOnlineStatus( m_protocol->m_UserStatusOnline );
		manager()->addContact( static_cast<Kopete::Contact*>(user) , true);
	}
	else
	{
	        user = MYACCOUNT->mySelf();
	}

	if ( firstChar == '@' || firstChar == '%' )
		manager()->setContactOnlineStatus( static_cast<Kopete::Contact*>(user), m_protocol->m_UserStatusOp );
	else if( firstChar == '+')
		manager()->setContactOnlineStatus( static_cast<Kopete::Contact*>(user), m_protocol->m_UserStatusVoice );

	QTimer::singleShot(0, this, SLOT( slotAddNicknames() ) );
}

void IRCChannelContact::channelTopic(const QString &topic)
{
	mTopic = topic;
	setProperty( m_protocol->propChannelTopic, mTopic );
	manager()->setDisplayName( caption() );
	Kopete::Message msg((Kopete::Contact*)this, mMyself, i18n("Topic for %1 is %2").arg(m_nickName).arg(mTopic),
	                    Kopete::Message::Internal, Kopete::Message::RichText, Kopete::Message::Chat);
	appendMessage(msg);
}

void IRCChannelContact::channelHomePage(const QString &url)
{
	kdDebug(14120) << k_funcinfo << endl;
	setProperty( m_protocol->propHomepage, url );
}

void IRCChannelContact::join()
{
	if ( !manager(false) && onlineStatus().status() == Kopete::OnlineStatus::Online )
	{
		kdDebug() << k_funcinfo << "My nickname:" << m_nickName << endl;
		kdDebug() << k_funcinfo << "My manager:" << manager(false) << endl;
		if( manager(false) )
			kdDebug() << k_funcinfo << "My view:" << manager(false)->view(false) << endl;
		startChat();
	}
}

void IRCChannelContact::part()
{
	if (manager(false))
		MYACCOUNT->engine()->part(m_nickName, MYACCOUNT->defaultPart());
}

void IRCChannelContact::slotIncomingUserIsAway( const QString &nick, const QString & )
{
	if( nick.lower() == MYACCOUNT->mySelf()->nickName().lower() )
	{
		IRCUserContact *c = MYACCOUNT->mySelf();
		if( manager(false) && manager()->members().contains( c ) )
		{
			Kopete::OnlineStatus status = manager()->contactOnlineStatus( c );
			if( status == m_protocol->m_UserStatusOp )
				manager()->setContactOnlineStatus( c, m_protocol->m_UserStatusOpAway );
			else if( status == m_protocol->m_UserStatusOpAway )
				manager()->setContactOnlineStatus( c, m_protocol->m_UserStatusOp );
			else if( status == m_protocol->m_UserStatusVoice )
				manager()->setContactOnlineStatus( c, m_protocol->m_UserStatusVoiceAway );
			else if( status == m_protocol->m_UserStatusVoiceAway )
				manager()->setContactOnlineStatus( c, m_protocol->m_UserStatusVoice );
			else if( status == m_protocol->m_UserStatusAway )
				manager()->setContactOnlineStatus( c, m_protocol->m_UserStatusOnline );
			else
				manager()->setContactOnlineStatus( c, m_protocol->m_UserStatusAway );
		}
	}
}

void IRCChannelContact::userJoinedChannel(const QString &nickname)
{
	if( nickname.lower() == MYACCOUNT->mySelf()->nickName().lower() )
	{
		kdDebug() << k_funcinfo << "Me:" << this << endl;
		kdDebug() << k_funcinfo << "My nickname:" << m_nickName << endl;
		kdDebug() << k_funcinfo << "My manager:" << manager(false) << endl;
		if( manager(false) )
			kdDebug() << k_funcinfo << "My view:" << manager(false)->view(false) << endl;

		Kopete::Message msg((Kopete::Contact *)this, mMyself,
			i18n("You have joined channel %1").arg(m_nickName),
			Kopete::Message::Internal, Kopete::Message::PlainText,
			Kopete::Message::Chat);
		msg.setImportance( Kopete::Message::Low); //set the importance manualy to low
		appendMessage(msg);
	}
	else
	{
		IRCUserContact *contact = MYACCOUNT->contactManager()->findUser( nickname );
		contact->setOnlineStatus( m_protocol->m_UserStatusOnline );
		manager()->addContact((Kopete::Contact *)contact, true);
		Kopete::Message msg((Kopete::Contact *)this, mMyself,
			i18n("User <b>%1</b> joined channel %2").arg(nickname).arg(m_nickName),
			Kopete::Message::Internal, Kopete::Message::RichText, Kopete::Message::Chat);
		msg.setImportance( Kopete::Message::Low); //set the importance manualy to low
		manager()->appendMessage(msg);
	}
}

void IRCChannelContact::userPartedChannel(const QString &nickname,const QString &reason)
{
	if ( nickname.lower() != MYACCOUNT->engine()->nickName().lower() )
	{
		Kopete::Contact *c = locateUser( nickname );
		if ( c )
		{
			manager()->removeContact( c, Kopete::Message::unescape(reason) );
			if( c->metaContact()->isTemporary() && !static_cast<IRCContact*>(c)->isChatting( manager(false) ) )
				c->deleteLater();
		}
	}
}

void IRCChannelContact::userKicked(const QString &nick, const QString &nickKicked, const QString &reason)
{
	QString r = i18n("Kicked by %1.").arg( nick );
	if( reason != nick )
		r.append( i18n(" Reason: %2").arg( reason ) );

	if( nickKicked.lower() != MYACCOUNT->engine()->nickName().lower() )
	{
		Kopete::Contact *c = locateUser( nickKicked );
		if ( c )
		{
			manager()->removeContact( c, r );
			Kopete::Message msg( (Kopete::Contact *)this, mMyself,
			                     r, Kopete::Message::Internal, Kopete::Message::PlainText, Kopete::Message::Chat);
			msg.setImportance(Kopete::Message::Low);
			appendMessage(msg);
			if( c->metaContact()->isTemporary() && !static_cast<IRCContact*>(c)->isChatting( manager(false) ) )
				c->deleteLater();
		}
	}
	else
	{
		KMessageBox::error(Kopete::UI::Global::mainWidget(), r, i18n("IRC Plugin"));
		manager()->view()->closeView();
	}
}

void IRCChannelContact::setTopic(const QString &topic)
{
	if ( manager(false) )
	{
		if( manager()->contactOnlineStatus( manager()->user() ) ==
			m_protocol->m_UserStatusOp || !modeEnabled('t') )
		{
			bool okPressed = true;
			QString newTopic = topic;
			if( newTopic.isNull() )
				newTopic = KInputDialog::getText( i18n("New Topic"), i18n("Enter the new topic:"),
					Kopete::Message::unescape(mTopic), &okPressed, 0L );

			if( okPressed )
			{
				mTopic = newTopic;
				MYACCOUNT->engine()->topic(m_nickName, newTopic);
			}
		}
		else
		{
			Kopete::Message msg(MYACCOUNT->myServer(), manager()->members(),
				i18n("You must be a channel operator on %1 to do that.").arg(m_nickName),
				Kopete::Message::Internal, Kopete::Message::PlainText, Kopete::Message::Chat);
			manager()->appendMessage(msg);
		}
	}
}

void IRCChannelContact::topicChanged(const QString &nick, const QString &newtopic)
{
	mTopic = newtopic;
	setProperty( m_protocol->propChannelTopic, mTopic );
	manager()->setDisplayName( caption() );
	Kopete::Message msg(MYACCOUNT->myServer(), mMyself,
		i18n("%1 has changed the topic to: %2").arg(nick).arg(newtopic),
		Kopete::Message::Internal, Kopete::Message::RichText, Kopete::Message::Chat);
	msg.setImportance(Kopete::Message::Low); //set the importance manualy to low
	appendMessage(msg);
}

void IRCChannelContact::topicUser(const QString &nick, const QDateTime &time)
{
	Kopete::Message msg( MYACCOUNT->myServer(), mMyself,
		i18n("Topic set by %1 at %2").arg(nick).arg(
			KGlobal::locale()->formatDateTime(time, true)
		), Kopete::Message::Internal, Kopete::Message::PlainText, Kopete::Message::Chat);
	msg.setImportance(Kopete::Message::Low); //set the importance manualy to low
	appendMessage(msg);
}

void IRCChannelContact::incomingModeChange( const QString &nick, const QString &mode )
{
	Kopete::Message msg((Kopete::Contact *)this, mMyself, i18n("%1 sets mode %2 on  %3").arg(nick).arg(mode).arg(m_nickName), Kopete::Message::Internal, Kopete::Message::PlainText, Kopete::Message::Chat);
	msg.setImportance( Kopete::Message::Low); //set the importance manualy to low
	appendMessage(msg);

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

void IRCChannelContact::incomingChannelMode( const QString &mode,
	const QString &/*params*/ )
{
	for( uint i=1; i < mode.length(); i++ )
	{
		if( mode[i] != 'l' && mode[i] != 'k' )
			toggleMode( mode[i], true, false );
	}
}

void IRCChannelContact::setMode(const QString &mode)
{
	if (manager(false))
		MYACCOUNT->engine()->mode(m_nickName, mode);
}

void IRCChannelContact::slotModeChanged()
{
	toggleMode( 't', actionModeT->isChecked(), true );
	toggleMode( 'n', actionModeN->isChecked(), true );
	toggleMode( 's', actionModeS->isChecked(), true );
	toggleMode( 'm', actionModeM->isChecked(), true );
	toggleMode( 'i', actionModeI->isChecked(), true );
}

void IRCChannelContact::failedChanBanned()
{
	manager()->deleteLater();
	KMessageBox::error( Kopete::UI::Global::mainWidget(),
		i18n("<qt>You can not join %1 because you have been banned.</qt>").arg(m_nickName),
		i18n("IRC Plugin") );
}

void IRCChannelContact::failedChanInvite()
{
	manager()->deleteLater();
	KMessageBox::error( Kopete::UI::Global::mainWidget(),
		i18n("<qt>You can not join %1 because it is set to invite only, and no one has invited you.</qt>").arg(m_nickName), i18n("IRC Plugin") );
}

void IRCChannelContact::failedChanFull()
{
	manager()->deleteLater();
	KMessageBox::error( Kopete::UI::Global::mainWidget(),
		i18n("<qt>You can not join %1 because it has reached its user limit.</qt>").arg(m_nickName),
		i18n("IRC Plugin") );
}

void IRCChannelContact::failedChankey()
{
	bool ok;
	QString diaPassword = KInputDialog::getText( i18n( "IRC Plugin" ),
		i18n( "Please enter key for channel %1: ").arg(m_nickName),
		QString::null,
		&ok );

	if ( !ok )
		manager()->deleteLater();
	else
	{
		setPassword(diaPassword);
		MYACCOUNT->engine()->join(m_nickName, password());
	}
}

void IRCChannelContact::toggleMode( QChar mode, bool enabled, bool update )
{
	if( manager(false) )
	{
		switch( mode )
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
		}
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

QPtrList<KAction> *IRCChannelContact::customContextMenuActions()
{
	// KAction stuff
	QPtrList<KAction> *mCustomActions = new QPtrList<KAction>();
	if( !actionJoin )
	{
		actionJoin = new KAction(i18n("&Join"), 0, this, SLOT(join()), this, "actionJoin");
		actionPart = new KAction(i18n("&Part"), 0, this, SLOT(part()), this, "actionPart");
		actionTopic = new KAction(i18n("Change &Topic..."), 0, this, SLOT(setTopic()), this, "actionTopic");
		actionModeMenu = new KActionMenu(i18n("Channel Modes"), 0, this, "actionModeMenu");

		if( !property(m_protocol->propHomepage).value().isNull() )
		{
			actionHomePage = new KAction( i18n("Visit &Homepage"), 0, this,
				SLOT(slotHomepage()), this, "actionHomepage");
		}
		else if( actionHomePage )
		{
			delete actionHomePage;
		}

		actionModeMenu->insert( actionModeT );
		actionModeMenu->insert( actionModeN );
		actionModeMenu->insert( actionModeS );
		actionModeMenu->insert( actionModeM );
		actionModeMenu->insert( actionModeI );
		actionModeMenu->setEnabled( true );

		codecAction = new KCodecAction( i18n("&Encoding"), 0, this, "selectcharset" );
		connect( codecAction, SIGNAL( activated( const QTextCodec * ) ),
			this, SLOT( setCodec( const QTextCodec *) ) );
		codecAction->setCodec( codec() );
	}

	mCustomActions->append( actionJoin );
	mCustomActions->append( actionPart );
	mCustomActions->append( actionTopic );
	mCustomActions->append( actionModeMenu );
	mCustomActions->append( codecAction );
	if( actionHomePage )
		mCustomActions->append( actionHomePage );

	bool isOperator = manager(false) && ( manager()->contactOnlineStatus( MYACCOUNT->myself() ) == m_protocol->m_UserStatusOp );

	actionJoin->setEnabled( !manager(false) );
	actionPart->setEnabled( manager(false) );
	actionTopic->setEnabled( manager(false) && ( !modeEnabled('t') || isOperator ) );

	actionModeT->setEnabled(isOperator);
	actionModeN->setEnabled(isOperator);
	actionModeS->setEnabled(isOperator);
	actionModeM->setEnabled(isOperator);
	actionModeI->setEnabled(isOperator);

	return mCustomActions;
}

void IRCChannelContact::slotHomepage()
{
	QString homePage = property(m_protocol->propHomepage).value().toString();
	if( !homePage.isEmpty() )
	{
	       new KRun( KURL( homePage ), 0, false);
	}
}

const QString IRCChannelContact::caption() const
{
	QString cap = QString::fromLatin1("%1 @ %2").arg(m_nickName).arg(MYACCOUNT->engine()->currentHost());
		if( !mTopic.isNull() && !mTopic.isEmpty() )
			cap.append( QString::fromLatin1(" - %1").arg(Kopete::Message::unescape(mTopic)) );

	return cap;
}

void IRCChannelContact::privateMessage(IRCContact *from, IRCContact *to, const QString &message)
{
	if(to == this)
	{
		Kopete::Message msg(from, manager()->members(), message, Kopete::Message::Inbound,
		                    Kopete::Message::RichText, Kopete::Message::Chat);
		appendMessage(msg);
	}
}

void IRCChannelContact::newAction(const QString &from, const QString &action)
{
	kdDebug(14120) << k_funcinfo << m_nickName << endl;

	IRCUserContact *f = MYACCOUNT->contactManager()->findUser(from);
	Kopete::Message::MessageDirection dir =
		(f == MYACCOUNT->mySelf()) ? Kopete::Message::Outbound : Kopete::Message::Inbound;
	Kopete::Message msg(f, manager()->members(), action, dir, Kopete::Message::RichText,
	                    Kopete::Message::Chat, Kopete::Message::TypeAction);
	appendMessage(msg);
}

#include "ircchannelcontact.moc"
