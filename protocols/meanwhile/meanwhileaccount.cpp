/*
    meanwhileaccount.cpp - a meanwhile account

    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>

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
#include <signal.h>
#include "meanwhileprotocol.h"
#include "meanwhileserver.h"
#include "meanwhileplugin.h"
#include "meanwhileaccount.h"
#include "meanwhilecontact.h"
#include "kopetemessagemanager.h"
#include "kopetepassword.h"

#include <kaction.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include "kopeteaway.h"
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <qdict.h>

#include "log.h"

#define INIT_SERVER()    \
    if (server == NULL)  \
        initServer();    \
    if (server != NULL)

MeanwhileAccount::MeanwhileAccount(
                        MeanwhileProtocol *parent,
                        const QString &accountID,
                        const char *name)
    : Kopete::PasswordedAccount ( parent, accountID, 0, name )
{
    //signal(SIGSEGV,SIG_DFL);
    //LOG("MeanwhileAccount");
    setMyself( new MeanwhileContact(
                        accountId(),
                        accountId(),
                        this,
                        0L));
    myself()->setOnlineStatus(MeanwhileProtocol::protocol()->meanwhileOffline);
    server = NULL;
    infoPlugin = new MeanwhilePlugin();
}

void MeanwhileAccount::initServer()
{
    server = new MeanwhileServer(serverName(),serverPort());
    if (server->bad())
    {
        delete server;
        server = NULL;
    }
    else
    {
        QObject::connect(server,
                     SIGNAL(loginDone()),
                     this,
                     SLOT(slotLoginDone()));
        QObject::connect(server,
                     SIGNAL(mesgReceived(const QString &,
                                         const QString &)),
                     SLOT(slotMesgReceived(const QString &,
                                         const QString &)));
        QObject::connect(server,
                     SIGNAL(userTyping(const QString &,
                                       bool)),
                     SLOT(slotUserTyping(const QString &,
                                         bool)));
        QObject::connect(server,
                     SIGNAL(connectionLost()),
                     SLOT(slotServerDead()));
        QObject::connect(server,
                     SIGNAL(notification(const QString &)),
                     SLOT(slotServerNotification(const QString&)));
    }
}

MeanwhileAccount::~MeanwhileAccount()
{
    meanwhileGoOffline();
}

void MeanwhileAccount::setPlugin(MeanwhilePlugin *plugin)
{
    delete infoPlugin;
    infoPlugin = plugin;
}

bool MeanwhileAccount::createContact(
                        const QString & contactId ,
                        Kopete::MetaContact * parentContact)
{
	MeanwhileContact* newContact =
                new MeanwhileContact(contactId,
                                     parentContact->displayName(),
                                     this,
                                     parentContact);
    if ((newContact != NULL) && (server != NULL)
        && (myself()->onlineStatus() !=
                MeanwhileProtocol::protocol()->meanwhileOffline))
        server->addContact(newContact,contacts());

	return newContact != NULL;
}

void MeanwhileAccount::connectWithPassword(const QString &password)
{
    if (password.isEmpty())
        return;

    if (server==NULL)
        meanwhileGoOnline();
}

void MeanwhileAccount::disconnect()
{
    meanwhileGoOffline();
}

void MeanwhileAccount::setAway(
                        bool away,
                        const QString &reason)
{
    if (away)
    {
        meanwhileGoAway(reason);
    }
    else
    {
        meanwhileGoOnline();
    }
}

KActionMenu * MeanwhileAccount::actionMenu()
{
    KActionMenu * theMenu = Kopete::Account::actionMenu();

    theMenu->popupMenu()->insertSeparator();

    theMenu->insert(
           new KAction( i18n("&Change Status Message"), QString::null,
                        0, this, SLOT(meanwhileChangeStatus()), this,
                        "meanwhileChangeStatus"));

    infoPlugin->addCustomMenus(theMenu);

    return theMenu;
}

void MeanwhileAccount::meanwhileGoOnline()
{
    if (server!=NULL)
    {
        server->goActive(statusMesg);
        return;
    }

    QString passwd = password().cachedValue();
    if (!passwd.isNull())
    {
        INIT_SERVER()
        {
            server->login(accountId(),passwd);
        }
    }
    else
    {
        connect();
    }
}

void MeanwhileAccount::meanwhileGoOffline()
{
    if ((server!=NULL) &&
        (myself()->onlineStatus() !=
            MeanwhileProtocol::protocol()->meanwhileOffline))
            server->logoff();
    if (server!=NULL)
    {
        delete server;
        server = NULL;
    }
    myself()->setOnlineStatus(MeanwhileProtocol::protocol()->meanwhileOffline);

    QDictIterator<Kopete::Contact> it(contacts());

    for( ; it.current(); ++it )
    {
        Kopete::Contact *contact =
                static_cast<Kopete::Contact *>(it.current());
        contact->setOnlineStatus(MeanwhileProtocol::protocol()->meanwhileOffline);
    }
}

void MeanwhileAccount::meanwhileGoAway()
{
    meanwhileGoAway(Kopete::Away::getInstance()->message());
}

void MeanwhileAccount::meanwhileGoAway(const QString &statusmsg)
{
    if ((server!=NULL) &&
        (myself()->onlineStatus() !=
            MeanwhileProtocol::protocol()->meanwhileOffline))
        server->goAway(statusmsg);
}

void MeanwhileAccount::meanwhileGoDND()
{
    if ((server!=NULL) &&
        (myself()->onlineStatus() !=
            MeanwhileProtocol::protocol()->meanwhileOffline))
        server->goDND(QString("Please do not disturb"));
}

void MeanwhileAccount::slotLoginDone()
{
    myself()->setOnlineStatus(MeanwhileProtocol::protocol()->meanwhileOnline);
    statusMesg = QString("I am active");
    server->changeStatus(statusMesg);
    server->addContacts(contacts());
}

QString MeanwhileAccount::serverName()
{
    return pluginData(protocol(),"Server");
}

int MeanwhileAccount::serverPort()
{
    return pluginData(protocol(),"Port").toInt();
}

void MeanwhileAccount::setServerName(const QString &server)
{
    setPluginData(protocol(), "Server", server);
}

void MeanwhileAccount::setServerPort(int port)
{
    setPluginData(protocol(), "Port", QString::number(port));
}

void MeanwhileAccount::slotMesgReceived(
                            const QString &fromUser,
                            const QString &msg)
{
    MeanwhileContact *contact = static_cast<MeanwhileContact *>(contacts()[fromUser]);
    if(!contact)
        addContact( fromUser, 0L, Kopete::Account::DontChangeKABC );

    contact = static_cast<MeanwhileContact *>(contacts()[fromUser]);
    // Create a Kopete::Message
    Kopete::ContactPtrList contactList;
    contactList.append( myself() );
    Kopete::Message newMessage( contact, contactList, msg, Kopete::Message::Inbound );

    // Add it to the manager
    Kopete::ChatSession *mm = contact->manager(Kopete::Contact::CanCreate);
    mm->appendMessage(newMessage);
}

void MeanwhileAccount::slotUserTyping(
                            const QString &user,
                            bool isTyping)
{
    MeanwhileContact *contact = static_cast<MeanwhileContact *>(contacts()[user]);
    if(!contact)
        addContact( user, 0L, Kopete::Account::DontChangeKABC );

    contact = static_cast<MeanwhileContact *>(contacts()[user]);
    // Create a Kopete::Message
    Kopete::ContactPtrList contactList;
    contactList.append( myself() );

    // Add it to the manager
    Kopete::ChatSession *mm = contact->manager(Kopete::Contact::CanCreate);
    mm->receivedTypingMsg(contact, isTyping);
}

void MeanwhileAccount::meanwhileChangeStatus()
{
    bool ok;
    statusMesg = KInputDialog::getText( i18n( "Change Status Message - Meanwhile Plugin" ),
        i18n( "Enter the message to show under your status:" ),
        statusMesg, &ok );

    if ( ok )
    {
        if ( server!=NULL )
            server->changeStatus(statusMesg);
    }
}

void MeanwhileAccount::slotServerNotification(const QString &mesg)
{
    KMessageBox::queuedMessageBox(
                        0, KMessageBox::Error ,
                        mesg,
                        i18n( "Meanwhile Plugin: Message from server" ),
                        KMessageBox::Notify );
}

void MeanwhileAccount::slotServerDead()
{
    delete server;
    server = NULL;
    meanwhileGoOffline();
}

void MeanwhileAccount::setOnlineStatus( const Kopete::OnlineStatus & status )
{
 	if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Online )
		connect( status );
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Offline )
		disconnect();
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Away )
		setAway( true, QString::null );
}


#include "meanwhileaccount.moc"
