/*
    meanwhilelibrary.h - interface to the 'C' meanwhile library

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
#ifndef MEANWHILELIBRARY_H
#define MEANWHILELIBRARY_H

#include <qptrlist.h>
#include <qdict.h>
#include <qobject.h>

#include <time.h>
#include <ghash.h>
#include <kextsock.h>

#include "kopetecontact.h"

class MeanwhileLibrary : public QObject
{
    Q_OBJECT

    public:
        MeanwhileLibrary(QString server,int port);
        ~MeanwhileLibrary();
        bool bad();
        void login(QString username, QString passwd);
        void logoff();
        void addContacts(const QDict<KopeteContact> &contacts);
        void addContact(const QString &contact);
        int sendIm(const QString &toUser, const QString &msg);
        void sendTyping(const QString &toUser, bool isTyping);
        void setState(int state,const QString &awayMsg);
        void setStatusMesg(const QString &statusMesg);

        static int Active;
        static int Idle;
        static int Away;
        static int Busy;

    signals:
        void loginDone();
        void connectionLost();
        void userStatusChanged(
                            const QString &user,
                            bool    isOnline,
                            time_t  idletime,
                            int     fullStatus,
                            const QString &statusMessage);
        void mesgSendError(const QString &toUser,
                            const QString &errorString);
        void mesgReceived(const QString &fromUser,
                            const QString &mesg);
        void userTyping(const QString &user,bool isTyping);
        void invitedToConf(void *confId,
                            const QString &byUser,
                            const QString &confName,
                            const QString &confTopic, 
                            const QString &mesgText);
        void welcomeReceived(void *confId,
                            QString *confMembers,
                            int memberCount);
        void confClosed(void *confId);
        void userJoinedConf(void *confId,
                            const QString &user);
        void userLeftConf(void *confId,
                            const QString &user);
        void confMesgReceived(void *confId,
                            const QString &fromUser,
                            const QString &msgText);
        void confUserTyping(void *confId,
                            const QString &user,
                            bool isTyping);
        void serverNotificationReceived(const QString &mesgString);

#define DEFINE_MW_HOOK(func) \
    static void _ ## func;   \
    void func
    
    private:
        DEFINE_MW_HOOK(on_start(
                            struct mwSession *s));
        DEFINE_MW_HOOK(on_handshake(
                            struct mwSession *s, 
                            struct mwMsgHandshake *msg));
        DEFINE_MW_HOOK(on_loginAck(
                            struct mwSession *s, 
                            struct mwMsgLoginAck *msg));
        DEFINE_MW_HOOK(on_stop(
                            struct mwSession *session, 
                            guint32 reason));
        DEFINE_MW_HOOK(on_setUserStatus(
                            struct mwSession *s,
                            struct mwMsgSetUserStatus *msg));
        DEFINE_MW_HOOK(got_aware(
                            struct mwAwareList *list, 
                            struct mwSnapshotAwareIdBlock *idb, 
                            struct mwService *srvc));
        DEFINE_MW_HOOK(got_error(
                            struct mwServiceIM *srvc, 
                            struct mwIdBlock *who, 
                            unsigned int err));
        DEFINE_MW_HOOK(got_text(
                            struct mwServiceIM *srvc,
                            struct mwIdBlock *who,
                            const char *mesg));
        DEFINE_MW_HOOK(got_typing(
                            struct mwServiceIM *srvc, 
                            struct mwIdBlock *who, 
                            int typing));
        DEFINE_MW_HOOK(got_invite(
                            struct mwConference *conf, 
                            struct mwIdBlock *id,
                            const char *text));
        DEFINE_MW_HOOK(got_welcome(
                            struct mwConference *conf, 
                            struct mwIdBlock *members,
                            unsigned int count));
        DEFINE_MW_HOOK(got_closed(
                            struct mwConference *conf));
        DEFINE_MW_HOOK(got_join(
                            struct mwConference *conf, 
                            struct mwIdBlock *id));
        DEFINE_MW_HOOK(got_part(
                            struct mwConference *conf, 
                            struct mwIdBlock *id));
        DEFINE_MW_HOOK(got_conf_text(
                            struct mwConference *conf, 
                            struct mwIdBlock *id,
                            const char *text));
        DEFINE_MW_HOOK(got_conf_typing(
                            struct mwConference *conf, 
                            struct mwIdBlock *id,
                            int typing));
        DEFINE_MW_HOOK(on_handshakeAck(
                            struct mwSession *s,
                            struct mwMsgHandshakeAck *msg));
        static int _writeToSocket(
                            struct mwSessionHandler *handler,
                            const char *buffer,
                            unsigned int count);
        int writeToSocket(
                            struct mwSessionHandler *handler,
                            const char *buffer,
                            unsigned int count);
        static void _closeSocket(
                            struct mwSessionHandler *handler);
        void closeSocket(
                            struct mwSessionHandler *handler);

        void newSession();

        KExtendedSocket *getConnectedSocket(QString server, int port);


        struct mwSession *session;
        struct mwServiceAware *srvc_aware;
        struct mwSessionHandler *handler;
        struct mwAwareList *aware_list; 
        mwServiceIM *srvc_im;
        KExtendedSocket *sock2server;

    public slots:
        void slotSocketReader();
        void slotSocketClosed(int reason);
};

#endif

