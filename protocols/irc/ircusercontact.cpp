/*
    ircusercontact.cpp - IRC User Contact

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

#include "ircusercontact.h"
#include "ircservercontact.h"
#include "ircchannelcontact.h"
#include "irccontactmanager.h"
#include "ircaccount.h"
#include "ircprotocol.h"
#include "kcodecaction.h"

#include "kopetemetacontact.h"
#include "kopeteview.h"

#include <kaction.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>

#include <qtimer.h>

IRCUserContact::IRCUserContact(IRCContactManager *contactManager, const QString &nickname, Kopete::MetaContact *m )
	: IRCContact(contactManager, nickname, m ),
	actionCtcpMenu(0L)
{
	setFileCapable(true);

	mOnlineTimer = new QTimer( this );

	QObject::connect(mOnlineTimer, SIGNAL(timeout()), this, SLOT( slotUserOffline() ) );

	QObject::connect(kircEngine(), SIGNAL(incomingChannelModeChange(const QString&, const QString&, const QString&)),
		this, SLOT(slotIncomingModeChange(const QString&,const QString&, const QString&)));

	mInfo.isOperator = false;
	mInfo.isIdentified = false;
	mInfo.idle = 0;
	mInfo.hops = 0;
	mInfo.away = false;
	mInfo.online = metaContact()->isTemporary();

	updateStatus();
}

void IRCUserContact::updateStatus()
{
	//kdDebug(14120) << k_funcinfo << endl;

        Kopete::OnlineStatus newStatus;

	switch (kircEngine()->status())
	{
		case KIRC::Engine::Idle:
			newStatus = m_protocol->m_UserStatusOffline;
			break;

		case KIRC::Engine::Connecting:
		case KIRC::Engine::Authentifying:
			if (this == ircAccount()->mySelf())
				newStatus = m_protocol->m_UserStatusConnecting;
			else
				newStatus = m_protocol->m_UserStatusOffline;
			break;

		case KIRC::Engine::Connected:
		case KIRC::Engine::Closing:
			if (mInfo.away)
				newStatus = m_protocol->m_UserStatusAway;
			else if (mInfo.online)
				newStatus = m_protocol->m_UserStatusOnline;
			break;

		default:
			newStatus = m_protocol->m_StatusUnknown;
	}

	// Try hard not to emit several onlineStatusChanged() signals.
	bool onlineStatusChanged = false;


	/* The away status is global, so if the user goes away, we must set
	 * the new status on all channels.
	 */


	// This may not be created yet ( for myself() on startup )
	if( ircAccount()->contactManager() )
	{
		QValueList<IRCChannelContact*> channels = ircAccount()->contactManager()->findChannelsByMember(this);

		for( QValueList<IRCChannelContact*>::iterator it = channels.begin(); it != channels.end(); ++it )
		{
			IRCChannelContact *channel = *it;
			Kopete::OnlineStatus currentStatus = channel->manager()->contactOnlineStatus(this);

			//kdDebug(14120) << k_funcinfo << "iterating channel " << channel->nickName() << " internal status: " << currentStatus.internalStatus() << endl;

			if( currentStatus.internalStatus() >= IRCProtocol::Online )
			{
				onlineStatusChanged = true;

				if( !(currentStatus.internalStatus() & IRCProtocol::Away) && newStatus == m_protocol->m_UserStatusAway )
				{
					setOnlineStatus( newStatus );
					//kdDebug(14120) << k_funcinfo << "was NOT away, but is now, channel " << channel->nickName() << endl;
					adjustInternalOnlineStatusBits(channel, IRCProtocol::Away, AddBits);
				}
				else if( (currentStatus.internalStatus() & IRCProtocol::Away) && newStatus == m_protocol->m_UserStatusOnline )
				{
					setOnlineStatus( newStatus );
					//kdDebug(14120) << k_funcinfo << "was away, but not anymore, channel " << channel->nickName() << endl;
					adjustInternalOnlineStatusBits(channel, IRCProtocol::Away, RemoveBits);

				}
				else if( newStatus.internalStatus() < IRCProtocol::Away )
				{
					//kdDebug(14120) << k_funcinfo << "offline or connecting?" << endl;
					channel->manager()->setContactOnlineStatus( this, newStatus );
				}
			}
		}
	}

	if (!onlineStatusChanged) {
		//kdDebug(14120) << k_funcinfo << "setting status at last" << endl;
		setOnlineStatus( newStatus );
	}
}

