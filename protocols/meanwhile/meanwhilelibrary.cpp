/*
    meanwhilelibrary.cpp - interface to the 'C' meanwhile library

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

#include <string.h>
#include <stdlib.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <kopetepassword.h>
#include "meanwhilelibrary.h"
#include "meanwhileprotocol.h"
//Added by qt3to4:
#include <Q3ValueList>

extern "C"
{
#include <meanwhile/mw_channel.h>
#include <meanwhile/mw_message.h>
#include <meanwhile/mw_error.h>
#include <meanwhile/mw_service.h>
#include <meanwhile/mw_session.h>
#include <meanwhile/mw_srvc_aware.h>
#include <meanwhile/mw_srvc_conf.h>
#include <meanwhile/mw_srvc_im.h>
#include <meanwhile/mw_cipher.h>
}

#define ADVERTISE_KOPETE(s) (s+" *using kopete")

/* for these macros:
 *  func - the name of the function
 *  args - the full arguments (including type) of the member (and static)
 *         function. Must be enclosed in brackets. The first argument will be
 *         used to get a reference to the library - it must be named as per
 *         the last word of the macro name (but in lower case)
 *  call - the arguments to the call to the non-static function. This is the
 *         same as 'args', but without the type delcarations
 */
 
#define MEANWHILE_HOOK_SESSION(func, args, call)                     \
void MeanwhileLibrary::_ ## func args                                \
{                                                                    \
    MeanwhileLibrary *lib = (MeanwhileLibrary *)                     \
        mwSession_getClientData(session);                            \
    if (lib)                                                         \
        lib-> func call;                                             \
    else                                                             \
        mwDebug() << "No client data for session in " #func << endl; \
}                                                                    \
void MeanwhileLibrary:: func args 

#define MEANWHILE_HOOK_SERVICE(func, args, call)                     \
void MeanwhileLibrary::_ ## func args                                \
{                                                                    \
    MeanwhileLibrary *lib = (MeanwhileLibrary *)                     \
        mwService_getClientData((struct mwService *)service);        \
    if (lib)                                                         \
        lib-> func call;                                             \
    else                                                             \
        mwDebug() << "No client data for service in " #func << endl; \
}                                                                    \
void MeanwhileLibrary:: func args 

#define MEANWHILE_HOOK_CONVERSATION(func, args, call)                \
void MeanwhileLibrary::_ ## func args                                \
{                                                                    \
    struct mwService *service = (struct mwService *)                 \
        mwConversation_getService(conv);                             \
    MeanwhileLibrary *lib = (MeanwhileLibrary *)                     \
        mwService_getClientData(service);                            \
    if (lib)                                                         \
        lib-> func call;                                             \
    else                                                             \
        mwDebug() << "No client data for conv. in " #func << endl;   \
}                                                                    \
void MeanwhileLibrary:: func args 

void MeanwhileLibrary::_stateChange(struct mwSession *session,
        enum mwSessionState state, guint32 data)
{
    HERE;
    MeanwhileLibrary *lib =
        (MeanwhileLibrary *)mwSession_getClientData(session);

    if (lib == 0L) {
        mwDebug() << "Invalid handler in stateChange callback"
            << endl;
        return;
    }

    switch (state) {
        case mwSession_LOGIN_ACK:
            lib->session_loginAck(session);
            break;

        case mwSession_STOPPED:
            lib->session_stop(session, data);
            break;

        default:
            mwDebug() << "Unhandled state change " << state << endl;
    }

}

void MeanwhileLibrary::_handler_clear(struct mwSession *)
{
    HERE;
}

void MeanwhileLibrary::session_loginAck(struct mwSession *s)
{
    HERE;
    connected = true;
    struct mwUserStatus stat = { mwStatus_ACTIVE, 0, 0L };
    mwSession_setUserStatus(s, &stat);

    /* get own nickname */
    struct mwLoginInfo *logininfo = mwSession_getLoginInfo(s);
    if (logininfo) {
        mwDebug() << "got login info. username: " << logininfo->user_name <<
            ", nickname: " << getNickName(logininfo) << endl;
        account->myself()->setNickName(getNickName(logininfo));
    } else
        mwDebug() << "no login info" << endl;

    emit loginDone();
}

