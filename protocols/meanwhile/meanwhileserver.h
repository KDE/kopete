/*
    meanwhileserver.h - server interface to an account

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
#ifndef MEANWHILESERVER_H
#define MEANWHILESERVER_H

#include "kopetecontact.h"
#include "meanwhilelibrary.h"
#include "qdict.h"

class MeanwhileServer : public QObject
{
    Q_OBJECT

    public:
        MeanwhileServer(QString server, int port);
        ~MeanwhileServer();
        bool bad();
        int login(QString username, QString passwd);
        void logoff();
        void addContacts(const QDict<KopeteContact>& contacts);
        void addContact(KopeteContact *contact,
                        const QDict<KopeteContact>& contacts);
        int sendIm(KopeteContact *contact, const QString & msg);
        void sendTyping(KopeteContact *contact, bool isTyping);
        void goAway(const QString &awayMsg);
        void goDND(const QString &dndMsg);
        void goActive(const QString &activeMsg);
        void changeStatus(const QString &statusMesg);

    signals:
        void loginDone();
        void mesgReceived(const QString &fromUser,
                          const QString &mesg);
        void userTyping(  const QString & user,
                          bool isTyping);
        void connectionLost();
        void notificationAvailable(const QString &reason);

    private:
        MeanwhileLibrary *library;
        QString server;
        int port;
        QDict<KopeteContact> contacts;

    protected slots:
        void slotLoginDone();
        void slotConnectionLost();
        void slotServerNotification(const QString &errMesg);
        void slotUserStatusChanged(
                                const QString & userid,
                                bool isOnline,
                                time_t idletime,
                                int status,
                                const QString & statusDesc);
        void slotMesgReceived(
                                const QString & fromUser,
                                const QString & msg);
        void slotUserTyping(
                                const QString & user,
                                bool isTyping);
};

#endif