void IRCUserContact::sendFile(const KURL &sourceURL, const QString&, unsigned int)
{
	QString filePath;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
		filePath = KFileDialog::getOpenFileName(QString::null, "*", 0l  , i18n("Kopete File Transfer"));
	else
		filePath = sourceURL.path(-1);

	kdDebug(14120) << k_funcinfo << "File chosen to send:" << filePath << endl;

	if (!filePath.isEmpty())
		kircEngine()->CtcpRequest_dcc( m_nickName, filePath, 0, KIRC::Transfer::FileOutgoing);
}

void IRCUserContact::slotUserOffline()
{
	mInfo.online = false;
	mInfo.away   = false;

	updateStatus();

	if( !metaContact()->isTemporary() )
		kircEngine()->writeMessage( QString::fromLatin1("WHOWAS %1").arg(m_nickName) );

	removeProperty( m_protocol->propUserInfo );
	removeProperty( m_protocol->propServer );
	removeProperty( m_protocol->propChannels );
}

void IRCUserContact::setAway(bool isAway)
{
	//kdDebug(14120) << k_funcinfo << isAway << endl;

	mInfo.away = isAway;
	updateStatus();
}

void IRCUserContact::incomingUserIsAway(const QString &reason)
{
	if( manager( Kopete::Contact::CannotCreate ) )
	{
		Kopete::Message msg( (Kopete::Contact*)ircAccount()->myServer(), mMyself,
			i18n("%1 is away (%2)").arg( m_nickName ).arg( reason ),
			Kopete::Message::Internal, Kopete::Message::RichText, CHAT_VIEW );
		manager(Kopete::Contact::CanCreate)->appendMessage(msg);
	}
}

void IRCUserContact::userOnline()
{
	mInfo.online = true;
	updateStatus();
	if (this != ircAccount()->mySelf() && !metaContact()->isTemporary() && ircAccount()->isConnected())
	{
		mOnlineTimer->start( 45000, true );
		ircAccount()->setCurrentCommandSource(0);
		kircEngine()->whois(m_nickName);
	}

	removeProperty( m_protocol->propLastSeen );
}

void IRCUserContact::slotUserInfo()
{
	if (isChatting())
	{
		ircAccount()->setCurrentCommandSource(manager());
		kircEngine()->whois(m_nickName);
	}
}

const QString IRCUserContact::caption() const
{
	return i18n("%1 @ %2").arg(m_nickName).arg(kircEngine()->currentHost());
}

void IRCUserContact::slotOp()
{
	contactMode( QString::fromLatin1("+o") );
}

void IRCUserContact::slotDeop()
{
	contactMode( QString::fromLatin1("-o") );
}

void IRCUserContact::slotVoice()
{
	contactMode( QString::fromLatin1("+v") );
}

void IRCUserContact::slotDevoice()
{
	contactMode( QString::fromLatin1("-v") );
}

void IRCUserContact::slotBanHost()
{
	// MODE #foofoofoo +b *!*@host.domain.net

	if (mInfo.hostName.isEmpty()) {
		if (kircEngine()->isConnected()) {
			kircEngine()->whois(m_nickName);
			QTimer::singleShot( 750, this, SLOT( slotBanHostOnce() ) );
		}
	} else {
		slotBanHostOnce();
	}
}
void IRCUserContact::slotBanHostOnce()
{
	if (mInfo.hostName.isEmpty())
		return;

	Kopete::ContactPtrList members = mActiveManager->members();
	QString channelName = static_cast<IRCContact*>(members.first())->nickName();

	kircEngine()->mode(channelName, QString::fromLatin1("+b *!*@%1").arg(mInfo.hostName));
}