void MeanwhileLibrary::session_stop(struct mwSession *, unsigned int status)
{
    HERE;
    connected = false;
    if (status & ERR_FAILURE) {
        if (status == INCORRECT_LOGIN)
            account->password().setWrong();
        char *reason = mwError(status);
        emit serverNotification(QString(reason));
        free(reason);
    }

    emit connectionLost();
}

/* aware attribute handlers */
void MeanwhileLibrary::_on_attrib(struct mwServiceAware *,
        struct mwAwareAttribute *)
{
    HERE;
}
void MeanwhileLibrary::_attrib_clear(struct mwServiceAware *)
{
    HERE;
}

/* aware list handlers */
void MeanwhileLibrary::_on_aware(struct mwAwareList *list,
        struct mwAwareSnapshot *id)
{
    HERE;
    MeanwhileLibrary *lib = (MeanwhileLibrary *)
        mwAwareList_getClientData(list);
    if (lib)
        lib->on_aware(list, id);
}

#define STATUS(VALUE,onlinestatus)                        \
    (snapshot->status.status==MeanwhileLibrary::VALUE)?                    \
            MeanwhileProtocol::protocol()->onlinestatus:
void MeanwhileLibrary::on_aware(struct mwAwareList *,
        struct mwAwareSnapshot *snapshot)
{
    HERE;
    MeanwhileContact *contact = static_cast<MeanwhileContact *>
        (account->contacts()[snapshot->id.user]);

    if (!contact)
        return;
    
    contact->setProperty(MeanwhileProtocol::protocol()->statusMessage, 
            snapshot->status.desc);
    contact->setProperty(MeanwhileProtocol::protocol()->awayMessage,
            snapshot->status.desc);

    Kopete::OnlineStatus status;
    if (snapshot->online) {
        switch (snapshot->status.status) {
        case MeanwhileLibrary::Away:
            status = MeanwhileProtocol::protocol()->meanwhileAway;
            break;
        case MeanwhileLibrary::Active:
            status = MeanwhileProtocol::protocol()->meanwhileOnline;
            break;
        case MeanwhileLibrary::Idle:
            status = MeanwhileProtocol::protocol()->meanwhileIdle;
            break;
        case MeanwhileLibrary::Busy:
            status = MeanwhileProtocol::protocol()->meanwhileBusy;
            break;
        default:
            status = MeanwhileProtocol::protocol()->meanwhileUnknown;
        }
    } else {
        status = MeanwhileProtocol::protocol()->meanwhileOffline;
    }
    contact->setOnlineStatus(status);

#if 0
    /* Commented out in previous kopete/meanwhile plugin for some reason,
     * but has still been ported to the new API.
     */
    time_t idletime = 0;
    if (snapshot->status.status == mwStatus_IDLE) {
    	idletime = (snapshot->status.time == 0xdeadbeef) ?
            0 : snapshot->status.time;
        if (idletime != 0) {
        contact->setStatusDescription(statusDesc + "[" +
                QString::number(idletime/60)+" mins]");
        }
    } else
        contact->setStatusDescription(snapshot->status.desc);
#endif
}

void MeanwhileLibrary::_on_aware_attrib(struct mwAwareList *,
        struct mwAwareIdBlock *,
        struct mwAwareAttribute *)
{
    mwDebug() << "_on_aware_attrib() called" << endl;
}
void MeanwhileLibrary::_aware_clear(struct mwAwareList *)
{
    mwDebug() << "_aware_clear() called" << endl;
}

