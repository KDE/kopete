/*
    meanwhilelibrary.cpp - interface to the 'C' meanwhile library

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

#include <string.h>
#include <stdlib.h>
#include <kmessagebox.h>
#include <klocale.h>

#define MEANWHILELIBRARY_CPP

#include "meanwhilelibrary.h"
#include "meanwhileprotocol.h"
#include "meanwhileserver.h"
#include "meanwhilecontact.h"

#define HERE printf("siva: %s-%d\n",__FILE__,__LINE__);

extern "C"
{
#include <meanwhile/channel.h>
#include <meanwhile/message.h>
#include <meanwhile/mw_error.h>
#include <meanwhile/service.h>
#include <meanwhile/session.h>
#include <meanwhile/srvc_aware.h>
#include <meanwhile/srvc_conf.h>
#include <meanwhile/srvc_im.h>

}

void crash()
{
int *i=0;
*i=5;
}

#define ADVERTISE_KOPETE(s) (s+" *using kopete")
void prettyprint(const char *buffer,int c)
{
    int i,j;
    i=j=0;
    while(i<c)
    {
        printf("%.2hhx",*(buffer+i));
        i++;
        if (i%30 == 0)
        {
            printf(" ");
            for(;i!=j;j++)
                printf("%c",(*(buffer+j)<32)?'.':*(buffer+j));
            printf("\n");
        }
    }
    printf(" ");
    for(;i!=j;j++)
        printf("%c",(*(buffer+j)==0)?'.':*(buffer+j));
    printf("\n");
}

struct kopete_handler
{
    struct mwSessionHandler public_data;
    struct 
    {
        MeanwhileLibrary *library;
    } private_data;
};

int MeanwhileLibrary::Away = mwStatus_AWAY;
int MeanwhileLibrary::Active = mwStatus_ACTIVE;
int MeanwhileLibrary::Idle = mwStatus_IDLE;
int MeanwhileLibrary::Busy = mwStatus_BUSY;

#define REMOVE_UNUSED_VAR_WARNING(x) x=NULL
#define NOT_IMPLEMENTED NULL

#define MEANWHILE_HOOK(func,call)                       \
void MeanwhileLibrary::_ ## func                        \
{                                                       \
    MeanwhileLibrary *lib =                             \
	((struct kopete_handler *)s->handler)->         \
		private_data.library;                   \
    lib-> call;                                         \
}                                                       \
                                                        \
void MeanwhileLibrary:: func 

#define MEANWHILE_HOOK_SVC(func,call)                   \
void MeanwhileLibrary::_ ## func                        \
{                                                       \
    struct mwSession *s =                               \
	((struct mwService*)srvc)->session;             \
    MeanwhileLibrary *lib =                             \
	((struct kopete_handler *)s->handler)->         \
		private_data.library;                   \
    lib-> call;                                         \
}                                                       \
                                                        \
void MeanwhileLibrary:: func 

#define MEANWHILE_HOOK_CONF(func,call)                  \
void MeanwhileLibrary::_ ## func                        \
{                                                       \
    struct mwSession *s =                               \
	((struct mwService*)conf->srvc)->session;       \
    MeanwhileLibrary *lib =                             \
	((struct kopete_handler *)s->handler)->         \
		private_data.library;                   \
    lib-> call;                                         \
}                                                       \
                                                        \
void MeanwhileLibrary:: func 

MEANWHILE_HOOK(on_start(struct mwSession *s),
               on_start(s))
{
HERE
    onStart_sendHandshake(s);
}


MEANWHILE_HOOK(on_loginAck(
                    struct mwSession *s, 
                    struct mwMsgLoginAck *msg) ,
               on_loginAck(s,msg))

{
HERE
    REMOVE_UNUSED_VAR_WARNING(msg);
    
    mwService_start(MW_SERVICE(srvc_aware));

    struct mwUserStatus stat = { mwStatus_ACTIVE, 0, NULL };
    emit loginDone();
    mwSession_setUserStatus(s, &stat);
}

MEANWHILE_HOOK(on_stop(
                    struct mwSession *s,
                    guint32 reason) ,
               on_stop(s,reason))
{
HERE
    REMOVE_UNUSED_VAR_WARNING(s);

printf("siva close : %s\n",mwError(reason));
    if (reason & ERR_FAILURE)
    {
        emit serverNotificationReceived(QString(mwError(reason)));
    }

}

MEANWHILE_HOOK(on_setUserStatus(
                    struct mwSession *s,
                    struct mwMsgSetUserStatus *msg) ,
               on_setUserStatus(s,msg))
{
HERE
printf("sss %p %p",s,msg);
printf("siva: status of [%s] to %x [%s]\n",
            (s->login.user_id==NULL)?"null":s->login.user_id,
            (msg->status.status),
            (msg->status.desc==NULL)?"null":msg->status.desc);
        struct mwAwareIdBlock id = 
                { 
                    mwAware_USER,
                    s->login.user_id, 
                    s->login.community 
                };

        mwServiceAware_setStatus(srvc_aware, &id, &msg->status);
}

MEANWHILE_HOOK_SVC(got_aware(
                    struct mwAwareList *list, 
                    struct mwSnapshotAwareIdBlock *idb, 
                    struct mwService *srvc),
                got_aware(list,idb,srvc))
{
HERE
    REMOVE_UNUSED_VAR_WARNING(list);
    REMOVE_UNUSED_VAR_WARNING(srvc);

    time_t idletime;
    idletime = 0; 
    if (idb->status.status == mwStatus_IDLE)
    	idletime = (idb->status.time == 0xdeadbeef)?
                            0:idb->status.time;
    
//printf("siva: user status change for %s",idb->id.user);
    emit userStatusChanged(QString(idb->id.user),
                        idb->online,
                        idletime,
                        idb->status.status,
                        QString(idb->status.desc));
}

MEANWHILE_HOOK_SVC(got_error(
                    struct mwServiceIM *srvc, 
                    struct mwIdBlock *who, 
                    unsigned int err) ,
               got_error(srvc,who,err))
{
HERE
    REMOVE_UNUSED_VAR_WARNING(srvc);

    emit mesgSendError(QString(who->user),QString(mwError(err)));
}

MEANWHILE_HOOK_SVC(got_text(
                    struct mwServiceIM *srvc, 
                    struct mwIdBlock *who, 
                    const char *text) ,
               got_text(srvc,who,text))
{
HERE
    REMOVE_UNUSED_VAR_WARNING(srvc);

    emit mesgReceived(QString(who->user),QString(text));
}

MEANWHILE_HOOK_SVC(got_typing(
                    struct mwServiceIM *srvc, 
                    struct mwIdBlock *who, 
                    int typing) ,
               got_typing(srvc,who,typing))
{
HERE
    REMOVE_UNUSED_VAR_WARNING(srvc);

    emit userTyping(QString(who->user),(typing!=0));
}

MEANWHILE_HOOK_CONF(got_invite(
                    struct mwConference *conf,
                    struct mwIdBlock *id,
                    const char *text) ,
               got_invite(conf,id,text))
{
HERE
    emit invitedToConf(conf,
                QString(id->user),
                QString(conf->name),
                QString(conf->topic),
                QString(text));
}

MEANWHILE_HOOK_CONF(got_welcome(
                    struct mwConference *conf,
                    struct mwIdBlock *members,
                    unsigned int count) ,
               got_welcome(conf,members,count))
{
HERE
    QString *chatMembers = new QString[count];
    unsigned int i;
    for(i=0;i<count;i++,members++)
    {
        chatMembers[i] = QString(members->user);
    }
    emit welcomeReceived(conf,chatMembers,count);
}

MEANWHILE_HOOK_CONF(got_closed(
                    struct mwConference *conf) ,
               got_closed(conf))
{
HERE
    emit confClosed(conf);
}

MEANWHILE_HOOK_CONF(got_join(
                    struct mwConference *conf, 
                    struct mwIdBlock *id) ,
               got_join(conf,id))
{
HERE
    emit userJoinedConf(conf,QString(id->user));
}


MEANWHILE_HOOK_CONF(got_part(
                    struct mwConference *conf, 
                    struct mwIdBlock *id) ,
               got_part(conf,id))
{
HERE
    emit userLeftConf(conf,QString(id->user));
}

MEANWHILE_HOOK_CONF(got_conf_text(
                    struct mwConference *conf, 
                    struct mwIdBlock *id,
                    const char *text) ,
               got_conf_text(conf,id,text))
{
HERE
    emit confMesgReceived(conf,QString(id->user),
                          QString(text));
}

MEANWHILE_HOOK_CONF(got_conf_typing(
                    struct mwConference *conf, 
                    struct mwIdBlock *id,
                    int typing) ,
               got_conf_typing(conf,id,typing))
{
HERE
    emit confUserTyping(conf,
                    QString(id->user),
                    typing!=0);
}


MEANWHILE_HOOK(on_handshakeAck(
                    struct mwSession *s,
                    struct mwMsgHandshakeAck *msg) ,
               on_handshakeAck(s,msg))
{
HERE
    onHandshakeAck_sendLogin(s, msg);
}

MeanwhileLibrary::MeanwhileLibrary(QString server, int port)
{
HERE
    newSession();

    sock2server = getConnectedSocket(server,port);
    if (sock2server == NULL);
}

MeanwhileLibrary::~MeanwhileLibrary()
{
HERE
    free(handler);
    if (session != NULL)
        mwSession_free(session);
    if (sock2server != NULL)
        delete sock2server; 
}

bool MeanwhileLibrary::bad()
{
HERE
    return (sock2server == NULL);
}

void MeanwhileLibrary::login(QString username, QString passwd)
{
HERE
    session->login.user_id = strdup(username.ascii());
    session->auth.password = strdup(passwd.ascii());

    mwSession_start(session);
}

void MeanwhileLibrary::logoff()
{
HERE
    if(session) 
      mwSession_stop(session, ERR_SUCCESS);
}

int MeanwhileLibrary::_writeToSocket(
                        struct mwSessionHandler *handler,
                        const char *buffer,
                        unsigned int count)
{
HERE
    return ((kopete_handler*)handler)->private_data.library->
			writeToSocket(handler,buffer,count);
}

int MeanwhileLibrary::writeToSocket(
                        struct mwSessionHandler *handler,
                        const char *buffer,
                        unsigned int count)
{
HERE
    REMOVE_UNUSED_VAR_WARNING(handler);
    prettyprint(buffer,count);
    int returnval = sock2server->writeBlock(buffer,count);
    sock2server->flush();
    return returnval;
}

void MeanwhileLibrary::_closeSocket(
                        struct mwSessionHandler *handler) 
{
HERE
    return ((kopete_handler*)handler)->private_data.library->
                       closeSocket(handler);
}

void MeanwhileLibrary::closeSocket(
                        struct mwSessionHandler *handler)
{
HERE
    REMOVE_UNUSED_VAR_WARNING(handler);

    QObject::disconnect(sock2server, SIGNAL(closed(int)),
                     this,SLOT(slotSocketClosed(int)));
    sock2server->flush();
    sock2server->closeNow();
    emit connectionLost();
}

void MeanwhileLibrary::newSession()
{
HERE
    /* session setup */
    session = mwSession_new();
    session->on_start = _on_start;
    session->on_stop = _on_stop;
    
    session->on_channelOpen = NOT_IMPLEMENTED;
    session->on_channelClose = NOT_IMPLEMENTED;
    
    session->on_handshake = NOT_IMPLEMENTED;
    session->on_handshakeAck = _on_handshakeAck;
    session->on_login = NOT_IMPLEMENTED;
    session->on_loginRedirect = NOT_IMPLEMENTED;
    session->on_loginContinue = NOT_IMPLEMENTED;
    session->on_loginAck = _on_loginAck;
    
    session->on_setPrivacyInfo = NOT_IMPLEMENTED;
    session->on_setUserStatus = _on_setUserStatus;
    session->on_admin = NOT_IMPLEMENTED;

    /* awareness service setup */
    srvc_aware = mwServiceAware_new(session);
    aware_list = mwAwareList_new(srvc_aware);
    mwAwareList_setOnAware(aware_list,
                        (mwAwareList_onAwareHandler)_got_aware,
                        srvc_aware);
    mwSession_putService(session, (struct mwService *) srvc_aware);

    /* im  */
    srvc_im = mwServiceIM_new(session);
    srvc_im->got_error = _got_error;
    srvc_im->got_text = _got_text;
    srvc_im->got_typing = _got_typing;
    mwSession_putService(session, (struct mwService *) srvc_im);

    /* conference */
    struct mwServiceConf *srvc_conf;
    srvc_conf = mwServiceConf_new(session);
    srvc_conf->got_invite = _got_invite;
    srvc_conf->got_welcome = _got_welcome;
    srvc_conf->got_closed = _got_closed;
    srvc_conf->got_join = _got_join;
    srvc_conf->got_part = _got_part;
    srvc_conf->got_text = _got_conf_text;
    srvc_conf->got_typing = _got_conf_typing;
    mwSession_putService(session, (struct mwService *) srvc_conf);

    /* socket writer */
    handler = (struct mwSessionHandler*)
                    malloc(sizeof(struct kopete_handler));
    ((struct kopete_handler*)handler)->public_data.write = _writeToSocket;
    ((struct kopete_handler*)handler)->public_data.close = _closeSocket;
    ((struct kopete_handler*)handler)->private_data.library = this;
    session->handler = (struct mwSessionHandler*)handler;
}

