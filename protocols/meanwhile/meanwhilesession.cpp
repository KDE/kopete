/*
    meanwhilesession.cpp - interface to the 'C' meanwhile library

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

#include "meanwhilesession.h"

#include <string.h>
#include <stdlib.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qtcpsocket.h>

#include <kopetepassword.h>
#include <kopetechatsession.h>
#include <kopetegroup.h>
#include <kopetecontactlist.h>
#include <kopetesockettimeoutwatcher.h>
#include "meanwhileprotocol.h"

#include <meanwhile/mw_channel.h>
#include <meanwhile/mw_message.h>
#include <meanwhile/mw_error.h>
#include <meanwhile/mw_service.h>
#include <meanwhile/mw_session.h>
#include <meanwhile/mw_srvc_aware.h>
#include <meanwhile/mw_srvc_conf.h>
#include <meanwhile/mw_srvc_im.h>
#include <meanwhile/mw_srvc_store.h>
#include <meanwhile/mw_cipher.h>
#include <meanwhile/mw_st_list.h>

#define set_session_handler(a,b) sessionHandler.a = _handleSession ## b
#define set_aware_handler(a,b)   awareHandler.a = _handleAware ## b
#define set_aware_list_handler(a,b) \
    awareListHandler.a = _handleAwareList ## b
#define set_im_handler(a,b)   imHandler.a = _handleIm ## b

#define get_protocol() (static_cast<MeanwhileProtocol *>(account->protocol()))

static struct MeanwhileClientID ids[] = {
    { mwLogin_LIB,		"Lotus Binary Library" },
    { mwLogin_JAVA_WEB,		"Lotus Java Applet", },
    { mwLogin_BINARY,		"Lotus Binary App", },
    { mwLogin_JAVA_APP,		"Lotus Java App", },
    { mwLogin_LINKS,		"Sametime Links", },

    { mwLogin_NOTES_6_5,	"Notes 6.5", },
    { mwLogin_NOTES_6_5_3,	"Notes 6.5.3", },
    { mwLogin_NOTES_7_0_beta,	"Notes 7.0 beta", },
    { mwLogin_NOTES_7_0,	"Notes 7.0", },
    { mwLogin_ICT,		"ICT", },
    { mwLogin_ICT_1_7_8_2,	"ICT 1.7.8.2", },
    { mwLogin_ICT_SIP,		"ICT SIP", },
    { mwLogin_NOTESBUDDY_4_14,	"NotesBuddy 4.14", },
    { mwLogin_NOTESBUDDY_4_15,	"NotesBuddy 4.15" },
    { mwLogin_NOTESBUDDY_4_16,	"NotesBuddy 4.16" },
    { mwLogin_SANITY,		"Sanity", },
    { mwLogin_ST_PERL,		"ST Perl", },
    { mwLogin_PMR_ALERT,	"PMR Alert", },
    { mwLogin_TRILLIAN,		"Trillian", },
    { mwLogin_TRILLIAN_IBM,	"Trillian (IBM)", },
    { mwLogin_MEANWHILE,	"Meanwhile Library", },
    { 0, NULL },
};


MeanwhileSession::MeanwhileSession(MeanwhileAccount *acc)
    : session(0), state(mwSession_STOPPED), account(acc), socket(0)
{
    HERE;

    /* set up main session hander */
    memset(&sessionHandler, 0, sizeof(sessionHandler));
    set_session_handler(io_write,          IOWrite);
    set_session_handler(io_close,          IOClose);
    set_session_handler(on_stateChange,    StateChange);
    set_session_handler(on_setPrivacyInfo, SetPrivacyInfo);
    set_session_handler(on_setUserStatus,  SetUserStatus);
    set_session_handler(on_admin,          Admin);
    set_session_handler(on_announce,       Announce);
    set_session_handler(clear,             Clear);

    session = mwSession_new(&sessionHandler);
    mwSession_setClientData(session, this, 0L);

    /* set up the aware service */
    memset(&awareHandler, 0, sizeof(awareHandler));
    set_aware_handler(on_attrib, Attrib);

    awareService = mwServiceAware_new(session, &awareHandler);
    mwSession_addService(session, (struct mwService *)awareService);

    /* create an aware list */
    memset(&awareListHandler, 0, sizeof(awareListHandler));
    set_aware_list_handler(on_aware,  Aware);
    set_aware_list_handler(on_attrib, Attrib);
    awareList = mwAwareList_new(awareService, &awareListHandler);
    mwAwareList_setClientData(awareList, this, 0L);

    /* set up an im service */
    memset(&imHandler, 0, sizeof(imHandler));
    set_im_handler(conversation_opened, ConvOpened);
    set_im_handler(conversation_closed, ConvClosed);
    set_im_handler(conversation_recv,   ConvReceived);
    imHandler.place_invite = 0L;
    imHandler.clear = 0L;

    imService = mwServiceIm_new(session, &imHandler);
    mwService_setClientData((struct mwService *)imService, this, 0L);
    mwSession_addService(session, (struct mwService *) imService);

    /* add resolve service */
    resolveService = mwServiceResolve_new(session);
    mwService_setClientData((struct mwService *)resolveService, this, 0L);
    mwSession_addService(session, (struct mwService *) resolveService);

    /* storage service */
    storageService = mwServiceStorage_new(session);
    mwService_setClientData((struct mwService *)storageService, this, 0L);
    mwSession_addService(session, (struct mwService *) storageService);