void IRCUserContact::slotBanUserHost()
{
	// MODE #foofoofoo +b *!*user@host.domain.net

	if (mInfo.hostName.isEmpty()) {
		if (kircEngine()->isConnected()) {
			kircEngine()->whois(m_nickName);
			QTimer::singleShot( 750, this, SLOT( slotBanUserHostOnce() ) );
		}
	} else {
		slotBanUserHostOnce();
	}
}
void IRCUserContact::slotBanUserHostOnce()
{
	if (mInfo.hostName.isEmpty())
		return;

	Kopete::ContactPtrList members = mActiveManager->members();
	QString channelName = static_cast<IRCContact*>(members.first())->nickName();

	kircEngine()->mode(channelName, QString::fromLatin1("+b *!*%1@%2").arg(mInfo.userName, mInfo.hostName));
}

void IRCUserContact::slotBanDomain()
{
	// MODE #foofoofoo +b *!*@*.domain.net

	if (mInfo.hostName.isEmpty()) {
		if (kircEngine()->isConnected()) {
			kircEngine()->whois(m_nickName);
			QTimer::singleShot( 750, this, SLOT( slotBanDomainOnce() ) );
		}
	} else {
		slotBanDomainOnce();
	}
}
void IRCUserContact::slotBanDomainOnce()
{
	if (mInfo.hostName.isEmpty())
		return;

	Kopete::ContactPtrList members = mActiveManager->members();
	QString channelName = static_cast<IRCContact*>(members.first())->nickName();

	QString domain = mInfo.hostName.section('.', 1);

	kircEngine()->mode(channelName, QString::fromLatin1("+b *!*@*.%1").arg(domain));
}

void IRCUserContact::slotBanUserDomain()
{
	// MODE #foofoofoo +b *!*user@*.domain.net

	if (mInfo.hostName.isEmpty()) {
		if (kircEngine()->isConnected()) {
			kircEngine()->whois(m_nickName);
			QTimer::singleShot( 750, this, SLOT( slotBanUserDomainOnce() ) );
		}
	} else {
		slotBanUserDomainOnce();
	}
}
void IRCUserContact::slotBanUserDomainOnce()
{
	if (mInfo.hostName.isEmpty())
		return;

	Kopete::ContactPtrList members = mActiveManager->members();
	QString channelName = static_cast<IRCContact*>(members.first())->nickName();

	QString domain = mInfo.hostName.section('.', 1);

	kircEngine()->mode(channelName, QString::fromLatin1("+b *!*%1@*.%2").arg(mInfo.userName, domain));
}

void IRCUserContact::slotKick()
{
	Kopete::ContactPtrList members = mActiveManager->members();
	QString channelName = static_cast<IRCContact*>(members.first())->nickName();
	kircEngine()->kick(m_nickName, channelName, QString::null);
}

void IRCUserContact::contactMode(const QString &mode)
{
	Kopete::ContactPtrList members = mActiveManager->members();
	QString channelName = static_cast<IRCContact*>(members.first())->nickName();
	kircEngine()->mode(channelName, QString::fromLatin1("%1 %2").arg(mode).arg(m_nickName));
}

void IRCUserContact::slotCtcpPing()
{
	kircEngine()->CtcpRequest_ping(m_nickName);
}

void IRCUserContact::slotCtcpVersion()
{
	kircEngine()->CtcpRequest_version(m_nickName);
}

void IRCUserContact::newWhoIsUser(const QString &username, const QString &hostname, const QString &realname)
{
	mInfo.channels.clear();
	mInfo.userName = username;
	mInfo.hostName = hostname;
	mInfo.realName = realname;

	if( onlineStatus().status() == Kopete::OnlineStatus::Offline )
	{
		setProperty( m_protocol->propUserInfo, QString::fromLatin1("%1@%2")
			.arg(mInfo.userName).arg(mInfo.hostName) );
		setProperty( m_protocol->propServer, mInfo.serverName );
		setProperty( m_protocol->propFullName, mInfo.realName );
	}
}

void IRCUserContact::newWhoIsServer(const QString &servername, const QString &serverinfo)
{
	mInfo.serverName = servername;
	if( metaContact()->isTemporary() || onlineStatus().status() == Kopete::OnlineStatus::Online
		|| onlineStatus().status() == Kopete::OnlineStatus::Away )
		mInfo.serverInfo = serverinfo;
	else
	{
		//kdDebug(14120)<< "Setting last online: " << serverinfo << endl;

		// Try to convert first, since server can return depending if
		// user is online or not:
		// 
		//   312 mynick othernick localhost.localdomain :FooNet Server
		//   312 mynick othernick localhost.localdomain :Thu Jun 16 21:00:36 2005

		QDateTime lastSeen = QDateTime::fromString( serverinfo );
		if( lastSeen.isValid() )
			setProperty( m_protocol->propLastSeen, lastSeen );
	}
}

