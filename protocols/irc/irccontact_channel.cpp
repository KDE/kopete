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

#include "ircaccount.h"
#include "irccontact.h"
#include "ircprotocol.h"

#include "kopeteview.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "kopetechatsessionmanager.h"

#include <kdebug.h>
#include <krun.h>
#include <kinputdialog.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include <qtimer.h>
/*

//This is the number of nicknames we will process concurrently when joining a channel
//Lower numbers ensure less GUI blocking, but take marginally longer to complete.
//Higher numbers are absolute fastest, but block GUI until all members are added
#define NICK_BATCH_LENGTH 1

IRCChannelContact::IRCChannelContact(IRCAccount *account, const QString &channel, Kopete::MetaContact *metac)
    : IRCContact(account, channel, metac, "irc_channel")
{
    actionJoin = 0L;
    actionModeT = new KToggleAction(i18n("Only Operators Can Change &Topic"), 0, this, SLOT(slotModeChanged()), this );
    actionModeN = new KToggleAction(i18n("&No Outside Messages"), 0, this, SLOT(slotModeChanged()), this );
    actionModeS = new KToggleAction(i18n("&Secret"), 0, this, SLOT(slotModeChanged()), this );
    actionModeM = new KToggleAction(i18n("&Moderated"), 0, this, SLOT(slotModeChanged()), this );
    actionModeI = new KToggleAction(i18n("&Invite Only"), 0, this, SLOT(slotModeChanged()), this );
    actionHomePage = 0L;

    updateStatus();
}

void IRCChannelContact::slotUpdateInfo()
{
//	This woudl be nice, but it generates server errors too often
//	if( !manager(Kopete::Contact::CannotCreate) && onlineStatus() == m_protocol->m_ChannelStatusOnline )
//		kircEngine()->writeMessage( QString::fromLatin1("LIST %1").arg(m_nickName) );
//	else
//		setProperty( QString::fromLatin1("channelMembers"), i18n("Members"), manager()->members().count() );

    KIRC::Engine *engine = kircEngine();

    if (manager(Kopete::Contact::CannotCreate))
    {
        setProperty(m_protocol->propChannelMembers, manager()->members().count());
        engine->writeMessage(QString::fromLatin1("WHO %1").arg(m_nickName));
    }
    else
    {
        removeProperty(m_protocol->propChannelMembers);
        removeProperty(m_protocol->propChannelTopic);
    }

    mInfoTimer->start( 45000, true );
}

void IRCChannelContact::slotChannelListed( const QString &channel, uint members, const QString &topic )
{
    if (!manager(Kopete::Contact::CannotCreate) &&
        onlineStatus() == m_protocol->m_ChannelStatusOnline &&
        channel.toLower() == m_nickName.toLower())
    {
        mTopic = topic;
        setProperty(m_protocol->propChannelMembers, members);
        setProperty(m_protocol->propChannelTopic, topic);
    }
}

void IRCChannelContact::chatSessionDestroyed()
{
    if (manager(Kopete::Contact::CannotCreate))
    {
        part();
        Kopete::ContactPtrList contacts = manager()->members();

        // remove all the users on the channel
        for (Kopete::Contact *c = contacts.first(); c; c = contacts.next())
        {
            if (c->metaContact()->isTemporary() &&
                !static_cast<IRCContact*>(c)->isChatting(manager()))
                c->deleteLater();
        }
    }

    IRCContact::chatSessionDestroyed();
}

void IRCChannelContact::initConversation()
{
    kircEngine()->join(m_nickName, password());
}

void IRCChannelContact::slotConnectedToServer()
{
    setOnlineStatus(m_protocol->m_ChannelStatusOnline);
    if (manager(Kopete::Contact::CannotCreate))
        kircEngine()->join(m_nickName, password());
}

void IRCChannelContact::namesList(const QStringList &nicknames)
{
    mInfoTimer->stop();
    mJoinedNicks += nicknames;
    slotAddNicknames();
}

void IRCChannelContact::endOfNames()
{
    setMode(QString::null);	//krazy:exclude=nullstrassign for old broken gcc
    slotUpdateInfo();
}

void IRCChannelContact::slotAddNicknames()
{
    if( !manager(Kopete::Contact::CannotCreate) || mJoinedNicks.isEmpty())
    {
        return;
    }

    IRCAccount *account = ircAccount();

    for( uint i = 0; !mJoinedNicks.isEmpty() && i < NICK_BATCH_LENGTH; ++i )
    {
        QString nickToAdd = mJoinedNicks.front();
        QChar firstChar = nickToAdd[0];
        if( firstChar == '@' || firstChar == '%' || firstChar == '+' )
            nickToAdd = nickToAdd.remove(0, 1);

        IRCContact *user;

        if ( nickToAdd.toLower() != account->mySelf()->nickName().toLower() )
        {
            //kDebug(14120) << m_nickName << " NICK: " << nickToAdd;
            user = account->contactManager()->findUser(nickToAdd);
            user->setOnlineStatus(m_protocol->m_UserStatusOnline);
        }
        else
        {
            user = account->mySelf();
        }

        Kopete::OnlineStatus status;
        if ( firstChar == '@' || firstChar == '%' )
            status = m_protocol->m_UserStatusOp;
        else if( firstChar == '+')
            status = m_protocol->m_UserStatusVoice;
        else
            status = user->onlineStatus();

        if( user != account->mySelf() )
            manager()->addContact(static_cast<Kopete::Contact*>(user) , status, true);
        else
            manager()->setContactOnlineStatus( static_cast<Kopete::Contact*>(user), status );

        mJoinedNicks.pop_front();
    }

    QTimer::singleShot( 0, this, SLOT(slotAddNicknames()) );
}

void IRCChannelContact::channelTopic(const QString &topic)
{
    mTopic = topic;
    setProperty( m_protocol->propChannelTopic, mTopic );
    manager()->setDisplayName(caption());
    Kopete::Message msg((Kopete::Contact*)this, mMyself, i18n("Topic for %1 is %2",m_nickName,mTopic),
                        Kopete::Message::Internal, Kopete::Message::RichText, CHAT_VIEW);
    appendMessage(msg);
}

void IRCChannelContact::channelHomePage(const QString &url)
{
    kDebug(14120) ;
    setProperty( m_protocol->propHomepage, url );
}

void IRCChannelContact::join()
{
    if (!manager(Kopete::Contact::CannotCreate) &&
        onlineStatus().status() == Kopete::OnlineStatus::Online)
    {
        kDebug() << "My nickname:" << m_nickName;
        kDebug() << "My manager:" << manager(Kopete::Contact::CannotCreate);
        if( manager(Kopete::Contact::CannotCreate) )
            kDebug() << "My view:" << manager(Kopete::Contact::CannotCreate)->view(false);
        startChat();
    }
}

void IRCChannelContact::part()
{
    if (manager(Kopete::Contact::CannotCreate))
        kircEngine()->part(m_nickName, ircAccount()->defaultPart());
}

void IRCChannelContact::slotIncomingUserIsAway( const QString &nick, const QString & )
{
    IRCAccount *account = ircAccount();

    if( nick.toLower() == account->mySelf()->nickName().toLower() )
    {
        IRCUserContact *c = account->mySelf();
        if (manager(Kopete::Contact::CannotCreate) && manager()->members().contains(c))
        {
            Kopete::OnlineStatus status = manager()->contactOnlineStatus(c);
            if (status == m_protocol->m_UserStatusOp)
                manager()->setContactOnlineStatus(c, m_protocol->m_UserStatusOpAway );
            else if (status == m_protocol->m_UserStatusOpAway)
                manager()->setContactOnlineStatus(c, m_protocol->m_UserStatusOp);
            else if (status == m_protocol->m_UserStatusVoice)
                manager()->setContactOnlineStatus(c, m_protocol->m_UserStatusVoiceAway);
            else if (status == m_protocol->m_UserStatusVoiceAway)
                manager()->setContactOnlineStatus(c, m_protocol->m_UserStatusVoice);
            else if (status == m_protocol->m_UserStatusAway)
                manager()->setContactOnlineStatus(c, m_protocol->m_UserStatusOnline);
            else
                manager()->setContactOnlineStatus(c, m_protocol->m_UserStatusAway);
        }
    }
}

void IRCChannelContact::userJoinedChannel(const QString &nickname)
{
    IRCAccount *account = ircAccount();

    if (nickname.toLower() == account->mySelf()->nickName().toLower())
    {
        kDebug() << "Me:" << this;
        kDebug() << "My nickname:" << m_nickName;
        kDebug() << "My manager:" << manager(Kopete::Contact::CannotCreate);
        if (manager(Kopete::Contact::CannotCreate))
            kDebug() << "My view:" << manager(Kopete::Contact::CannotCreate)->view(false);

        Kopete::Message msg((Kopete::Contact *)this, mMyself,
            i18n("You have joined channel %1",m_nickName),
            Kopete::Message::Internal, Kopete::Message::PlainText,
            CHAT_VIEW);
        msg.setImportance( Kopete::Message::Low); //set the importance manualy to low
        appendMessage(msg);
    }
    else
    {
        IRCUserContact *contact = account->contactManager()->findUser( nickname );
        contact->setOnlineStatus( m_protocol->m_UserStatusOnline );
        manager()->addContact((Kopete::Contact *)contact, true);
        Kopete::Message msg((Kopete::Contact *)this, mMyself,
            i18n("User <b>%1</b> joined channel %2",nickname,m_nickName),
            Kopete::Message::Internal, Kopete::Message::RichText, CHAT_VIEW);
        msg.setImportance( Kopete::Message::Low); //set the importance manualy to low
        manager()->appendMessage(msg);
    }
}

void IRCChannelContact::userPartedChannel(const QString &nickname,const QString &reason)
{
    IRCAccount *account = ircAccount();

    if (nickname.toLower() != account->engine()->nickName().toLower())
    {
        Kopete::Contact *c = locateUser( nickname );
        if ( c )
        {
            manager()->removeContact( c, Kopete::Message::unescape(reason) );
            if( c->metaContact()->isTemporary() && !static_cast<IRCContact*>(c)->isChatting( manager(Kopete::Contact::CannotCreate) ) )
                c->deleteLater();
        }
    }
}

void IRCChannelContact::userKicked(const QString &nick, const QString &nickKicked, const QString &reason)
{
    IRCAccount *account = ircAccount();

    QString r = i18n("Kicked by %1.",nick);
    if (reason != nick)
        r.append( i18n(" Reason: %2", reason ) );

    if( nickKicked.toLower() != account->engine()->nickName().toLower() )
    {
        Kopete::Contact *c = locateUser( nickKicked );
        if (c)
        {
            manager()->removeContact( c, r );
            Kopete::Message msg( (Kopete::Contact *)this, mMyself,
                          r, Kopete::Message::Internal, Kopete::Message::PlainText, CHAT_VIEW);
            msg.setImportance(Kopete::Message::Low);
            appendMessage(msg);
            if( c->metaContact()->isTemporary() && !static_cast<IRCContact*>(c)->isChatting( manager(Kopete::Contact::CannotCreate) ) )
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
    IRCAccount *account = ircAccount();

    if (manager(Kopete::Contact::CannotCreate))
    {
        if( manager()->contactOnlineStatus( manager()->myself() ) ==
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
                kircEngine()->topic(m_nickName, newTopic);
            }
        }
        else
        {
            Kopete::Message msg(account->myServer(), manager()->members(),
                i18n("You must be a channel operator on %1 to do that.",m_nickName),
                Kopete::Message::Internal, Kopete::Message::PlainText, CHAT_VIEW);
            manager()->appendMessage(msg);
        }
    }
}

void IRCChannelContact::topicChanged(const QString &nick, const QString &newtopic)
{
    IRCAccount *account = ircAccount();

    mTopic = newtopic;
    setProperty( m_protocol->propChannelTopic, mTopic );
    manager()->setDisplayName( caption() );
    Kopete::Message msg(account->myServer(), mMyself,
        i18n("%1 has changed the topic to: %2",nick,newtopic),
        Kopete::Message::Internal, Kopete::Message::RichText, CHAT_VIEW);
    msg.setImportance(Kopete::Message::Low); //set the importance manualy to low
    appendMessage(msg);
}

void IRCChannelContact::topicUser(const QString &nick, const QDateTime &time)
{
    IRCAccount *account = ircAccount();

    Kopete::Message msg(account->myServer(), mMyself,
        i18n("Topic set by %1 at %2",nick,
            KLocale::global()->formatDateTime(time, KLocale::ShortDate)
    ), Kopete::Message::Internal, Kopete::Message::PlainText, CHAT_VIEW);
    msg.setImportance(Kopete::Message::Low); //set the importance manualy to low
    appendMessage(msg);
}

void IRCChannelContact::incomingModeChange( const QString &nick, const QString &mode )
{
    Kopete::Message msg((Kopete::Contact *)this, mMyself, i18n("%1 sets mode %2 on  %3",nicki,mode,m_nickName), Kopete::Message::Internal, Kopete::Message::PlainText, CHAT_VIEW);
    msg.setImportance( Kopete::Message::Low); //set the importance manualy to low
    appendMessage(msg);

    bool inParams = false;
    bool modeEnabled = false;
    QString params;
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

void IRCChannelContact::incomingChannelMode(const QString &mode, const QString &params)
{
    for( uint i=1; i < mode.length(); i++ )
    {
        if( mode[i] != 'l' && mode[i] != 'k' )
            toggleMode( mode[i], true, false );
    }
}

void IRCChannelContact::setMode(const QString &mode)
{
    if (manager(Kopete::Contact::CannotCreate))
        kircEngine()->mode(m_nickName, mode);
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
        i18n("<qt>You cannot join %1 because you have been banned.</qt>",m_nickName),
        i18n("IRC Plugin") );
}

void IRCChannelContact::failedChanInvite()
{
    manager()->deleteLater();
    KMessageBox::error( Kopete::UI::Global::mainWidget(),
        i18n("<qt>You cannot join %1 because it is set to invite only, and no one has invited you.</qt>",m_nickName), i18n("IRC Plugin") );
}

void IRCChannelContact::failedChanFull()
{
    manager()->deleteLater();
    KMessageBox::error( Kopete::UI::Global::mainWidget(),
        i18n("<qt>You cannot join %1 because it has reached its user limit.</qt>",m_nickName),
        i18n("IRC Plugin") );
}

void IRCChannelContact::failedChankey()
{
    bool ok;
    QString diaPassword = KInputDialog::getText( i18n( "IRC Plugin" ),
        i18n( "Please enter key for channel %1: ",m_nickName),
        QString(),
        &ok );

    if ( !ok )
        manager()->deleteLater();
    else
    {
        setPassword(diaPassword);
        kircEngine()->join(m_nickName, password());
    }
}

void IRCChannelContact::toggleMode( QChar mode, bool enabled, bool update )
{
    if( manager(Kopete::Contact::CannotCreate) )
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

QPtrList<QAction> *IRCChannelContact::customContextMenuActions()
{
    // QAction stuff
    QPtrList<QAction> *mCustomActions = new QPtrList<QAction>();
    if( !actionJoin )
    {
        actionJoin = new QAction(i18n("&Join"), 0, this, SLOT(join()), this, "actionJoin");
        actionPart = new QAction(i18n("&Part"), 0, this, SLOT(part()), this, "actionPart");
        actionTopic = new QAction(i18n("Change &Topic..."), 0, this, SLOT(setTopic()), this, "actionTopic");
        actionModeMenu = new KActionMenu(i18n("Channel Modes"), 0, this, "actionModeMenu");

        if( !property(m_protocol->propHomepage).value().isNull() )
        {
            actionHomePage = new QAction( i18n("Visit &Homepage"), 0, this,
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
        connect( codecAction, SIGNAL(activated(const QTextCodec*)),
            this, SLOT(setCodec(const QTextCodec*)) );
        codecAction->setCodec( codec() );
    }

    mCustomActions->append( actionJoin );
    mCustomActions->append( actionPart );
    mCustomActions->append( actionTopic );
    mCustomActions->append( actionModeMenu );
    mCustomActions->append( codecAction );
    if( actionHomePage )
        mCustomActions->append( actionHomePage );

    bool isOperator = manager(Kopete::Contact::CannotCreate) &&
        (manager()->contactOnlineStatus(ircAccount()->myself()) == m_protocol->m_UserStatusOp);

    actionJoin->setEnabled( !manager(Kopete::Contact::CannotCreate) );
    actionPart->setEnabled( manager(Kopete::Contact::CannotCreate) );
    actionTopic->setEnabled( manager(Kopete::Contact::CannotCreate) && ( !modeEnabled('t') || isOperator ) );

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
           new KRun( KUrl( homePage ), 0, false);
    }
}
*/