#if 0
    /* conference service setup - just declines invites for now. */
    memset(&conf_handler, 0, sizeof(conf_handler));
    conf_handler.on_invited = _conference_invite;

    srvc_conf = mwServiceConference_new(session, &conf_handler);
    mwService_setClientData((struct mwService *)srvc_conf, this, 0L);
    mwSession_addService(session, (struct mwService *) srvc_conf);
#endif

    /* add a necessary cipher */
    mwSession_addCipher(session, mwCipher_new_RC2_40(session));
    mwSession_addCipher(session, mwCipher_new_RC2_128(session));
}

MeanwhileSession::~MeanwhileSession()
{
    HERE;
    if (isConnected() || isConnecting())
        disconnect();

    mwSession_removeService(session, mwService_STORAGE);
    mwSession_removeService(session, mwService_RESOLVE);
    mwSession_removeService(session, mwService_IM);
    mwSession_removeService(session, mwService_AWARE);

    mwAwareList_free(awareList);
    mwService_free(MW_SERVICE(storageService));
    mwService_free(MW_SERVICE(resolveService));
    mwService_free(MW_SERVICE(imService));
    mwService_free(MW_SERVICE(awareService));
    mwCipher_free(mwSession_getCipher(session, mwCipher_RC2_40));
    mwCipher_free(mwSession_getCipher(session, mwCipher_RC2_128));

    mwSession_free(session);
}

void MeanwhileSession::getDefaultClientIDParams(int *clientID,
	int *verMajor, int *verMinor)
{
    *clientID = mwLogin_MEANWHILE;
    *verMajor = MW_PROTOCOL_VERSION_MAJOR;
    *verMinor = MW_PROTOCOL_VERSION_MINOR;
}

/* external interface called by meanwhileaccount */
void MeanwhileSession::connect(QString password)
{
    HERE;

    int port, clientID, versionMajor, versionMinor;
    bool useCustomID;
    QString host;

    host = account->getServerName();
    port = account->getServerPort();
    useCustomID = account->getClientIDParams(&clientID,
		    &versionMajor, &versionMinor);


    QTcpSocket *sock = new QTcpSocket(this);
    Kopete::SocketTimeoutWatcher* timeoutWatcher = Kopete::SocketTimeoutWatcher::watch(sock);
    if (timeoutWatcher)
        QObject::connect(timeoutWatcher, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketAboutToClose()));

    sock->connectToHost(host, quint16(port));

    // TODO - make asynchronous
    if (!sock->waitForConnected()) {
        KMessageBox::queuedMessageBox(0, KMessageBox::Error,
                i18n( "Could not connect to server"), i18n("Meanwhile Plugin"),
                KMessageBox::Notify);
        delete sock;
        return;
    }
    socket = sock;
    /* we want to receive signals when there is data to read */
    QObject::connect(sock, SIGNAL(readyRead()), this,
                     SLOT(slotSocketDataAvailable()));
    QObject::connect(sock, SIGNAL(aboutToClose()), this,
                     SLOT(slotSocketAboutToClose()));

    /* set login details */
    mwSession_setProperty(session, mwSession_AUTH_USER_ID,
                    g_strdup(account->meanwhileId().toAscii()), g_free);
    mwSession_setProperty(session, mwSession_AUTH_PASSWORD,
                    g_strdup(password.toAscii()), g_free);

    /* set client type parameters */
    if (useCustomID) {
	mwSession_setProperty(session, mwSession_CLIENT_TYPE_ID,
			GUINT_TO_POINTER(clientID), NULL);
	mwSession_setProperty(session, mwSession_CLIENT_VER_MAJOR,
			GUINT_TO_POINTER(versionMajor), NULL);
	mwSession_setProperty(session, mwSession_CLIENT_VER_MINOR,
			GUINT_TO_POINTER(versionMinor), NULL);
    }


    /* go!! */
    mwSession_start(session);
}

