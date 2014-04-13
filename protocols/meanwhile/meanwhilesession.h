/*
    meanwhilesession.h - interface to the 'C' meanwhile session

    Copyright (c) 2005      by Jeremy Kerr <jk@ozlabs.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef MEANWHILESESSION_H
#define MEANWHILESESSION_H

#include "meanwhileaccount.h"
#include "meanwhilecontact.h"

#include <meanwhile/mw_session.h>
#include <meanwhile/mw_service.h>
#include <meanwhile/mw_srvc_aware.h>
#include <meanwhile/mw_srvc_im.h>
#include <meanwhile/mw_srvc_resolve.h>

#include <QList>

class QTcpSocket;

struct MeanwhileClientID {
    int		id;
    const char *name;
};


/**
 * A class to handle libmeanwhile session management.
 */
class MeanwhileSession : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a session. By default, the session is not connected - you will
     * need to call login() to initiate the connection process.
     * @param account The account that the connection is for
     */
    MeanwhileSession(MeanwhileAccount *account);

    /**
     * Destroy the session
     */
    ~MeanwhileSession();

    /**
     * Connect to the server. This will open a socket and start login. Note that
     * the connection process is ascychronous - a loginDone() signal will be
     * emitted when successfully logged in.
     */
    void connect(QString password);

    /**
     * Disconnect from the server.
     */
    void disconnect();

    /**
     * Set our (the local contact's) online status. The internalStatus of the
     * state argument will be used to define the state message we send - it
     * should be one of the Status enum fields (and not Offline)
     * @param state the new state of the local user
     * @param msg a custom message to use, if required
     */
    void setStatus(Kopete::OnlineStatus status,
            const Kopete::StatusMessage &msg = Kopete::StatusMessage());

    /**
     * Add a single contact to be registered for status updates
     * @param contact The contact to register
     */
    void addContact(const Kopete::Contact *contact);

    /**
     * Add a list of contacts to be registered for status updates
     * @param contact The list of contacts to register
     */
    void addContacts(const QHash<QString, Kopete::Contact *> &contacts);

    /**
     * Send a message (with recipient specified).
     * @param message The message to send
     * @return non-zero if  the message could be sent
     */
    int sendMessage(Kopete::Message &message);

    /**
     * Send a typing notification to a contact
     * @param contact  The contact to notify
     * @param isTyping If true, the typing notification is set
     */
    void sendTyping(MeanwhileContact *contact, bool isTyping);

    /**
     * Determine if the session is connected to the server
     * @return true if the session is connected
     */
    bool isConnected();

    /**
     * Determine if the session is in the process of connecting to the server
     * @return true if the session is connecting
     */
    bool isConnecting();

    /**
     * Returns an array of well-known meanwhile client IDs
     */
    static const struct MeanwhileClientID *getClientIDs();

    /**
     * Get the default client ID parameters for kopete
     */
    static void getDefaultClientIDParams(int *clientID,
	    int *verMajor, int *verMinor);

signals:
    /**
     * Emitted when the status of the connection changes
     * @param status The new status of the session
     */
    void sessionStateChange(Kopete::OnlineStatus status);

    /**
     * Emitted when a notification is received from the server, or other
     * out-of-band data (eg, the password is incorrect).
     * @param mesgString A description of the notification
     */
    void serverNotification(const QString &mesgString);

private:
    /** Main libmeanwhile session object */
    struct mwSession *session;

    /** Session handler */
    struct mwSessionHandler sessionHandler;

    /** Aware service */
    struct mwServiceAware *awareService;

    /** Aware handler */
    struct mwAwareHandler awareHandler;

    /** Aware List Handler */
    struct mwAwareListHandler awareListHandler;

    /** The aware list */
    struct mwAwareList *awareList;

    /** Aware service */
    struct mwServiceIm *imService;

    /** Aware handler */
    struct mwImHandler imHandler;

    /** Resolve service */
    struct mwServiceResolve *resolveService;

    /** Storage service, for contact list */
    struct mwServiceStorage *storageService;

    /** Last recorded meanwhile state */
    enum mwSessionState state;

    /** The kopete account that this library is for */
    MeanwhileAccount *account;

    /** socket to the server */
    QTcpSocket *socket;

    /* These structures are stored in the libmeanwhile 'ClientData' fields */

    /** Stored in the mwConversation struct */
    struct ConversationData {
        MeanwhileContact *contact;
        Kopete::ChatSession *chat;
        QList<Kopete::Message> *queue;
    };

    /** (To be) stored in the mwConference struct */
    struct ConferenceData {
        Kopete::ChatSession *chatsession;
    };

    /**
     * Initialise the conversation data struct for a conversation, and store it 
     * in the meanwhile conversation object
     * @param conv        the meanwhile conversation object
     * @param contact     the contact that the conversation is with
     * @param createQueue whether a message queue is required for this
     *                    conversation
     * @return The created conversation data struct
     */
    struct ConversationData *createConversationData(
            struct mwConversation *conv, MeanwhileContact *contact,
            bool createQueue = false);

    /**
     * Get the contact for a conversation
     * @param conv the meanwhile conversation
     * @return the contact that this conversation is held with
     */
    MeanwhileContact *conversationContact(struct mwConversation *conv);

    /**
     * Convert a libmeanwhile-type status into one of the MeanwhileProtocol
     * statuses
     * @param mstatus The internal status to convert
     * @return The Meanwhile status
     */
    Kopete::OnlineStatus convertStatus(int mstatus);

    /**
     * Parse the nickname of a libmeanwhile contact. From what I've seen,
     * usernames are in the format:
     *  <userid> - <name>/<domain>/<domain>
     * @param name the extened username to parse
     * @return just the name part of the username info
     */
    QString getNickName(QString name);

    /**
     * Convenience method to call the above from a mwLoginInfo struct. All is
     * checked for null.
     * @param logininfo the login info for a contact
     * @return just the name part of the login info data
     */
    QString getNickName(struct mwLoginInfo *logininfo);

    /**
     * Resolve a contact to find (and set) the display name. This requires the
     * session to be connected to use the meanwhile resolve service.
     * @param contact The contact to resolve
     */
     void resolveContactNickname(MeanwhileContact *contact);

