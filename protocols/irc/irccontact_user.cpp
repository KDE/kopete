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

#include "ircaccount.h"
#include "irccontact.h"
#include "ircprotocol.h"

#include "kopetemetacontact.h"
#include "kopeteview.h"

#include <QAction>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>

#include <qtimer.h>
#include <QFileDialog>
/*
QString IRCContact::user_caption() const
{
    return i18n("%1 @ %2").arg(m_nickName).arg(kircEngine()->currentHost());
}

void IRCContact::user_updateStatus()
{
        Kopete::OnlineStatus newStatus;

    switch (kircEngine()->connectionState())
    {
    case KIRC::Idle:
        newStatus = m_protocol->m_UserStatusOffline;
        break;

    case KIRC::Connecting:
    case KIRC::Authentifying:
        if (this == ircAccount()->mySelf())
            newStatus = m_protocol->m_UserStatusConnecting;
        else
            newStatus = m_protocol->m_UserStatusOffline;
        break;

    case KIRC::Connected:
//		if (m_isAway)
//			newStatus = m_protocol->m_UserStatusAway;
//		else if (m_isOnline)
            newStatus = m_protocol->m_UserStatusOnline;
        break;
    case KIRC::Closing:
        newStatus = m_protocol->m_UserStatusOffline;
        break;
    default:
        newStatus = m_protocol->m_StatusUnknown;
    }

    // This may not be created yet ( for myself() on startup )
    if( ircAccount()->contactManager() )
    {
        QValueList<IRCChannelContact*> channels = ircAccount()->contactManager()->findChannelsByMember(this);

        for( QValueList<IRCChannelContact*>::iterator it = channels.begin(); it != channels.end(); ++it )
        {
            IRCChannelContact *channel = *it;
            Kopete::OnlineStatus currentStatus = channel->manager()->contactOnlineStatus(this);

            if( currentStatus.internalStatus() > IRCProtocol::Online )
            {
                if( !(currentStatus.internalStatus() & IRCProtocol::Away) && newStatus == m_protocol->m_UserStatusAway )
                {
                    channel->manager()->setContactOnlineStatus(
                        this, m_protocol->statusLookup(
                            (IRCProtocol::IRCStatus)(currentStatus.internalStatus()+IRCProtocol::Away)
                        )
                    );
                }
                else if( (currentStatus.internalStatus() & IRCProtocol::Away) && newStatus == m_protocol->m_UserStatusOnline )
                {
                    channel->manager()->setContactOnlineStatus(
                        this, m_protocol->statusLookup(
                            (IRCProtocol::IRCStatus)(currentStatus.internalStatus()-IRCProtocol::Away)
                        )
                    );
                }
                else if( newStatus.internalStatus() < IRCProtocol::Away )
                {
                    channel->manager()->setContactOnlineStatus( this, newStatus );
                }
            }
        }
    }
    setOnlineStatus( newStatus );
}

IRCUserContact::IRCUserContact(IRCContactManager *contactManager, const QString &nickname, Kopete::MetaContact *m )
    : IRCContact(contactManager, nickname, m ),
      m_isAway(false)
{
    setFileCapable(true);

    mOnlineTimer = new QTimer( this );
    m_isOnline = metaContact()->isTemporary();

    QObject::connect(mOnlineTimer, SIGNAL(timeout()), this, SLOT(slotUserOffline()) );

    QObject::connect(kircEngine(), SIGNAL(incomingChannelModeChange(QString,QString,QString)),
        this, SLOT(slotIncomingModeChange(QString,QString,QString)));

    actionCtcpMenu = 0L;

    mInfo.isOperator = false;
    mInfo.isIdentified = false;
    mInfo.idle = 0;
    mInfo.hops = 0;
    mInfo.away = false;

    updateStatus();
}

void IRCUserContact::sendFile(const KUrl &sourceURL, const QString&, unsigned int)
{
    QString filePath;

    //If the file location is null, then get it from a file open dialog
    if( !sourceURL.isValid() )
        filePath = QFileDialog::getOpenFileName(0l  , i18n("Kopete File Transfer"), QString::null, "*");
    else
        filePath = sourceURL.path(-1);

    kDebug(14120) << "File chosen to send:" << filePath;

    if (!filePath.isEmpty())
        kircEngine()->CtcpRequest_dcc( m_nickName, filePath, 0, KIRC::Transfer::FileOutgoing);
}

void IRCUserContact::slotUserOffline()
{
    m_isOnline = false;
    m_isAway = false;
    updateStatus();

    if( !metaContact()->isTemporary() )
        kircEngine()->writeMessage( QString::fromLatin1("WHOWAS %1").arg(m_nickName) );

    removeProperty( m_protocol->propUserInfo );
    removeProperty( m_protocol->propServer );
    removeProperty( m_protocol->propChannels );
}

void IRCUserContact::setAway(bool isAway)
{
    m_isAway = isAway;
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
    m_isOnline = true;
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
    slotKick();
}

void IRCUserContact::slotBanUserHost()
{
    slotKick();
}

void IRCUserContact::slotBanDomain()
{
    slotKick();
}

void IRCUserContact::slotBanUserDomain()
{
    slotKick();
}

void IRCUserContact::slotKick()
{
    Kopete::ContactPtrList members = mActiveManager->members();
    QString channelName = static_cast<IRCContact*>(members.first())->nickName();
    kircEngine()->kick(m_nickName, channelName, QString());
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
//	if( metaContact()->isTemporary() || isOnline() )
    if( metaContact()->isTemporary() || onlineStatus().status() != Kopete::OnlineStatus::Offline )
        mInfo.serverInfo = serverinfo;
    else
    {
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
            KLocale::global()->formatDateTime(
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

QPtrList<QAction> *IRCUserContact::customContextMenuActions( Kopete::ChatSession *manager )
{
    if( manager )
    {
        QPtrList<QAction> *mCustomActions = new QPtrList<QAction> ();
        mActiveManager = manager;
        Kopete::ContactPtrList members = mActiveManager->members();
        IRCChannelContact *isChannel = dynamic_cast<IRCChannelContact*>( members.first() );

        if( !actionCtcpMenu )
        {
            actionCtcpMenu = new KActionMenu(i18n("C&TCP"), 0, this );
            actionCtcpMenu->insert( new QAction(i18n("&Version"), 0, this,
                SLOT(slotCtcpVersion()), actionCtcpMenu) );
            actionCtcpMenu->insert(  new QAction(i18n("&Ping"), 0, this,
                SLOT(slotCtcpPing()), actionCtcpMenu) );

            actionModeMenu = new KActionMenu(i18n("&Modes"), 0, this, "actionModeMenu");
            actionModeMenu->insert( new QAction(i18n("&Op"), 0, this,
                SLOT(slotOp()), actionModeMenu, "actionOp") );
            actionModeMenu->insert( new QAction(i18n("&Deop"), 0, this,
                SLOT(slotDeop()), actionModeMenu, "actionDeop") );
            actionModeMenu->insert( new QAction(i18n("&Voice"), 0, this,
                SLOT(slotVoice()), actionModeMenu, "actionVoice") );
            actionModeMenu->insert( new QAction(i18n("Devoice"), 0, this,
                SLOT(slotDevoice()), actionModeMenu, "actionDevoice") );
            actionModeMenu->setEnabled( false );

            actionKick = new QAction(i18n("&Kick"), 0, this, SLOT(slotKick()), this);
            actionKick->setEnabled( false );

            actionBanMenu = new KActionMenu(i18n("&Ban"), 0, this, "actionBanMenu");
            actionBanMenu->insert( new QAction(i18n("Ban *!*@*.host"), 0, this,
                SLOT(slotBanHost()), actionBanMenu ) );
            actionBanMenu->insert( new QAction(i18n("Ban *!*@domain"), 0, this,
                SLOT(slotBanDomain()), actionBanMenu ) );
            actionBanMenu->insert( new QAction(i18n("Ban *!*user@*.host"), 0, this,
                 SLOT(slotBanUserHost()), actionBanMenu ) );
            actionBanMenu->insert( new QAction(i18n("Ban *!*user@domain"), 0, this,
                 SLOT(slotBanUserDomain()), actionBanMenu ) );
            actionBanMenu->setEnabled( false );

            codecAction = new KCodecAction( i18n("&Encoding"), 0, this, "selectcharset" );
            connect( codecAction, SIGNAL(activated(const QTextCodec*)),
                this, SLOT(setCodec(const QTextCodec*)) );
            codecAction->setCodec( codec() );
        }

        mCustomActions->append( actionCtcpMenu );
        mCustomActions->append( actionModeMenu );
        mCustomActions->append( actionKick );
        mCustomActions->append( actionBanMenu );
        mCustomActions->append( codecAction );

        if( isChannel )
        {
            bool isOperator = ( manager->contactOnlineStatus( account()->myself() ) == m_protocol->m_UserStatusOp );
            actionModeMenu->setEnabled(isOperator);
            actionBanMenu->setEnabled(isOperator);
            actionKick->setEnabled(isOperator);
        }

        return mCustomActions;
    }

    mActiveManager = 0L;

    return 0L;
}

void IRCUserContact::slotIncomingModeChange( const QString &channel, const QString &, const QString &mode )
{
    IRCChannelContact *chan = ircAccount()->contactManager()->findChannel( channel );
    if( chan->locateUser( m_nickName ) )
    {
        QString user = mode.section(' ', 1, 1);
        kDebug(14120) << mode << ", " << user << ", " << m_nickName;
        if( user == m_nickName )
        {
            QString modeChange = mode.section(' ', 0, 0);
            if(modeChange == QString::fromLatin1("+o"))
                setManagerStatus( chan, m_protocol->m_UserStatusOp.internalStatus() );
            else if(modeChange == QString::fromLatin1("-o"))
                setManagerStatus( chan, -m_protocol->m_UserStatusOp.internalStatus() );
            else if(modeChange == QString::fromLatin1("+v"))
                setManagerStatus( chan, m_protocol->m_UserStatusVoice.internalStatus() );
            else if(modeChange == QString::fromLatin1("-v"))
                setManagerStatus( chan, -m_protocol->m_UserStatusVoice.internalStatus() );
        }
    }
}

void IRCUserContact::setManagerStatus(IRCChannelContact *channel, int statusAdjustment)
{
    Kopete::OnlineStatus currentStatus = channel->manager()->contactOnlineStatus(this);
    Kopete::OnlineStatus newStatus = m_protocol->statusLookup(
        (IRCProtocol::IRCStatus)(currentStatus.internalStatus() + statusAdjustment)
    );

    channel->manager()->setContactOnlineStatus(this, newStatus);
}
*/