void IRCUserContact::newWhoIsIdle(unsigned long idle)
{
	mInfo.idle = idle;
}

void IRCUserContact::newWhoIsOperator()
{
	mInfo.isOperator = true;
}

void IRCUserContact::newWhoIsIdentified()
{
	mInfo.isIdentified = true;
	setProperty( m_protocol->propIsIdentified, i18n("True") );
}

void IRCUserContact::newWhoIsChannels(const QString &channel)
{
	mInfo.channels.append( channel );
}

void IRCUserContact::whoIsComplete()
{
	Kopete::ChatSession *commandSource = ircAccount()->currentCommandSource();

	updateInfo();

	if( isChatting() && commandSource &&
		commandSource == manager(Kopete::Contact::CannotCreate) )
	{
		//User info
		QString msg = i18n("%1 is (%2@%3): %4<br/>")
			.arg(m_nickName)
			.arg(mInfo.userName)
			.arg(mInfo.hostName)
			.arg(mInfo.realName);

		if( mInfo.isIdentified )
			msg += i18n("%1 is authenticated with NICKSERV<br/>").arg(m_nickName);

		if( mInfo.isOperator )
			msg += i18n("%1 is an IRC operator<br/>").arg(m_nickName);

		//Channels
		msg += i18n("on channels %1<br/>").arg(mInfo.channels.join(" ; "));

		//Server
		msg += i18n("on IRC via server %1 ( %2 )<br/>").arg(mInfo.serverName).arg(mInfo.serverInfo);

		//Idle
		QString idleTime = formattedIdleTime();
		msg += i18n("idle: %2<br/>").arg( idleTime.isEmpty() ? QString::number(0) : idleTime );

		//End
		ircAccount()->appendMessage(msg, IRCAccount::InfoReply );
		ircAccount()->setCurrentCommandSource(0);
	}
}

void IRCUserContact::whoWasComplete()
{
	if( isChatting() && ircAccount()->currentCommandSource() == manager() )
	{
		//User info
		QString msg = i18n("%1 was (%2@%3): %4\n")
			.arg(m_nickName)
			.arg(mInfo.userName)
			.arg(mInfo.hostName)
			.arg(mInfo.realName);

		msg += i18n("Last Online: %1\n").arg(
			KGlobal::locale()->formatDateTime(
				property( m_protocol->propLastSeen ).value().toDateTime()
			)
		);

		ircAccount()->appendMessage(msg, IRCAccount::InfoReply );
		ircAccount()->setCurrentCommandSource(0);
	}
}

QString IRCUserContact::formattedName() const
{
	return mInfo.realName;
}

void IRCUserContact::updateInfo()
{
	setProperty( m_protocol->propUserInfo, QString::fromLatin1("%1@%2")
		.arg(mInfo.userName).arg(mInfo.hostName) );
	setProperty( m_protocol->propServer, mInfo.serverName );
	setProperty( m_protocol->propChannels, mInfo.channels.join(" ") );
	setProperty( m_protocol->propHops, QString::number(mInfo.hops) );
	setProperty( m_protocol->propFullName, mInfo.realName );

	setIdleTime( mInfo.idle );

	mInfo.lastUpdate = QTime::currentTime();
}

void IRCUserContact::newWhoReply( const QString &channel, const QString &user, const QString &host,
	const QString &server, bool away, const QString &flags, uint hops, const QString &realName )
{
	if( !mInfo.channels.contains( channel ) )
		mInfo.channels.append( channel );

	mInfo.userName = user;
	mInfo.hostName = host;
	mInfo.serverName = server;
	mInfo.flags = flags;
	mInfo.hops = hops;
	mInfo.realName = realName;

	setAway(away);

	updateInfo();

	if( isChatting() && ircAccount()->currentCommandSource() == manager() )
	{
		ircAccount()->setCurrentCommandSource(0);
	}
}

