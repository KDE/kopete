/*
    meanwhileaccount.cpp - a meanwhile account

    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>
    Copyright (c) 2005      by Jeremy Kerr <jk@ozlabs.org>

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
#include "meanwhileplugin.h"
#include "meanwhileaccount.h"
#include "meanwhilecontact.h"
#include "meanwhilelibrary.h"
#include "kopetechatsession.h"
#include "kopetepassword.h"

#include <kaction.h>
#include <kmenu.h>
#include <klocale.h>
#include <kconfigbase.h>
#include "kopeteaway.h"
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <q3dict.h>

MeanwhileAccount::MeanwhileAccount(
                        MeanwhileProtocol *parent,
                        const QString &accountID,
                        const char *name)
    : Kopete::PasswordedAccount(parent, accountID, 0, name)
{
    HERE;
    //signal(SIGSEGV,SIG_DFL);
    setMyself(new MeanwhileContact(accountId(), accountId(), this, 0L));
    myself()->setOnlineStatus(MeanwhileProtocol::protocol()->meanwhileOffline);
    m_library = 0L;
    infoPlugin = new MeanwhilePlugin();
}

void MeanwhileAccount::initLibrary()
{
}

MeanwhileLibrary *MeanwhileAccount::library()
{
    return m_library;
}

MeanwhileAccount::~MeanwhileAccount()
{
    meanwhileGoOffline();
    if (m_library)
        delete m_library;
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
    if ((newContact != 0L) && (m_library != 0L)
        && (myself()->onlineStatus() !=
                MeanwhileProtocol::protocol()->meanwhileOffline))
        m_library->addContact(newContact);

    return newContact != 0L;
}

void MeanwhileAccount::connectWithPassword(const QString &password)
{
    if (password.isEmpty()) {
        disconnect(Kopete::Account::Manual);
        return;
    }

    if (m_library == 0L) {
        m_library = new MeanwhileLibrary(this);
        if (!m_library) {
            mwDebug() << "library creation failed" << endl;
            return;
        }

        QObject::connect(m_library, SIGNAL(loginDone()),
                this, SLOT(slotLoginDone()));
        QObject::connect(m_library, SIGNAL(connectionLost()),
                this, SLOT(slotConnectionLost()));
        QObject::connect(m_library,
                SIGNAL(serverNotification(const QString &)),
                this, SLOT(slotServerNotification(const QString&)));
    }
        
    if (!m_library->isConnected()) {
        m_library->login();
    }
}

void MeanwhileAccount::disconnect()
{
    disconnect(Manual);
}

void MeanwhileAccount::disconnect(Kopete::Account::DisconnectReason reason)
{
    if (m_library != 0L  && m_library->isConnected())
        m_library->logoff();

    if (m_library != 0L) {
        delete m_library;
        m_library = 0L;
    }

    myself()->setOnlineStatus(MeanwhileProtocol::protocol()->meanwhileOffline);
    setAllContactsStatus(MeanwhileProtocol::protocol()->meanwhileOffline);
    disconnected(reason);
    emit isConnectedChanged();
}

void MeanwhileAccount::setAway(bool away, const QString &reason)
{
    if (away)
        meanwhileGoAway(reason);
    else
        meanwhileGoOnline();
}

KActionMenu * MeanwhileAccount::actionMenu()
{
    KActionMenu * theMenu =
            new KActionMenu(accountId(),
                            myself()->onlineStatus().iconFor(this),
                            this);
    theMenu->popupMenu()->insertTitle(
                            myself()->icon(),
                            i18n("Meanwhile (%1)").arg(accountId()));
    theMenu->insert(
           new KAction( i18n( "Go Online" ),
                        MeanwhileProtocol::protocol()->meanwhileOnline.iconFor(this),
                        0, this, SLOT(meanwhileGoOnline()), this, "meanwhileGoOnline"));

    theMenu->insert(
           new KAction( i18n( "Go Offline" ),
                        MeanwhileProtocol::protocol()->meanwhileOffline.iconFor(this),
                        0, this, SLOT(meanwhileGoOffline()), this, "meanwhileGoOffline"));

    theMenu->insert(
           new KAction( i18n( "Go Away" ),
                        MeanwhileProtocol::protocol()->meanwhileAway.iconFor(this),
                        0, this, SLOT(meanwhileGoAway()), this, "meanwhileGoAway"));

    theMenu->insert(
           new KAction( i18n( "Mark as Busy" ),
                        MeanwhileProtocol::protocol()->meanwhileBusy.iconFor(this),
                        0, this, SLOT(meanwhileGoDND()), this, "meanwhileGoDND"));

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
    HERE;
    if (m_library != 0L && m_library->isConnected())
        m_library->setState(MeanwhileProtocol::protocol()->meanwhileOnline);
    else
        connect(MeanwhileProtocol::protocol()->meanwhileOnline);
}

void MeanwhileAccount::meanwhileGoOffline()
{
    disconnect();
}

void MeanwhileAccount::meanwhileGoAway()
{
    meanwhileGoAway(Kopete::Away::getInstance()->message());
}

void MeanwhileAccount::meanwhileGoAway(const QString &statusmsg)
{
    if ((m_library != 0L) && (myself()->onlineStatus() !=
            MeanwhileProtocol::protocol()->meanwhileOffline))
        m_library->setState(MeanwhileProtocol::protocol()->meanwhileAway,
                statusmsg);
}

void MeanwhileAccount::meanwhileGoDND()
{
    if ((m_library != 0L) && (myself()->onlineStatus() !=
            MeanwhileProtocol::protocol()->meanwhileOffline))
        m_library->setState(MeanwhileProtocol::protocol()->meanwhileBusy);
}

void MeanwhileAccount::slotLoginDone()
{
    myself()->setOnlineStatus(MeanwhileProtocol::protocol()->meanwhileOnline);
    statusMesg = QString("I am active");
    m_library->setState(MeanwhileProtocol::protocol()->meanwhileOnline);
    m_library->addContacts(contacts());
    emit isConnectedChanged();
}

QString MeanwhileAccount::serverName()
{
    return configGroup()->readEntry("Server");
}

int MeanwhileAccount::serverPort()
{
    return configGroup()->readNumEntry("Port");
}

void MeanwhileAccount::setServerName(const QString &server)
{
    configGroup()->writeEntry("Server", server);
}

void MeanwhileAccount::setServerPort(int port)
{
    configGroup()->writeEntry("Port", port);
}

void MeanwhileAccount::meanwhileChangeStatus()
{
    bool ok;
    statusMesg = KInputDialog::getText(
            i18n("Change Status Message - Meanwhile Plugin"),
            i18n("Enter the message to show under your status:"),
            statusMesg, &ok);

    if (ok && m_library != 0L)
        m_library->setStatusMesg(statusMesg);
}

void MeanwhileAccount::slotServerNotification(const QString &mesg)
{
    KMessageBox::queuedMessageBox(
                        0, KMessageBox::Error ,
                        mesg,
                        i18n( "Meanwhile Plugin: Message from server" ),
                        KMessageBox::Notify );
}

void MeanwhileAccount::slotConnectionLost()
{
    delete m_library;
    m_library = 0L;
    meanwhileGoOffline();
}

void MeanwhileAccount::setOnlineStatus(const Kopete::OnlineStatus & status,
        const QString &reason)
{
    Kopete::OnlineStatus mystatus = myself()->onlineStatus().status();

    if (mystatus == Kopete::OnlineStatus::Offline
            && status.status() == Kopete::OnlineStatus::Online )
        connect(status);

    else if (mystatus != Kopete::OnlineStatus::Offline
            && status.status() == Kopete::OnlineStatus::Offline )
        disconnect();

    else if (mystatus != Kopete::OnlineStatus::Offline
            && status.status() == Kopete::OnlineStatus::Away )
        setAway(true, reason);
}


#include "meanwhileaccount.moc"
