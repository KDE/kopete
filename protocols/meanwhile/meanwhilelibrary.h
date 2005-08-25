/*
    meanwhilelibrary.h - interface to the 'C' meanwhile library

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
#ifndef MEANWHILELIBRARY_H
#define MEANWHILELIBRARY_H

#include <q3ptrlist.h>
#include <q3dict.h>
#include <qobject.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <glib/ghash.h>
#include <kextsock.h>

#include <kopetecontact.h>
#include <kopetechatsession.h>
#include <kopetemessage.h>

#include "meanwhileaccount.h"
#include "meanwhilecontact.h"

extern "C" {
#include <meanwhile/mw_session.h>
#include <meanwhile/mw_srvc_conf.h>
#include <meanwhile/mw_srvc_im.h>
#include <meanwhile/mw_srvc_aware.h>
}

/**
 * A class to handle the interface to the libmeanwhile code, currently using
 * version 0.4.2
 */
class MeanwhileLibrary : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a library. By default, the library is not connected - you will
     * need to call login() to initiate the connection process.
     * @param account The account that the connection is for
     */
    MeanwhileLibrary(MeanwhileAccount *account);
    
    /**
     * Destroy the library
     */
    ~MeanwhileLibrary();

    /**
     * Determine whether the library is connected and logged in
     * @return true if there is a logged-in session with the meanwhile server
     */
    bool isConnected();

    /**
     * Log in to the server. This will open a socket and start login. Note that
     * the connection process is ascychronous - a loginDone() signal will be
     * emitted when sucessfully logged in.
     */
    void login();

    /**
     * Log off
     */
    void logoff();

    /**
     * Register contacts for presence updates
     * @param contacts A set of contacts to add
     */
    void addContacts(const Q3Dict<Kopete::Contact> &contacts);

    /**
     * Register a single contact for presence awareness
     * @param contact a contact to add
     */
    void addContact(const Kopete::Contact *contact);

    /**
     * Send a typing notification to a single contact
     * @param contact The contact that is being typed to
     * @param isTyping true if a typing message is to be sent
     */
    void sendTyping(MeanwhileContact *contact, bool isTyping);

    /**
     * Send a message to a contact. If a conversation is not yet open to the
     * message's recipient, the message will be queued until a conversation has
     * become open (we also tell the conversation to open, so it should happen
     * in the near future). The library will call appendMessage() and
     * messageSuccceeded() when the message has been sent.
     * @param message The message to send
     * @return non-zero if the message was sent OK
     */
    int sendMessage(Kopete::Message &message);

    /**
     * Set our (the local contact's) online status. The internalStatus of the
     * state argument will be used to define the state message we send - it
     * should be one of the Status enum fields (and not Offline)
     * @param state the new state of the local user
     * @param msg a custom message to use, if required
     */
    void setState(Kopete::OnlineStatus state,
            const QString msg = QString::null);

    /**
     * Set the status description of the local user
     * @param statusMesg a description of the local sataus
     */
    void setStatusMesg(const QString &statusMesg);

    /**
     * Allowed states for the meanwhile protocol.
     */
    enum Status {
        Active  = mwStatus_ACTIVE, /**< active/online */
        Away    = mwStatus_AWAY,   /**< away */
        Idle    = mwStatus_IDLE,   /**< idle */
        Busy    = mwStatus_BUSY,   /**< busy */
        Offline = 0xffff           /**< offline. */
    };

signals:
    /**
     * Emitted when the login process is complete
     */
    void loginDone();

    /**
     * Emitted when the connection to the server has been lost
     */
    void connectionLost();

    /**
     * Emitted when a notification is received from the server, or other
     * out-of-band data (eg, the password is incorrect).
     * @param mesgString A description of the notification
     */
    void serverNotification(const QString &mesgString);