QString MeanwhileLibrary::getNickName(struct mwLoginInfo *logininfo)
{
    if (logininfo == 0L || logininfo->user_name == 0L)
        return QString::null;

    /* try to find a friendly name. From what I've seen, usernames are in
     * the format:
     *  <userid> - <name>/<domain>/<domain>
     */
    QString name = logininfo->user_name;
    int index = name.find(" - ");
    if (index != -1)
        name = name.remove(0, index + 3);
    index = name.find('/');
    if (index != -1)
        name = name.left(index);
    
    return name;
}

MeanwhileContact *MeanwhileLibrary::convContact(
        struct mwConversation *conv)
{
    struct mwIdBlock *target = mwConversation_getTarget(conv);
    if (target == 0L || target->user == 0L) {
        return 0L;
    }
    QString user(target->user);

    MeanwhileContact *contact =
        static_cast<MeanwhileContact *>(account->contacts()[user]);

    if (!contact) {
        struct mwLoginInfo *logininfo = mwConversation_getTargetInfo(conv);
        QString name = getNickName(logininfo);
	account->addContact(user, name, 0L, Kopete::Account::Temporary);
        contact = static_cast<MeanwhileContact *>(account->contacts()[user]);
    }

    return contact;
}

struct MeanwhileLibrary::conv_data *MeanwhileLibrary::initConvData(
        struct mwConversation *conv, MeanwhileContact *contact)
{
    struct conv_data *conv_data = (struct conv_data *)malloc(sizeof *conv_data);
    if (!conv_data)
        return 0L;

    conv_data->library = this;
    /* grab a manager from the factory instead? */
    conv_data->chat = contact->manager();
    conv_data->chat->ref();
    conv_data->queue = new Q3ValueList<Kopete::Message>();

    mwConversation_setClientData(conv, conv_data, 0L);

    return conv_data;
}

/* conversation */
MEANWHILE_HOOK_CONVERSATION(conversation_opened,
    (struct mwConversation *conv),
    (conv))
{
    HERE;
    MeanwhileContact *contact = convContact(conv);
    if (!contact) {
        mwDebug() << "Couldn't find contact!" << endl;
        return;
    }

    struct conv_data *conv_data =
        (struct conv_data *)mwConversation_getClientData(conv);

    if (!conv_data && !(conv_data = initConvData(conv, contact))) {
        return;

    } else if (conv_data->queue && !conv_data->queue->isEmpty()) {
        /* send any messages that were waiting for the conversation to open */
        Q3ValueList<Kopete::Message>::iterator it;
        for (it = conv_data->queue->begin(); it != conv_data->queue->end();
                ++it) {
            mwConversation_send(conv, mwImSend_PLAIN,
                    (*it).plainBody().ascii());
            conv_data->chat->appendMessage(*it);
            conv_data->chat->messageSucceeded();
        }
        conv_data->queue->clear();
    }
}

MEANWHILE_HOOK_CONVERSATION(conversation_closed,
    (struct mwConversation *conv, guint32 err),
    (conv, err))
{
    HERE;
    /* @todo err is unused. This'll eat the warning */
    err = 0;
    struct conv_data *conv_data =
        (struct conv_data *)mwConversation_getClientData(conv);

    if (!conv_data)
        return;

    mwConversation_setClientData(conv, 0L, 0L);
    MeanwhileContact *contact = convContact(conv);
    if (!contact) {
        mwDebug() << "Couldn't find contact!" << endl;
        return;
    }

    conv_data->chat->removeContact(contact);
    conv_data->chat->deref();
    conv_data->chat = 0L;
    free(conv_data);
}

MEANWHILE_HOOK_CONVERSATION(conversation_recv,
    (struct mwConversation *conv, enum mwImSendType type,
        gconstpointer msg),
    (conv, type, msg))
{
    HERE;
    struct conv_data *conv_data =
        (struct conv_data *)mwConversation_getClientData(conv);

    if (!conv_data)
        return;

    MeanwhileContact *contact = convContact(conv);
    if (!contact) {
        mwDebug() << "Couldn't find contact!" << endl;
        return;
    }

    switch (type) {
    case mwImSend_PLAIN:
        {
            Kopete::Message message(contact, account->myself(),
                    QString((char *)msg), Kopete::Message::Inbound);
            conv_data->chat->appendMessage(message);
        }
        break;
    case mwImSend_TYPING:
        conv_data->chat->receivedTypingMsg(contact);
        break;
    default:
        mwDebug() << "Unable to handle message type " << type << endl;
    }
}