QPtrList<KAction> *IRCUserContact::customContextMenuActions( Kopete::ChatSession *manager )
{
	if( manager )
	{
		QPtrList<KAction> *mCustomActions = new QPtrList<KAction> ();
		mActiveManager = manager;
		Kopete::ContactPtrList members = mActiveManager->members();
		IRCChannelContact *isChannel = dynamic_cast<IRCChannelContact*>( members.first() );

		if( !actionCtcpMenu )
		{
			actionCtcpMenu = new KActionMenu(i18n("C&TCP"), 0, this );
			actionCtcpMenu->insert( new KAction(i18n("&Version"), 0, this,
				SLOT(slotCtcpVersion()), actionCtcpMenu) );
			actionCtcpMenu->insert(  new KAction(i18n("&Ping"), 0, this,
				SLOT(slotCtcpPing()), actionCtcpMenu) );

			actionModeMenu = new KActionMenu(i18n("&Modes"), 0, this, "actionModeMenu");
			actionModeMenu->insert( new KAction(i18n("&Op"), 0, this,
				SLOT(slotOp()), actionModeMenu, "actionOp") );
			actionModeMenu->insert( new KAction(i18n("&Deop"), 0, this,
				SLOT(slotDeop()), actionModeMenu, "actionDeop") );
			actionModeMenu->insert( new KAction(i18n("&Voice"), 0, this,
				SLOT(slotVoice()), actionModeMenu, "actionVoice") );
			actionModeMenu->insert( new KAction(i18n("Devoice"), 0, this,
				SLOT(slotDevoice()), actionModeMenu, "actionDevoice") );
			actionModeMenu->setEnabled( false );

			actionKick = new KAction(i18n("&Kick"), 0, this, SLOT(slotKick()), this);
			actionKick->setEnabled( false );

			actionBanMenu = new KActionMenu(i18n("&Ban"), 0, this, "actionBanMenu");
			actionBanMenu->insert( new KAction(i18n("Host (*!*@host.domain.net)"), 0, this,
				SLOT(slotBanHost()), actionBanMenu ) );
			actionBanMenu->insert( new KAction(i18n("Domain (*!*@*.domain.net)"), 0, this,
				SLOT(slotBanDomain()), actionBanMenu ) );
			actionBanMenu->insert( new KAction(i18n("User@Host (*!*user@host.domain.net)"), 0, this,
				 SLOT(slotBanUserHost()), actionBanMenu ) );
			actionBanMenu->insert( new KAction(i18n("User@Domain (*!*user@*.domain.net)"), 0, this,
				 SLOT(slotBanUserDomain()), actionBanMenu ) );
			actionBanMenu->setEnabled( false );

			codecAction = new KCodecAction( i18n("&Encoding"), 0, this, "selectcharset" );
			connect( codecAction, SIGNAL( activated( const QTextCodec * ) ),
				this, SLOT( setCodec( const QTextCodec *) ) );
			codecAction->setCodec( codec() );
		}

		mCustomActions->append( actionCtcpMenu );
		mCustomActions->append( actionModeMenu );
		mCustomActions->append( actionKick );
		mCustomActions->append( actionBanMenu );
		mCustomActions->append( codecAction );

		if( isChannel )
		{
			bool isOperator = ( manager->contactOnlineStatus( account()->myself() ).internalStatus() & IRCProtocol::Operator );
			actionModeMenu->setEnabled(isOperator);
			actionBanMenu->setEnabled(isOperator);
			actionKick->setEnabled(isOperator);
		}

		return mCustomActions;
	}

	mActiveManager = 0L;

	return 0L;
}