void MeanwhileSession::disconnect()
{
    HERE;
    if (state == mwSession_STOPPED || state == mwSession_STOPPING)
        return;

    mwSession_stop(session, ERR_SUCCESS);
}

bool MeanwhileSession::isConnected()
{
    return mwSession_isStarted(session);
}

bool MeanwhileSession::isConnecting()
{
    return mwSession_isStarting(session);
}

static void free_id_block(void *data, void *p)
{
    if (p != 0 || data == 0)
        return;

    struct mwAwareIdBlock *id = reinterpret_cast<struct mwAwareIdBlock *>(data);
    delete [] id->user;
    free(id);
}

void MeanwhileSession::addContacts(const QHash<QString, Kopete::Contact *> &contacts)
{
    HERE;
    GList *buddies = 0;
    QHash<QString, Kopete::Contact *>::const_iterator it = contacts.constBegin();

    /** Convert our QDict of kopete contact to a GList of meanwhile buddies */
    for( ; it != contacts.constEnd(); ++it) {
        MeanwhileContact *contact = static_cast<MeanwhileContact *>(it.value());
        struct mwAwareIdBlock *id = reinterpret_cast<struct mwAwareIdBlock *>(malloc(sizeof(*id)));
        if (!id)
            continue;
        id->user = qstrdup(contact->meanwhileId().toUtf8().constData());
        id->community = 0;
        id->type = mwAware_USER;
        buddies = g_list_append(buddies, id);
    }

    mwAwareList_addAware(awareList, buddies);

    g_list_foreach(buddies, free_id_block, 0);
    g_list_free(buddies);
}

/* private functions used only by the meanwhile session object */
void MeanwhileSession::addContact(const Kopete::Contact *contact)
{
    HERE;
    struct mwAwareIdBlock id = { mwAware_USER,
        strdup(static_cast<const MeanwhileContact *>(contact)
                ->meanwhileId().toAscii()),
        0L };

    GList *buddies = g_list_prepend(0L, &id);
    mwAwareList_addAware(awareList, buddies);
    g_list_free(buddies);
    free(id.user);
}

int MeanwhileSession::sendMessage(Kopete::Message &message)
{
    HERE;
    MeanwhileContact *contact =
        static_cast<MeanwhileContact *>(message.to().first());
    if (!contact) {
        mwDebug() << "No target for message!" <<endl;
        return 0;
    }

    struct mwIdBlock target = { strdup(contact->meanwhileId().toAscii()), 0L };
    struct mwConversation *conv;

    conv = mwServiceIm_getConversation(imService, &target);
    free(target.user);
    if (conv == 0L) {
        mwDebug() << "No target for conversation with '"
            << contact->meanwhileId() << "'" << endl;
        return 0;
    }

    struct ConversationData *convdata = (struct ConversationData *)
        mwConversation_getClientData(conv);

    if (convdata == 0L) {
        convdata = createConversationData(conv, contact, true);
        if (convdata == 0L) {
            mwDebug() << "No memory for conversation data!" << endl;
            return 0;
        }
    }

    /* if there's other messages in the queue, or the conversation isn't open,
     * then append to the queue instead of sending right away */
    if ((convdata->queue && !convdata->queue->isEmpty()) ||
            !mwConversation_isOpen(conv)) {
        convdata->queue->append(message);
        mwConversation_open(conv);

    } else if (!mwConversation_send(conv, mwImSend_PLAIN,
                message.plainBody().toAscii())) {
        convdata->chat->appendMessage(message);
        convdata->chat->messageSucceeded();
    }
    return 1;
}