MEANWHILE_HOOK_SESSION(setUserStatus,
        (struct mwSession *session),
        (session))
{
    struct mwLoginInfo *login;
    struct mwUserStatus *status;

    HERE;
    login = mwSession_getLoginInfo(session);
    status = mwSession_getUserStatus(session);

    mwDebug() << "meanwhile status for " <<
        ((login->user_id==0L) ?
             "null" : login->user_id) <<
        " changed to " << (status->status) << endl;

    struct mwAwareIdBlock id = { 
        mwAware_USER,
        login->user_id, 
        login->community 
    };

    mwServiceAware_setStatus(srvc_aware, &id, status);
}

MeanwhileLibrary::MeanwhileLibrary(MeanwhileAccount *a)
{
    HERE;
    account = a;
    session = 0L;
    connected = false;
}

MeanwhileLibrary::~MeanwhileLibrary()
{
    HERE;
    if (connected)
        logoff();
    if (session)
        mwSession_free(session);
    if (socket)
        delete socket; 
}

bool MeanwhileLibrary::isConnected()
{
    HERE;
    return connected;
}

void MeanwhileLibrary::login()
{
    HERE;
    socket = getConnectedSocket();
    if (socket == 0L) {
        mwDebug() << "getConnectedSocket failed" << endl;
        return;
    }

    newSession();
    mwSession_setProperty(session, mwSession_AUTH_USER_ID,
                    strdup(account->accountId().ascii()), 0L);
    mwSession_setProperty(session, mwSession_AUTH_PASSWORD,
                    strdup(account->password().cachedValue().ascii()), 0L);
    mwSession_setProperty(session, mwSession_CLIENT_TYPE_ID,
                    GUINT_TO_POINTER(0x1003), 0L);
    mwSession_setProperty(session, mwSession_CLIENT_VER_MAJOR,
                    GUINT_TO_POINTER(0x1e), 0L);
    mwSession_setProperty(session, mwSession_CLIENT_VER_MINOR,
                    GUINT_TO_POINTER(0x17), 0L);

    mwSession_start(session);
}

void MeanwhileLibrary::logoff()
{
    HERE;
    if (connected) {
        mwSession_stop(session, ERR_SUCCESS);
    }
}

int MeanwhileLibrary::_writeToSocket(struct mwSession *session,
    const char *buffer, gsize count)
{
    HERE;
    MeanwhileLibrary *lib = 
        (MeanwhileLibrary *)mwSession_getClientData(session);
    return lib->writeToSocket(buffer, count);
}

int MeanwhileLibrary::writeToSocket(const char *buffer, unsigned int count)
{
    HERE;
    int remaining, retval = 0;
    for (remaining = count; remaining > 0; remaining -= retval) {
        retval = socket->writeBlock(buffer, count);
        if (retval <= 0)
            return 1;
    }
    socket->flush();
    return 0;
}

void MeanwhileLibrary::_closeSocket(struct mwSession *session) 
{
    HERE;
    MeanwhileLibrary *lib = 
        (MeanwhileLibrary *)mwSession_getClientData(session);
    return lib->closeSocket();
}

void MeanwhileLibrary::closeSocket()
{
    HERE;
    QObject::disconnect(socket, SIGNAL(closed(int)),
                     this, SLOT(slotSocketClosed(int)));
    socket->flush();
    socket->closeNow();
}

