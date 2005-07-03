/*
    meanwhileaccount.h - meanwhile account

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
#ifndef MEANWHILEACCOUNT_H
#define MEANWHILEACCOUNT_H

#include <kopetepasswordedaccount.h>
#include "meanwhileprotocol.h"
#include "meanwhileplugin.h"

class MeanwhileLibrary;

class MeanwhileAccount : public Kopete::PasswordedAccount
{
    Q_OBJECT
public:
    MeanwhileAccount(   MeanwhileProtocol *parent,
                        const QString &accountID,
                        const char *name = 0L);

    ~MeanwhileAccount();

    virtual bool createContact(const QString &contactId,
                        Kopete::MetaContact *parentContact);

    virtual void connectWithPassword(const QString &password);

    virtual void disconnect();
    virtual void disconnect(Kopete::Account::DisconnectReason reason);

    virtual void setAway(bool away,
                        const QString &reason);

    virtual KActionMenu *actionMenu();

    QString serverName();
    int     serverPort();
    void    setServerName(const QString &server);
    void    setServerPort(int port);
    void    setPlugin(MeanwhilePlugin *plugin);

    MeanwhilePlugin *infoPlugin;

    /**
     * Get a reference to the meanwhile library object
     */
    MeanwhileLibrary *library();

protected slots:
    void meanwhileGoOnline();
    void meanwhileGoAway();
    void meanwhileGoOffline();
    void meanwhileGoDND();
    void meanwhileChangeStatus();

public slots:
    void slotLoginDone();
    void slotServerNotification(const QString &mesg);
    void slotConnectionLost();

    /** Reimplemented from Kopete::Account */
    void setOnlineStatus(const Kopete::OnlineStatus& status,
            const QString &reason = QString::null);

private:
    void initLibrary();
    void meanwhileGoAway(const QString &statusmsg);
    QString statusMesg;

    /** The interface to the libmeanwhile library */
    MeanwhileLibrary *m_library;
};

#endif