void MeanwhileSession::sendTyping(MeanwhileContact *contact, bool isTyping)
{
    HERE;
    struct mwIdBlock target = { strdup(contact->meanwhileId().toAscii()), 0L };
    struct mwConversation *conv;

    conv = mwServiceIm_getConversation(imService, &target);
    free(target.user);
    if (conv == 0L)
        return;

    if (mwConversation_isOpen(conv))
        mwConversation_send(conv, mwImSend_TYPING, (void *)isTyping);
}

void MeanwhileSession::setStatus(Kopete::OnlineStatus status,
        const Kopete::StatusMessage &msg)
{
    HERE;
    mwDebug() << "setStatus: " << status.description() << '('
        << status.internalStatus() << ')' << endl;
    if (status.internalStatus() == 0)
        return;

    struct mwUserStatus stat;
    mwUserStatus_clone(&stat, mwSession_getUserStatus(session));

    free(stat.desc);

    stat.status = (mwStatusType)status.internalStatus();
    if (msg.isEmpty())
        stat.desc = ::strdup(status.description().toUtf8().constData());
    else
        stat.desc = ::strdup(msg.message().toUtf8().constData());

    mwSession_setUserStatus(session, &stat);
    /* will free stat.desc */
    mwUserStatus_clear(&stat);
}

void MeanwhileSession::syncContactsToServer()
{
    HERE;
    struct mwSametimeList *list = mwSametimeList_new();

    /* set up a fallback group for top-level contacts */
    struct mwSametimeGroup *topstgroup = mwSametimeGroup_new(list,
            mwSametimeGroup_DYNAMIC, "People");
    mwSametimeGroup_setOpen(topstgroup, true);

    const QHash<QString, Kopete::Contact *> contacts = account->contacts();
   // Q3DictIterator<Kopete::Contact> it(account->contacts());
    for(QHash<QString, Kopete::Contact *>::const_iterator it = contacts.constBegin();
            it != contacts.constEnd(); ++it ) {
        MeanwhileContact *contact = static_cast<MeanwhileContact *>(it.value());

        /* Find the group that the metacontact is in */
        Kopete::MetaContact *mc = contact->metaContact();
        if (!mc)
            continue;

        Kopete::Group *contactgroup = mc->groups().value(0);
        if (!contactgroup)
            continue;

        if (contactgroup->type() == Kopete::Group::Temporary)
            continue;

        struct mwSametimeGroup *stgroup;
        if (contactgroup->type() == Kopete::Group::TopLevel) {
            stgroup = topstgroup;
        } else  {
            /* find (or create) a matching sametime list group */
            stgroup = mwSametimeList_findGroup(list,
                        contactgroup->displayName().toUtf8().constData());
            if (!stgroup) {
                stgroup = mwSametimeGroup_new(list, mwSametimeGroup_DYNAMIC,
                        contactgroup->displayName().toUtf8().constData());
            }
            mwSametimeGroup_setOpen(stgroup, contactgroup->isExpanded());
            mwSametimeGroup_setAlias(stgroup,
                    contactgroup->pluginData(account->protocol(), "alias").toUtf8().constData());
        }

        QByteArray tmpMeanwhileId = contact->meanwhileId().toUtf8();
        /* now add the user (by IDBlock) */
        struct mwIdBlock id =
            { (gchar*)tmpMeanwhileId.constData(), 0 };
        struct mwSametimeUser *stuser = mwSametimeUser_new(stgroup,
                mwSametimeUser_NORMAL, &id);

        mwSametimeUser_setAlias(stuser, mc->displayName().toUtf8().constData());
    }

    /* store! */
    struct mwPutBuffer *buf = mwPutBuffer_new();
    struct mwStorageUnit *unit = mwStorageUnit_new(mwStore_AWARE_LIST);
    struct mwOpaque *opaque = mwStorageUnit_asOpaque(unit);

    mwSametimeList_put(buf, list);
    mwPutBuffer_finalize(opaque, buf);

    mwServiceStorage_save(storageService, unit, NULL, NULL, NULL);

    mwSametimeList_free(list);
}