void MeanwhileLibrary::newSession()
{
    HERE;
    /* set up the session handler */
    memset(&session_handler, 0, sizeof(session_handler));
    session_handler.io_write       = _writeToSocket;
    session_handler.io_close       = _closeSocket;
    session_handler.clear          = _handler_clear;
    session_handler.on_stateChange = _stateChange;

    /* create the session */
    session = mwSession_new(&session_handler);
    mwSession_setClientData(session, this, 0L);

#if 0
    session->on_setUserStatus = _on_setUserStatus;
#endif

    /* awareness service setup */
    aware_handler.on_attrib = _on_attrib;
    aware_handler.clear     = _attrib_clear;
    srvc_aware = mwServiceAware_new(session, &aware_handler);

    aware_list_handler.on_aware = _on_aware;
    aware_list_handler.on_attrib = _on_aware_attrib;
    aware_list_handler.clear = _aware_clear;

    aware_list = mwAwareList_new(srvc_aware, &aware_list_handler);
    mwAwareList_setClientData(aware_list, this, 0L);

    mwService_setClientData((struct mwService *)srvc_aware, this, 0L);
    mwSession_addService(session, (struct mwService *) srvc_aware);

    /* im service setup */
    im_handler.conversation_opened = _conversation_opened;
    im_handler.conversation_closed = _conversation_closed;
    im_handler.conversation_recv = _conversation_recv;
    im_handler.clear = 0L;
    
    srvc_im = mwServiceIm_new(session, &im_handler);
    mwService_setClientData((struct mwService *)srvc_im, this, 0L);
    mwSession_addService(session, (struct mwService *) srvc_im);

#if 0
    /* FIXME: port to new API */
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
#endif

    /* add a necessary cipher */
    mwSession_addCipher(session, mwCipher_new_RC2_40(session));
}

KExtendedSocket *MeanwhileLibrary::getConnectedSocket()
{
    HERE;
    KExtendedSocket *sock = new KExtendedSocket(account->serverName(),
            account->serverPort(), KExtendedSocket::bufferedSocket);
    int error = sock->connect();
    if (error)
    {
        KMessageBox::queuedMessageBox( 
                        0, KMessageBox::Error , 
                        i18n( "Could not connect to server"),
                        i18n( "Meanwhile Plugin" ), 
                        KMessageBox::Notify );
        delete sock;
        return 0L;
    }
    /* we want to receive signals when there is data to read */
    sock->enableRead(true);
    QObject::connect(sock, SIGNAL(readyRead()) , 
                     SLOT(slotSocketReader()));
    QObject::connect(sock, SIGNAL(closed(int)),
                     SLOT(slotSocketClosed(int)));
    return sock;
}

void MeanwhileLibrary::slotSocketReader()
{
    HERE;
    char buffer[4000];
    int readAmount;
    readAmount = socket->readBlock(buffer,4000);
    if (readAmount < 0)
        return;
    mwSession_recv(session, buffer, (unsigned int) readAmount);
}

void MeanwhileLibrary::slotSocketClosed(int reason)
{
    HERE;
    connected = false;
    if (reason == KExtendedSocket::involuntary) { 
        emit serverNotification(
                QString("Lost connection with Meanwhile server"));
        emit connectionLost();
    }
}

static void free_iter(void *data, void *p)
{
    if (p != 0L)
        return;
    free(data);
}

void MeanwhileLibrary::addContacts(const Q3Dict<Kopete::Contact>& contacts)
{
    HERE;
    Q3DictIterator<Kopete::Contact> it(contacts); 
    GList *buddies = 0L;

    /** Convert our QDict of kopete contact to a GList of meanwhile buddies */
    for( ; it.current(); ++it) {
        struct mwAwareIdBlock *buddy = (struct mwAwareIdBlock *)
            malloc(sizeof(*buddy));
        MeanwhileContact *contact = 
                static_cast<MeanwhileContact *>(it.current());
        if (buddy == 0L)
            continue;
        buddy->user = (gchar*)contact->meanwhileId.ascii();
        buddy->community = 0L;
        buddy->type = mwAware_USER;
        mwDebug() << "Adding contact: '" << buddy->user << "'" << endl;
        buddies = g_list_append(buddies, buddy);
    }

    mwAwareList_addAware(aware_list, buddies);

    g_list_foreach(buddies, free_iter, 0L);
    g_list_free(buddies);
}