public:
    void syncContactsToServer();
    void syncContactsFromServer();

private slots:

    /** Notify the library that data is available on the socket */
    void slotSocketDataAvailable();

    /**
     * Notify the library that the socket is about to be closed
     */
    void slotSocketAboutToClose();

private:
    /* ugly callbacks for libmeanwhile interface. These declare a static method
     * to proxy the callback from libmeanwhile to a call to the MeanwhileSession
     */

#define declare_session_handler_type(type, func, args, ...)                    \
    static type _handleSession ## func (                                       \
            struct mwSession *mwsession, __VA_ARGS__) {                        \
        MeanwhileSession *session =                                            \
            (MeanwhileSession *)mwSession_getClientData(mwsession);            \
        return session->handleSession ## func args;                            \
    };                                                                         \
    type handleSession ## func(__VA_ARGS__)
#define declare_session_handler(func, args, ...)                               \
    static void _handleSession ## func (                                       \
            struct mwSession *mwsession, ## __VA_ARGS__) {                     \
        MeanwhileSession *session =                                            \
            (MeanwhileSession *)mwSession_getClientData(mwsession);            \
        session->handleSession ## func args;                                   \
    };                                                                         \
    void handleSession ## func(__VA_ARGS__)

    declare_session_handler_type(int, IOWrite, (buf, len),
            const guchar *buf, gsize len);
    declare_session_handler(IOClose,());
    declare_session_handler(Clear,());
    declare_session_handler(StateChange, (state, info),
            enum mwSessionState state, gpointer info);
    declare_session_handler(SetPrivacyInfo,());
    declare_session_handler(SetUserStatus,());
    declare_session_handler(Admin, (text), const char *text);
    declare_session_handler(Announce, (from, may_reply, text),
            struct mwLoginInfo *from, gboolean may_reply, const char *text);

#define declare_aware_handler(func, args, ...)                                 \
    static void _handleAware ## func (                                         \
            struct mwServiceAware *srvc, ## __VA_ARGS__) {                     \
        MeanwhileSession *session = (MeanwhileSession *)                       \
            mwService_getClientData((struct mwService *)srvc);                 \
        return session->handleAware ## func args;                              \
    }; \
    void handleAware ## func(__VA_ARGS__)

    declare_aware_handler(Attrib, (attrib), struct mwAwareAttribute *attrib);
    declare_aware_handler(Clear,());

#define declare_aware_list_handler(func, args, ...)                            \
    static void _handleAwareList ## func (                                     \
            struct mwAwareList *list, ## __VA_ARGS__){                         \
        MeanwhileSession *session = (MeanwhileSession *)                       \
            mwAwareList_getClientData(list);                                   \
        return session->handleAwareList ## func args;                          \
    }; \
    void handleAwareList ## func(__VA_ARGS__)

    declare_aware_list_handler(Aware, (snapshot),
            struct mwAwareSnapshot *snapshot);
    declare_aware_list_handler(Attrib, (id, attrib),
            struct mwAwareIdBlock *id, struct mwAwareAttribute *attrib);
    declare_aware_list_handler(Clear,());

#define declare_im_handler(func, args, ...)                                    \
    static void _handleIm ## func (                                            \
            struct mwConversation *conv, ## __VA_ARGS__) {                     \
        MeanwhileSession *session = (MeanwhileSession *)                       \
            mwService_getClientData(                                           \
                (struct mwService *)mwConversation_getService(conv));          \
        return session->handleIm ## func args;                                 \
    }; \
    void handleIm ## func (struct mwConversation *conv, ## __VA_ARGS__)

    declare_im_handler(ConvOpened, (conv));
    declare_im_handler(ConvClosed, (conv, err), guint32 err);
    declare_im_handler(ConvReceived, (conv, type, msg),
            enum mwImSendType type, gconstpointer msg);

    /* resolve service */
    static void _handleResolveLookupResults(struct mwServiceResolve *srvc,
            guint32 id, guint32 code, GList *results, gpointer data) {
        MeanwhileSession *session = (MeanwhileSession *)
            mwService_getClientData(MW_SERVICE(srvc));
        session->handleResolveLookupResults(srvc, id, code, results, data);
    };
    void handleResolveLookupResults(struct mwServiceResolve *srvc, guint32 id,
            guint32 code, GList *results, gpointer data);

    /* storage service */
    static void _handleStorageLoad(struct mwServiceStorage *srvc,
            guint32 result, struct mwStorageUnit *item, gpointer data) {
        MeanwhileSession *session = (MeanwhileSession *)
            mwService_getClientData(MW_SERVICE(srvc));
        session->handleStorageLoad(srvc, result, item, data);
    };
    void handleStorageLoad(struct mwServiceStorage *srvc,
            guint32 result, struct mwStorageUnit *item, gpointer data);

    void handleRedirect(const char *host);
};

#endif