void MeanwhileSession::syncContactsFromServer()
{
    struct mwStorageUnit *unit = mwStorageUnit_new(mwStore_AWARE_LIST);
    mwServiceStorage_load(storageService, unit, &_handleStorageLoad, 0L, 0L);
}

#define MEANWHILE_SESSION_BUFSIZ 4096

void MeanwhileSession::slotSocketDataAvailable()
{
    HERE;
    guchar *buf;
    qint64 bytesRead;

    if (!socket)
        return;

    if (!(buf = (guchar *)malloc(MEANWHILE_SESSION_BUFSIZ))) {
        mwDebug() << "buffer malloc failed" << endl;
        return;
    }

    while (socket && socket->bytesAvailable() > 0) {
        bytesRead = socket->read((char *)buf, MEANWHILE_SESSION_BUFSIZ);
        if (bytesRead < 0)
            break;
        mwSession_recv(session, buf, (unsigned int)bytesRead);
    }
    free(buf);
}

void MeanwhileSession::slotSocketAboutToClose()
{
    HERE;

    /* TODO -error handling
    if (reason & KExtendedSocket::involuntary)
        emit serverNotification(
                QString("Lost connection with Meanwhile server"));
    */

    mwSession_stop(session, 0x00);
}


Kopete::OnlineStatus MeanwhileSession::convertStatus(int mstatus)
{
    MeanwhileProtocol *protocol =
        static_cast<MeanwhileProtocol *>(account->protocol());

    switch (mstatus) {
    case mwStatus_ACTIVE:
        return protocol->statusOnline;
        break;
    case mwStatus_IDLE:
        return protocol->statusIdle;
        break;
    case mwStatus_AWAY:
        return protocol->statusAway;
        break;
    case mwStatus_BUSY:
        return protocol->statusBusy;
        break;
    case 0:
        return protocol->statusOffline;
        break;
    default:
        mwDebug() << "unknown status lookup: " << mstatus << endl;
    }
    return protocol->statusOffline;
}

void MeanwhileSession::resolveContactNickname(MeanwhileContact *contact)
{
    /* @todo: FIXME: leak! */
    char *id = strdup(contact->meanwhileId().toAscii());
    GList *query = g_list_prepend(NULL, id);
    mwServiceResolve_resolve(resolveService, query, mwResolveFlag_USERS,
            _handleResolveLookupResults, contact, NULL);
}

QString MeanwhileSession::getNickName(struct mwLoginInfo *logininfo)
{
    if (logininfo == 0L || logininfo->user_name == 0L)
        return QString();
    return getNickName(logininfo->user_name);
}

QString MeanwhileSession::getNickName(QString name)
{

    int index = name.indexOf(QLatin1String(" - "));
    if (index != -1)
        name.remove(0, index + 3);
    index = name.indexOf(QLatin1Char('/'));
    if (index != -1)
        name = name.left(index);

    return name;
}

MeanwhileContact *MeanwhileSession::conversationContact(
        struct mwConversation *conv)
{
    struct mwIdBlock *target = mwConversation_getTarget(conv);
    if (target == 0L || target->user == 0L) {
        return 0L;
    }
    QString user(target->user);

    MeanwhileContact *contact = static_cast<MeanwhileContact *>(account->contacts().value(user));

    struct mwLoginInfo *logininfo = mwConversation_getTargetInfo(conv);
    QString name = getNickName(logininfo);

    if (!contact) {
	account->addContact(user, name, 0L, Kopete::Account::Temporary);
	    contact = static_cast<MeanwhileContact *>(account->contacts().value(user));
    } else
        contact->setNickName(name);

    return contact;
}