KExtendedSocket *MeanwhileLibrary::getConnectedSocket(QString server, int port)
{
HERE
    KExtendedSocket *sock = new KExtendedSocket( server, port, 
                                        KExtendedSocket::bufferedSocket );
    int error = sock->connect();
    if (error)
    {
        KMessageBox::queuedMessageBox( 
                        0, KMessageBox::Error , 
                        i18n( "Could not connect to server"),
                        i18n( "Meanwhile Plugin" ), 
                        KMessageBox::Notify );
        delete sock;
        return NULL;
    }
    sock->enableRead(true);
//    sock->enableWrite(true);
    QObject::connect(sock, SIGNAL(readyRead()) , 
                     SLOT(slotSocketReader()));
    QObject::connect(sock, SIGNAL(closed(int)),
                     SLOT(slotSocketClosed(int)));
    return sock;
}

void MeanwhileLibrary::slotSocketReader()
{
HERE
    char buffer[4000];
    int readAmount;
    readAmount = sock2server->readBlock(buffer,4000);
    if (readAmount < 0)
        return;
prettyprint(buffer,readAmount);
    mwSession_recv(session, buffer, (unsigned int) readAmount);
}

void MeanwhileLibrary::slotSocketClosed(int reason)
{
    reason = 0; //  something to eat the compile warning
/*    if (reason == KExtendedSocket::involuntary)
    { */
        emit serverNotificationReceived(QString("Server closed connection"));
        emit connectionLost();
//    }
}