private:
    /** Session handler for general calls */
    struct mwSessionHandler   session_handler;
    /** Aware list changes */
    struct mwAwareListHandler aware_list_handler;
    /** Aware attribute changes */
    struct mwAwareHandler     aware_handler;
    /** Instant message handler */
    struct mwImHandler        im_handler;


    /* session handler callbacks */
    static void _stateChange(struct mwSession *s,
                    enum mwSessionState state, guint32 info);

    static void _handler_clear(struct mwSession *s);

    /* individual state change functions */
    void session_loginAck(mwSession *s);
    void session_stop(mwSession *s, unsigned int status);

    static int _writeToSocket(struct mwSession *session,
                    const char *buffer, gsize count);
    int writeToSocket(const char *buffer, unsigned int count);

    static void _closeSocket(struct mwSession *session);
    void closeSocket();

#define DEFINE_MW_HOOK(func) \
static void _ ## func;   \
void func

    DEFINE_MW_HOOK(setUserStatus(mwSession *s));

    /* aware attribute handlers */
    static void _on_attrib(struct mwServiceAware *srvc,
            struct mwAwareAttribute *attrib);
    static void _attrib_clear(struct mwServiceAware *srvc);

    /* aware list handlers */
    DEFINE_MW_HOOK(on_aware(struct mwAwareList *list,
            struct mwAwareSnapshot *id));
    static void _on_aware_attrib(struct mwAwareList *list,
            struct mwAwareIdBlock *id,
            struct mwAwareAttribute *attrib);
    static void _aware_clear(struct mwAwareList *list);

    /* conversation handlers */
    DEFINE_MW_HOOK(conversation_opened(struct mwConversation *conv));
    DEFINE_MW_HOOK(conversation_closed(struct mwConversation *conv,
        guint32 err));
    DEFINE_MW_HOOK(conversation_recv(struct mwConversation *conv,
        enum mwImSendType type, gconstpointer msg));

    /**
     * A structure to put into the private data of a conversation (and a
     * conference, later). Since the callbacks that we give to the
     * meanwhile library need to be static, we put a pointer to one of
     * these in the conversation's private data.
     */
    struct conv_data {
        MeanwhileLibrary  *library; /**< The library for this conv. */
        Kopete::ChatSession *chat;  /**< The chatsession for this conv. */
        Q3ValueList<Kopete::Message> *queue; /**< Unsent message queue */
    };

    /**
     * Convenience method to set a conversation's private data, with a new
     * conv_data struct. The conv_data should be free()ed when no longer
     * required.
     * @param conv The new conversation
     * @param contact The remote contact for this conversation
     * @return The new conv_data structure for this conversation
     */
    struct conv_data *initConvData(struct mwConversation *conv,
        MeanwhileContact *contact);

    /**
     * Find the MeanwhileContact for a conversation
     * @param conv The conversation
     * @return the MeanwhileContact on the other side of this conversation
     */
    MeanwhileContact *convContact(struct mwConversation *conv);

    /**
     * Parse a friendly name from a mwLoginInfo struct
     * @param logininfo The login info struct for the user
     * @return the user name (ie full name), or QString::null if none could
     *   be parsed.
     */
    static QString getNickName(struct mwLoginInfo *logininfo);

    /**
     * Initialise the session handlers for a library.
     */
    void newSession();

    /**
     * Create a connected socket to the server (defined by the account object)
     * @return a connected socket
     */
    KExtendedSocket *getConnectedSocket();


    /** The kopete account that this library is for */
    MeanwhileAccount *account;

    /** The meanwhile session that we're connected to */
    struct mwSession *session;

    /** The aware service */
    struct mwServiceAware *srvc_aware;

    /** Aware list */
    struct mwAwareList *aware_list; 

    /** IM service */
    struct mwServiceIm *srvc_im;

    /** socket to the server */
    KExtendedSocket *socket;

    /** connected (& logged in) flag */
    bool connected;

private slots:

    /** Notify the library that data is available on the socket */
    void slotSocketReader();

    /**
     * Notify the library that the socket has been closed
     * @param reason the reason for closing
     */
    void slotSocketClosed(int reason);
};

#endif