void MeanwhileSession::handleRedirect(const char *host)
{
    /* if configured manually, force the login */
    if (account->getForceLogin()) {
        mwSession_forceLogin(session);
        return;
    }

    /* if we're connecting to the same host, force */
    if (!host || account->getServerName() == host) {
        mwSession_forceLogin(session);
        return;
    }

    QTcpSocket *sock = new QTcpSocket(this);

    Kopete::SocketTimeoutWatcher* timeoutWatcher = Kopete::SocketTimeoutWatcher::watch(sock);
    if (timeoutWatcher)
        QObject::connect(timeoutWatcher, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketAboutToClose()));

    sock->connectToHost(host, quint16(account->getServerPort()));

    if (!sock->waitForConnected()) {
        KMessageBox::queuedMessageBox(0, KMessageBox::Error,
                i18n( "Could not connect to redirected server"),
                        i18n("Meanwhile Plugin"),
                KMessageBox::Notify);
        delete sock;
        mwSession_forceLogin(session);
        return;
    }

    /* we've redirected, so swap the sockets */
    delete this->socket;
    this->socket = sock;

    /* we want to receive signals when there is data to read */
    QObject::connect(sock, SIGNAL(readyRead()), this,
                     SLOT(slotSocketDataAvailable()));
    QObject::connect(sock, SIGNAL(aboutToClose()), this,
                     SLOT(slotSocketAboutToClose()));
}

/* priave session handling functions, called by libmeanwhile callbacks */
void MeanwhileSession::handleSessionStateChange(
        enum mwSessionState state, gpointer data)
{
    HERE;
    this->state = state;

    switch (state) {
        case mwSession_STARTING:
        case mwSession_HANDSHAKE:
        case mwSession_HANDSHAKE_ACK:
        case mwSession_LOGIN:
        case mwSession_LOGIN_CONT:
        case mwSession_LOGIN_ACK:
            break;

        case mwSession_LOGIN_REDIR:
            handleRedirect((char *)data);
            break;

        case mwSession_STARTED:
            {
                struct mwUserStatus stat = { mwStatus_ACTIVE, 0, 0L };
                mwSession_setUserStatus(session, &stat);
                struct mwLoginInfo *logininfo = mwSession_getLoginInfo(session);
                if (logininfo) {
                    account->myself()->setNickName(getNickName(logininfo));
                }
                syncContactsFromServer();
            }
            break;

        case mwSession_STOPPING:
            {
                unsigned int info = GPOINTER_TO_UINT(data);
                if (info & ERR_FAILURE) {
                    if (info == INCORRECT_LOGIN)
                        account->password().setWrong();
                    char *reason = mwError(info);
                    emit serverNotification(QString(reason));
                    free(reason);
                }
            }

            emit sessionStateChange(
                    static_cast<MeanwhileProtocol *>(account->protocol())
		    ->statusOffline);
            break;

        case mwSession_STOPPED:
            break;

        case mwSession_UNKNOWN:
        default:
            mwDebug() << "Unhandled state change " << state << endl;
    }
}

int MeanwhileSession::handleSessionIOWrite(const guchar *buffer,
        gsize count)
{
    HERE;

    if (socket == 0L)
        return 1;

    int remaining, retval = 0;
    for (remaining = count; remaining > 0; remaining -= retval) {
        retval = socket->write((char *)buffer, count);
        if (retval <= 0)
            return 1;
    }
    socket->flush();
    return 0;
}

void MeanwhileSession::handleSessionAdmin(const char *text)
{
    HERE;
    emit serverNotification(QString(text));
}

void MeanwhileSession::handleSessionAnnounce(struct mwLoginInfo *from,
        gboolean /* may_reply */, const char *text)
{
    HERE;
    QString message;
    message.sprintf("Announcement from %s:\n%s", from->user_id, text);
    emit serverNotification(message);
}

void MeanwhileSession::handleSessionSetUserStatus()
{
    struct mwUserStatus *userstatus = mwSession_getUserStatus(session);
    emit sessionStateChange(convertStatus((unsigned int)userstatus->status));
}

void MeanwhileSession::handleSessionSetPrivacyInfo()
{
}