void MeanwhileLibrary::addContacts(const QDict<KopeteContact>& contacts)
{
HERE
    QDictIterator<KopeteContact> it(contacts); 

    struct mwAwareIdBlock *buddies,*aBuddy;
    int count;

    count = contacts.count();
    aBuddy = buddies = 
                (struct mwAwareIdBlock*)
			malloc(sizeof(struct mwAwareIdBlock)*count);
    memset(buddies,0,sizeof(struct mwIdBlock)*count);
   
    for( ; it.current(); ++it, aBuddy++ )
    {
        MeanwhileContact *contact = 
                static_cast<MeanwhileContact *>(it.current());
        aBuddy->user = (gchar*)contact->meanwhileId.ascii();
        aBuddy->community = NULL;
	aBuddy->type = mwAware_USER;
    }

    mwAwareList_addAware(aware_list, buddies, count);

    free(buddies);
HERE
}

void MeanwhileLibrary::addContact(const QString & contact)
{
HERE
    struct mwAwareIdBlock aBuddy = 
		{ mwAware_USER, (char*) contact.ascii(), NULL };
    
    mwAwareList_addAware(aware_list, &aBuddy, 1);
}

int MeanwhileLibrary::sendIm(const QString &toUser, const QString &msg)
{
HERE
  struct mwIdBlock t = { (char *) toUser.ascii(), NULL };

  printf("sending data to user [%s]\n%s\n",toUser.ascii(),msg.ascii());
prettyprint(msg.ascii(),14);
  return !mwServiceIM_sendText(srvc_im, &t, msg.ascii());
}

void MeanwhileLibrary::sendTyping(const QString &toUser, bool isTyping)
{
HERE
  struct mwIdBlock t = { (char *) toUser.ascii(), NULL };
  mwServiceIM_sendTyping(srvc_im, &t, isTyping);
}

void MeanwhileLibrary::setState(int state,const QString &stateMsg)
{
HERE
    struct mwUserStatus stat;
    mwUserStatus_clone(&stat, &session->status);

    free(stat.desc);

    stat.status = (mwStatusType)state;
    stat.desc = (gchar*)strdup(ADVERTISE_KOPETE(stateMsg).ascii());

    mwSession_setUserStatus(session,&stat);
    mwUserStatus_clear(&stat);
}

void MeanwhileLibrary::setStatusMesg(const QString &statusMesg)
{
HERE
    if(statusMesg == QString::null) 
        return;
    struct mwUserStatus stat;
    mwUserStatus_clone(&stat, &session->status);

    free(stat.desc);
    stat.desc = (gchar*)strdup(ADVERTISE_KOPETE(statusMesg).ascii());

    mwSession_setUserStatus(session,&stat);
    mwUserStatus_clear(&stat);
}

#include "meanwhilelibrary.moc"