void MeanwhileLibrary::addContact(const Kopete::Contact *contact)
{
    HERE;
    char *targetID = strdup(static_cast<const MeanwhileContact*>(contact)
            ->meanwhileId.ascii());
    struct mwAwareIdBlock buddy = 
		{ mwAware_USER, targetID, 0L };
    GList *buddies = 0L;
    g_list_insert(buddies, &buddy, 0);
    
    mwDebug() << "Adding contact: '" << buddy.user << "'" << endl;
    mwAwareList_addAware(aware_list, buddies);
    g_list_free(buddies);
    free(targetID);
}

int MeanwhileLibrary::sendMessage(Kopete::Message &message)
{
    HERE;
    MeanwhileContact *contact =
        static_cast<MeanwhileContact *>(message.to().first());
    if (!contact) {
        mwDebug() << "No target for message!" <<endl;
        return 0;
    }

    char *targetID = strdup(contact->meanwhileId.ascii());
    struct mwIdBlock target = { targetID, 0L };
    struct mwConversation *conv;

    conv = mwServiceIm_getConversation(srvc_im, &target);
    if (conv == 0L) {
        mwDebug() << "No target for conversation with '" << targetID
            << "'" << endl;
        free(targetID);
        return 0;
    }
    free(targetID);

    struct conv_data *conv_data = (struct conv_data *)
        mwConversation_getClientData(conv);
    if (!conv_data && !(conv_data = initConvData(conv, contact)))
        return 0;

    /* if there's other messages in the queue, or the conversation isn't open,
     * then append to the queue instead of sending right away */
    if ((conv_data->queue && !conv_data->queue->isEmpty()) || 
            !mwConversation_isOpen(conv)) {
        conv_data->queue->append(message);
        mwConversation_open(conv);

    } else if (!mwConversation_send(conv, mwImSend_PLAIN,
                message.plainBody().ascii())) {
        conv_data->chat->appendMessage(message);
        conv_data->chat->messageSucceeded();
    }
    return 1;
}

void MeanwhileLibrary::sendTyping(MeanwhileContact *contact, bool isTyping)
{
    HERE;
    char *targetID = strdup(contact->meanwhileId.ascii());
    struct mwIdBlock target = { targetID, 0L };
    struct mwConversation *conv;

    conv = mwServiceIm_getConversation(srvc_im, &target);
    if (conv == 0L) {
        mwDebug() << "No target for typing flag with '" << targetID << "'"
            << endl;
        free(targetID);
        return;
    }
    free(targetID);

    if (mwConversation_isOpen(conv))
        mwConversation_send(conv, mwImSend_TYPING, (void *)isTyping);
}

void MeanwhileLibrary::setState(Kopete::OnlineStatus state,
        const QString msg)
{
    HERE;
    if (state.internalStatus() == MeanwhileLibrary::Offline ||
            state.internalStatus() == 0)
        return;

    struct mwUserStatus stat;
    mwUserStatus_clone(&stat, mwSession_getUserStatus(session));

    free(stat.desc);

    stat.status = (mwStatusType)state.internalStatus();
    if (msg.isNull() || msg.isEmpty())
        stat.desc = (gchar*)strdup(state.description().ascii());
    else
        stat.desc = (gchar*)strdup(msg.ascii());

    mwSession_setUserStatus(session,&stat);
    mwUserStatus_clear(&stat);
}

void MeanwhileLibrary::setStatusMesg(const QString &statusMesg)
{
    HERE;
    if(statusMesg.isNull())
        return;
    struct mwUserStatus stat;
    mwUserStatus_clone(&stat, mwSession_getUserStatus(session));

    free(stat.desc);
    stat.desc = (gchar*)strdup(ADVERTISE_KOPETE(statusMesg).ascii());

    mwSession_setUserStatus(session,&stat);
    mwUserStatus_clear(&stat);
}

#include "meanwhilelibrary.moc"