void MeanwhileSession::handleSessionIOClose()
{
    HERE;

    if (!socket)
        return;

    socket->flush();
    socket->close();

    delete socket;
    socket = 0;
}

void MeanwhileSession::handleSessionClear()
{
}

void MeanwhileSession::handleAwareAttrib(struct mwAwareAttribute * /* attrib */)
{
    HERE;
}

void MeanwhileSession::handleAwareListAware(struct mwAwareSnapshot *snapshot)
{
    HERE;
    MeanwhileContact *contact = static_cast<MeanwhileContact *>
		(account->contacts().value(snapshot->id.user));

    if (contact == 0L)
        return;

    /* use the setUserStatus callback for status updates for myself. */
    if (contact == account->myself())
        return;

    /* ### TODO!!
    contact->setProperty(get_protocol()->statusMessage, snapshot->status.desc);
    contact->setProperty(get_protocol()->awayMessage, snapshot->status.desc);
    */

    Kopete::OnlineStatus onlinestatus;
    if (snapshot->online) {
        onlinestatus = convertStatus(snapshot->status.status);
        resolveContactNickname(contact);
    } else {
        onlinestatus = convertStatus(0);
    }

    contact->setOnlineStatus(onlinestatus);

#if 0
    /* Commented out in previous kopete/meanwhile plugin for some reason,
     * but has still been ported to the new API.
     */
    time_t idletime = 0;
    if (snapshot->status.status == mwStatus_IDLE) {
	idletime = (snapshot->status.time == 0xdeadbeef) ?
            0 : snapshot->status.time;
        if (idletime != 0) {
        contact->setStatusDescription(statusDesc + '[' +
                QString::number(idletime/60)+" mins]");
        }
    } else
        contact->setStatusDescription(snapshot->status.desc);
#endif
}

void MeanwhileSession::handleAwareListAttrib(struct mwAwareIdBlock * /* id */,
        struct mwAwareAttribute * /* attrib */)
{
    HERE;
}

struct MeanwhileSession::ConversationData
    *MeanwhileSession::createConversationData(
        struct mwConversation *conv, MeanwhileContact *contact,
        bool createQueue)
{
    struct ConversationData *cd = new ConversationData();

    if (cd == 0L)
        return 0L;

    cd->contact = contact;
    cd->chat    = contact->manager(Kopete::Contact::CanCreate);
    cd->chat->ref();
    if (createQueue)
        cd->queue = new QList<Kopete::Message>();

    mwConversation_setClientData(conv, cd, 0L);

    return cd;
}

void MeanwhileSession::handleImConvOpened(struct mwConversation *conv)
{
    HERE;

    struct ConversationData *convdata =
        (struct ConversationData *)mwConversation_getClientData(conv);

    if (convdata == 0L) {
        /* a new conversation */
        convdata = createConversationData(conv, conversationContact(conv));

        if (convdata == 0L) {
            mwDebug() << "No memory for conversation data!" << endl;
            return;
        }

    } else if (convdata->queue && !convdata->queue->isEmpty()) {
        /* send any messages that were waiting for the conversation to open */
        QList<Kopete::Message>::iterator it;
        for (it = convdata->queue->begin(); it != convdata->queue->end();
                ++it) {
            mwConversation_send(conv, mwImSend_PLAIN,
                    (*it).plainBody().toAscii());
            convdata->chat->appendMessage(*it);
            convdata->chat->messageSucceeded();
        }
        convdata->queue->clear();
        delete convdata->queue;
        convdata->queue = 0L;
    }
    resolveContactNickname(convdata->contact);
}

void MeanwhileSession::handleImConvClosed(struct mwConversation *conv,
        guint32)
{
    HERE;

    ConversationData *convdata =
        (ConversationData *)mwConversation_getClientData(conv);

    if (!convdata)
        return;

    mwConversation_setClientData(conv, 0L, 0L);

    convdata->chat->removeContact(convdata->contact);
    convdata->chat->deref();
    convdata->chat = 0L;
    if (convdata->queue != 0L) {
        convdata->queue->clear();
        delete convdata->queue;
        convdata->queue = 0L;
    }
    free(convdata);
}

