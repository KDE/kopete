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
#include "meanwhilesession.h"
#include "kopetechatsession.h"
#include "kopetecontactlist.h"
#include "kopetepassword.h"

#include <kaction.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <kconfigbase.h>
#include "kopeteaway.h"
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <qdict.h>

MeanwhileAccount::MeanwhileAccount(
                        MeanwhileProtocol *parent,
                        const QString &accountID,
                        const char *name)
    : Kopete::PasswordedAccount(parent, accountID, 0, name)
{
    HERE;
    m_meanwhileId = accountID;
    m_session = 0L;
    setMyself(new MeanwhileContact(m_meanwhileId, m_meanwhileId, this,
                Kopete::ContactList::self()->myself()));
    setOnlineStatus(parent->statusOffline);
    infoPlugin = new MeanwhilePlugin();
}

MeanwhileAccount::~MeanwhileAccount()
{
    if (m_session)
        delete m_session;
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
    MeanwhileContact* newContact = new MeanwhileContact(contactId,
            parentContact->displayName(), this, parentContact);

    MeanwhileProtocol *p = static_cast<MeanwhileProtocol *>(protocol());

    if ((newContact != 0L) && (m_session != 0L)
        && (myself()->onlineStatus() !=
                p->statusOffline))
        m_session->addContact(newContact);

    return newContact != 0L;
}

void MeanwhileAccount::connectWithPassword(const QString &password)
{
    if (password.isEmpty()) {
        disconnect(Kopete::Account::Manual);
        return;
    }

    if (m_session == 0L) {
        m_session = new MeanwhileSession(this);
        if (m_session == 0L) {
            mwDebug() << "session creation failed" << endl;
            return;
        }

        QObject::connect(m_session,
                SIGNAL(sessionStateChange(Kopete::OnlineStatus)),
                this, SLOT(slotSessionStateChange(Kopete::OnlineStatus)));
        QObject::connect(m_session,
                SIGNAL(serverNotification(const QString &)),
                this, SLOT(slotServerNotification(const QString&)));

    }

    if (m_session == 0L) {
        mwDebug() << "No memory for session" << endl;
        return;
    }

    if (!m_session->isConnected() && !m_session->isConnecting())
        m_session->connect(password);

    m_session->setStatus(initialStatus());
}

void MeanwhileAccount::disconnect()
{
    disconnect(Manual);
}

void MeanwhileAccount::disconnect(Kopete::Account::DisconnectReason reason)
{
    if (m_session == 0L)
        return;

    MeanwhileProtocol *p = static_cast<MeanwhileProtocol *>(protocol());
    setAllContactsStatus(p->statusOffline);
    disconnected(reason);
    emit isConnectedChanged();

    delete m_session;
    m_session = 0L;
}

KActionMenu * MeanwhileAccount::actionMenu()
{
    KActionMenu *menu = Kopete::Account::actionMenu();

    menu->popupMenu()->insertSeparator();

#if 0
    menu->insert(new KAction(i18n("&Change Status Message"), QString::null, 0,
                this, SLOT(meanwhileChangeStatus()), this,
                "meanwhileChangeStatus"));
    //infoPlugin->addCustomMenus(theMenu);
#endif
    return menu;
}

QString MeanwhileAccount::getServerName()
{
    return configGroup()->readEntry("Server");
}

int MeanwhileAccount::getServerPort()
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

void MeanwhileAccount::setClientID(int client, int major, int minor)
{
    configGroup()->writeEntry("clientID", client);
    configGroup()->writeEntry("clientVersionMajor", major);
    configGroup()->writeEntry("clientVersionMinor", minor);
}

void MeanwhileAccount::resetClientID()
{
    configGroup()->deleteEntry("clientID");
    configGroup()->deleteEntry("clientVersionMajor");
    configGroup()->deleteEntry("clientVersionMinor");
}

bool MeanwhileAccount::getClientIDParams(int *clientID,
	int *verMajor, int *verMinor)
{
    bool custom_id = configGroup()->hasKey("clientID");

    MeanwhileSession::getDefaultClientIDParams(clientID, verMajor, verMinor);

    if (custom_id) {
	*clientID = configGroup()->readUnsignedNumEntry("clientID", *clientID);
	*verMajor = configGroup()->readUnsignedNumEntry("clientVersionMajor",
		    *verMinor);
	*verMinor = configGroup()->readUnsignedNumEntry("clientVersionMinor",
		    *verMinor);
    }

    return custom_id;
}

void MeanwhileAccount::slotServerNotification(const QString &mesg)
{
    KMessageBox::queuedMessageBox(0, KMessageBox::Error , mesg,
            i18n("Meanwhile Plugin: Message from server"), KMessageBox::Notify);
}

QString MeanwhileAccount::meanwhileId() const
{
    return m_meanwhileId;
}

void MeanwhileAccount::setAway(bool away, const QString &reason)
{
    MeanwhileProtocol *p = static_cast<MeanwhileProtocol *>(protocol());
    setOnlineStatus(away ? p->statusIdle : p->statusOnline, reason);
}

void MeanwhileAccount::setOnlineStatus(const Kopete::OnlineStatus &status,
        const QString &reason)
{
    HERE;
    Kopete::OnlineStatus oldstatus = myself()->onlineStatus();

    mwDebug() << "From: " << oldstatus.description() << "(" <<
        oldstatus.internalStatus() << "):" << oldstatus.isDefinitelyOnline()
                                              << endl;
    mwDebug() << "To:   " << status.description() << "(" <<
        status.internalStatus() << "):" << status.isDefinitelyOnline() << endl;

    if (oldstatus == status)
        return;

    if (!oldstatus.isDefinitelyOnline() && status.isDefinitelyOnline()) {
        connect();

    } else if (oldstatus.isDefinitelyOnline() && !status.isDefinitelyOnline()) {
        disconnect(Kopete::Account::Manual);

    } else if (m_session)
        /* todo: check session state? */
        m_session->setStatus(status, reason);

    else
        mwDebug() << "Trying to set status, but no session exists" << endl;

    /* we should set this on the callback below */
    //myself()->setOnlineStatus(status);
}

void MeanwhileAccount::syncContactsToServer()
{
    if (m_session != 0L)
        m_session->syncContactsToServer();
}

void MeanwhileAccount::slotSessionStateChange(Kopete::OnlineStatus status)
{
    HERE;
    Kopete::OnlineStatus oldstatus = myself()->onlineStatus();
    myself()->setOnlineStatus(status);

    if (status.isDefinitelyOnline() != oldstatus.isDefinitelyOnline()) {
        if (status.isDefinitelyOnline())
            m_session->addContacts(contacts());
        emit isConnectedChanged();
    }
}

MeanwhileSession *MeanwhileAccount::session()
{
    return m_session;
}

#include "meanwhileaccount.moc"
