/*
    meanwhileaccount.h - meanwhile account

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
#ifndef MEANWHILEACCOUNT_H
#define MEANWHILEACCOUNT_H

#include <kopetepasswordedaccount.h>

class MeanwhileServer;
class MeanwhileProtocol;
class MeanwhilePlugin;

class MeanwhileAccount : public Kopete::PasswordedAccount
{
    Q_OBJECT
public:
    MeanwhileAccount(   MeanwhileProtocol *parent,
                        const QString &accountID,
                        const char *name = 0 );

    ~MeanwhileAccount();

    virtual bool createContact(
                        const QString &contactId,
                        Kopete::MetaContact *parentContact);

    virtual void connectWithPassword(const QString &password);

    virtual void disconnect();

    virtual void setAway(bool away,
                        const QString &reason);

    virtual KActionMenu *actionMenu();

    QString serverName();
    int     serverPort();
    void    setServerName(const QString &server);
    void    setServerPort(int port);
    void    setPlugin(MeanwhilePlugin *plugin);

    MeanwhileServer *server;
    MeanwhilePlugin *infoPlugin;

protected slots:
    void meanwhileGoOnline();
    void meanwhileGoAway();
    void meanwhileGoOffline();
    void meanwhileGoDND();
    void meanwhileChangeStatus();

public slots:
    void slotLoginDone();
    void slotMesgReceived(const QString &fromUser,
                          const QString &msg);
    void slotUserTyping(  const QString &user,
                          bool isTyping);
    void slotServerNotification(const QString &mesg);
    void slotServerDead();

private:
    void initServer();
    void meanwhileGoAway(const QString &statusmsg);
    QString statusMesg;
};

#endif