void MeanwhileSession::handleImConvReceived(struct mwConversation *conv,
        enum mwImSendType type, gconstpointer msg)
{
    HERE;
    ConversationData *convdata =
        (ConversationData *)mwConversation_getClientData(conv);

    if (!convdata)
        return;

    switch (type) {
    case mwImSend_PLAIN:
        {
            Kopete::Message message(convdata->contact, account->myself());
            message.setPlainBody(QString::fromUtf8((const char *)msg));
            message.setDirection(Kopete::Message::Inbound);
            convdata->chat->appendMessage(message);
        }
        break;
    case mwImSend_TYPING:
        convdata->chat->receivedTypingMsg(convdata->contact);
        break;
    default:
        mwDebug() << "Unable to handle message type: " << type << endl;
    }
}

void MeanwhileSession::handleResolveLookupResults(
        struct mwServiceResolve * /* srvc */, guint32 /* id */,
        guint32 /* code */, GList *results, gpointer data)
{
    struct mwResolveResult *result;
    struct mwResolveMatch *match;

    if (results == 0L)
        return;
    if ((result = (struct mwResolveResult *)results->data) == 0L)
        return;

    if (result->matches == 0L)
        return;
    if ((match = (struct mwResolveMatch *)result->matches->data) == 0L)
        return;

    mwDebug() << "resolve lookup returned '" << match->name << "'" << endl;

    MeanwhileContact *contact = (MeanwhileContact *)data;
    if (contact == 0L)
        return;

    contact->setNickName(getNickName(match->name));
}

void MeanwhileSession::handleStorageLoad(struct mwServiceStorage * /* srvc */,
        guint32 result, struct mwStorageUnit *item, gpointer /* data */)
{
    HERE;
    if (result != ERR_SUCCESS) {
        mwDebug() << "contact list load returned " << result << endl;
        return;
    }

    struct mwGetBuffer *buf = mwGetBuffer_wrap(mwStorageUnit_asOpaque(item));
    struct mwSametimeList *list = mwSametimeList_new();
    mwSametimeList_get(buf, list);

    GList *gl, *glf, *cl, *clf;

    Kopete::ContactList *contactlist = Kopete::ContactList::self();

    for (glf = gl = mwSametimeList_getGroups(list); gl; gl = gl->next) {
        struct mwSametimeGroup *stgroup = (struct mwSametimeGroup *)gl->data;

        Kopete::Group *group =
            contactlist->findGroup(mwSametimeGroup_getName(stgroup));
        group->setPluginData(account->protocol(), "alias",
                mwSametimeGroup_getAlias(stgroup));

        for (clf = cl = mwSametimeGroup_getUsers(stgroup); cl; cl = cl->next) {
            struct mwSametimeUser *stuser = (struct mwSametimeUser *)cl->data;

            MeanwhileContact *contact = static_cast<MeanwhileContact *>
		        (account->contacts().value(mwSametimeUser_getUser(stuser)));

            if (contact != 0L)
                continue;

            account->addContact(mwSametimeUser_getUser(stuser),
                    mwSametimeUser_getAlias(stuser), group,
                    Kopete::Account::ChangeKABC);
        }
        g_list_free(clf);
    }
    g_list_free(glf);

    mwSametimeList_free(list);
}

const struct MeanwhileClientID *MeanwhileSession::getClientIDs()
{
    return ids;
}

#if 0
MEANWHILE_HOOK_CONFERENCE(conference_invite,
        (struct mwConference *conf, struct mwLoginInfo *inviter,
         const char *invite),
        (conf, inviter, invite))
{
    HERE;
    QString message;

    message.sprintf("%s has invited you to a conference called \"%s\"\n"
        "However, this version of the meanwhile plugin does "
        "not support conferences, so the invitiation has been declined.",
        inviter->user_id, invite);

    mwConference_reject(conf, ERR_SUCCESS,
            "Sorry, my client doesn't support conferences!");
    KMessageBox::queuedMessageBox(0, KMessageBox::Sorry , message,
            i18n("Meanwhile Plugin: Conference invitation"),
            KMessageBox::Notify);
}
#endif
#include "meanwhilesession.moc"