void IRCUserContact::slotIncomingModeChange( const QString &channel, const QString &, const QString &mode_ )
{
	kdDebug(14120) << k_funcinfo << "channel: " << channel << " mode: " << mode_ << endl;

	IRCChannelContact *chan = ircAccount()->contactManager()->findChannel( channel );

	if( !chan->locateUser( m_nickName ) )
		return;

	// :foobar_!~fooobar@dhcp.inet.fi MODE #foofoofoo2 +o kakkonen
	// :foobar_!~fooobar@dhcp.inet.fi MODE #foofoofoo2 +o-o foobar001 kakkonen
	// :foobar_!~fooobar@dhcp.inet.fi MODE #foofoofoo2 +oo kakkonen foobar001
	// :foobar_!~fooobar@dhcp.inet.fi MODE #foofoofoo2 +o-ov foobar001 kakkonen foobar001
	//
	// irssi manual example: /MODE #channel +nto-o+v nick1 nick2 nick3

	QStringList users = QStringList::split(' ', mode_);
	users.pop_front();

	const QString mode = mode_.section(' ', 0, 0);

	bitAdjustment adjMode = RemoveBits;
	QStringList::iterator user = users.begin();

	//kdDebug(14120) << "me: " << m_nickName << " users: " << users << " mode: " << mode << endl;

	for( uint i=0; i < mode.length(); i++ )
	{
		switch( mode[i] )
		{
		case '+':
			adjMode = AddBits;
			break;

		case '-':
			adjMode = RemoveBits;
			break;

		default:
			//kdDebug(14120) << "got " << mode[i] << ", user: " << *user << endl;

			if (mode[i] == 'o') {
				if (user == users.end())
					return;

				if ((*user).lower() == m_nickName.lower())
					adjustInternalOnlineStatusBits(chan, IRCProtocol::Operator, adjMode);

				++user;
			}
			else if (mode[i] == 'v') {
				if (user == users.end())
					return;

				if ((*user).lower() == m_nickName.lower())
					adjustInternalOnlineStatusBits(chan, IRCProtocol::Voiced, adjMode);

				++user;
			}

			break;
		}
	}
}


/* Remove or add the given bits for the given channel from the current internal online status.
 *
 * You could fiddle with bits like IRCProtocol::Operator, IRCProtocol::Voiced, etc.
 */

void IRCUserContact::adjustInternalOnlineStatusBits(IRCChannelContact *channel, unsigned statusAdjustment, bitAdjustment adj)
{
	Kopete::OnlineStatus currentStatus = channel->manager()->contactOnlineStatus(this);
	Kopete::OnlineStatus newStatus;

	if (adj == RemoveBits) {

		// If the bit is not set in the current internal status, stop here.
		if ((currentStatus.internalStatus() & ~statusAdjustment) == currentStatus.internalStatus())
			return;

		newStatus = m_protocol->statusLookup(
				(IRCProtocol::IRCStatus)(currentStatus.internalStatus() & ~statusAdjustment)
				);

	} else if (adj == AddBits) {

		// If the bit is already set in the current internal status, stop here.
		if ((currentStatus.internalStatus() | statusAdjustment) == currentStatus.internalStatus())
			return;

		newStatus = m_protocol->statusLookup(
				(IRCProtocol::IRCStatus)(currentStatus.internalStatus() | statusAdjustment)
				);

	}

	channel->manager()->setContactOnlineStatus(this, newStatus);
}

void IRCUserContact::privateMessage(IRCContact *from, IRCContact *to, const QString &message)
{
	if (to == this)
	{
		if(to==account()->myself())
		{
			Kopete::Message msg(from, from->manager(Kopete::Contact::CanCreate)->members(), message,
				Kopete::Message::Inbound, Kopete::Message::RichText, CHAT_VIEW);
			from->appendMessage(msg);
		}
		else
		{
			kdDebug(14120) << "IRC Server error: Received a private message for " << to->nickName() << ":" << message << endl;
			// emit/call something on main ircservercontact
		}
	}
}

void IRCUserContact::newAction(const QString &to, const QString &action)
{
	IRCAccount *account = ircAccount();

	IRCContact *t = account->contactManager()->findUser(to);

	Kopete::Message::MessageDirection dir =
		(this == account->mySelf()) ? Kopete::Message::Outbound : Kopete::Message::Inbound;
	Kopete::Message msg(this, t, action, dir, Kopete::Message::RichText,
			CHAT_VIEW, Kopete::Message::TypeAction);

	//Either this is from me to a guy, or from a guy to me. Either way its a PM
	if (dir == Kopete::Message::Outbound)
		t->appendMessage(msg);
	else
		appendMessage(msg);
}

#include "ircusercontact.moc"
