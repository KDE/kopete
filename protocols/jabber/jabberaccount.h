
/***************************************************************************
                jabberaccount.h  -  core Jabber account class
                             -------------------
    begin                : Sat Mar 8 2003
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
				Based on JabberProtocol by Daniel Stone <dstone@kde.org>
				and Till Gerken <till@tantalo.net>.
    copyright            : (C) 2006 by Olivier Goffart <ogoffart at kde.org>

    Copyright 2006 by Tommi Rantala <tommi.rantala@cs.helsinki.fi>

    Kopete (C) 2001-2006 Kopete developers  <kopete-devel@kde.org>.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERACCOUNT_H
#define JABBERACCOUNT_H

// we need these for type reasons
#include <kopetepasswordedaccount.h>
#include <kopeteonlinestatus.h>
#include <im.h>
#include "jabberclient.h"
#include "mood.h"

#include <QMap>
#include <QtCrypto>

class QString;
class KActionMenu;
class JabberResourcePool;
class JabberContact;
class JabberContactPool;
class JabberProtocol;
class JabberTransport;
class JabberBookmarks;


#ifdef LIBJINGLE_SUPPORT
class Libjingle;
#endif

#ifdef JINGLE_SUPPORT
class JingleCallsManager;
#endif

namespace Kopete
{
	class MetaContact;
	class StatusMessage;
}

/* @author Daniel Stone, Till Gerken */

class JabberAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT

public:
	JabberAccount (JabberProtocol * parent, const QString & accountID);
	 ~JabberAccount ();

	/* Fills the menu for this account. */
	virtual void fillActionMenu( KActionMenu *actionMenu );

	/* Return the resource of the client */
	QString resource () const;
	QString server () const;
	int port () const;

	JabberResourcePool *resourcePool ();
	JabberContactPool *contactPool ();

	/* to get the protocol from the account */
	JabberProtocol *protocol () const
	{
		return m_protocol;
	}

	JabberClient *client () const
	{
		return m_jabberClient;
	}
	
	PrivacyManager *privacyManager () const
	{
		return m_privacyManager;
	}

	// change the default S5B server port
	void setS5BServerPort ( int port );

	/* Tells the user to connect first before they can do whatever it is
	 * that they want to do. */
	void errorConnectFirst ();

	/* Tells the user that the connection was lost while we waited for
	 * an answer of him. */
	void errorConnectionLost ();

	/**
	 * Handle a TLS warning. Displays a dialog and returns if the
	 * stream can be continued or not.
	 *
	 * @param client JabberClient instance
	 * @param identityResult Peer identity checking result from QCA::TLS
	 * @param validityResult Certificate validity checking result from QCA::TLS
	 * @return True if stream can be resumed.
	 */
	static bool handleTLSWarning ( JabberClient *client, QCA::TLS::IdentityResult identityResult, QCA::Validity validityResult );

	/*
	 * Handle stream errors. Displays a dialog and returns.
	 */
	static void handleStreamError (int streamError, int streamCondition, int connectorCode, const QString &server, Kopete::Account::DisconnectReason &errorClass, QString additionalErrMsg);

	const QMap<QString, JabberTransport *> &transports()
	{ return m_transports; }


	/**
	 * called when the account is removed in the config ui
	*/
	virtual bool removeAccount();

	/**
	 * return if kopete show all messges from all resources to one session
	 */
	bool mergeMessages();

	/**
	 * set if kopete will merge all messages to one session
	 */
	void setMergeMessages(bool b);

	/**
	 * return if kopete use old inline PGP format
	 */
	bool oldEncrypted();

	/**
	 * set if kopte use old inline PGP format
	 */
	void setOldEncrypted(bool b);
	
#ifdef LIBJINGLE_SUPPORT
	bool enabledLibjingle();
	void enableLibjingle(bool b);
	bool supportLibjingle(const QString &user);
	void makeLibjingleCall(const QString &user);
#endif

#ifdef JINGLE_SUPPORT
	JingleCallsManager *jingleCallsManager() const {return m_jcm;}
#endif

public slots:
	/* Connects to the server. */
	void connectWithPassword ( const QString &password );

	/* Disconnects from the server. */
	void disconnect ();

	/* Disconnect with a reason */
	void disconnect ( Kopete::Account::DisconnectReason reason );

    /* Disconnect with a reason, and status */
    void disconnect( Kopete::Account::DisconnectReason reason, XMPP::Status &status );
	/* Reimplemented from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                      const OnlineStatusOptions& options = None );
	void setStatusMessage( const Kopete::StatusMessage &statusMessage );

	void addTransport( JabberTransport *tr ,  const QString &jid);
	void removeTransport( const QString &jid );


protected:
	/**
	 * Create a new contact in the specified metacontact
	 *
	 * You shouldn't ever call this method yourself, For adding contacts see @ref addContact()
	 *
	 * This method is called by @ref Kopete::Account::addContact() in this method, you should
	 * simply create the new custom @ref Kopete::Contact in the given metacontact. You should
	 * NOT add the contact to the server here as this method gets only called when synchronizing
	 * the contact list on disk with the one in memory. As such, all created contacts from this
	 * method should have the "dirty" flag set.
	 *
	 * This method should simply be used to intantiate the new contact, everything else
	 * (updating the GUI, parenting to meta contact, etc.) is being taken care of.
	 *
	 * @param contactId The unique ID for this protocol
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool createContact (const QString & contactID, Kopete::MetaContact * parentContact);



private:
	JabberProtocol *m_protocol;

	// backend for this account
	JabberClient *m_jabberClient;
	
	PrivacyManager *m_privacyManager;

	JabberResourcePool *m_resourcePool;
	JabberContactPool *m_contactPool;
	JabberBookmarks *m_bookmarks;

	/* Set up our actions for the status menu. */
	void initActions ();

	void cleanup ();

	/* Initial presence to set after connecting. */
	XMPP::Status m_initialPresence;

	/**
	 * Sets our own presence. Updates our resource in the
	 * resource pool and sends a presence packet to the server.
	 */
	void setPresence ( const XMPP::Status &status );

	/**
	 * Returns if a connection attempt is currently in progress.
	 */
	bool isConnecting ();

	QMap<QString, JabberTransport*> m_transports;

	/* used in removeAccount() */
	bool m_removing;
	/* keep track if we told the user we were not able to bind the
	   jabber transfer port, to avoid popup insanity */
	bool m_notifiedUserCannotBindTransferPort;

	/* last resource for this account */
	QString m_lastResource;
	XMPP::Resource m_lastXMPPResource;

	/* last status for this account */
	XMPP::Status m_lastStatus;

#ifdef LIBJINGLE_SUPPORT
	Libjingle * m_libjingle;
#endif

#ifdef JINGLE_SUPPORT
	JingleCallsManager *m_jcm;
#endif
private slots:
	/* Connects to the server. */
	void slotConnect ();

	/* Disconnects from the server. */
	void slotDisconnect ();

	// handle a TLS warning
	void slotHandleTLSWarning ( QCA::TLS::IdentityResult identityResult, QCA::Validity validityResult );

	// handle client errors
	void slotClientError ( JabberClient::ErrorCode errorCode );

	// we are connected to the server
	void slotConnected ();

	/* Called from Psi: tells us when we've been disconnected from the server. */
	void slotCSDisconnected ();

	/* Called from Psi: alerts us to a protocol error. */
	void slotCSError (int);

	/* Called from Psi: roster request finished */
	void slotRosterRequestFinished ( bool success );

	/* Called from Psi: incoming file transfer */
	void slotIncomingFileTransfer ();

	/* Called from Psi: debug messages from the backend. */
	void slotClientDebugMessage (const QString &msg);

	/* XMPP console dialog */
	void slotXMPPConsole ();

	void slotSetMood();

	/* Slots for handling groupchats. */
	void slotJoinNewChat ();
	void slotGroupChatJoined ( const XMPP::Jid &jid );
	void slotGroupChatLeft ( const XMPP::Jid &jid );
	void slotGroupChatPresence ( const XMPP::Jid &jid, const XMPP::Status &status );
	void slotGroupChatError ( const XMPP::Jid &jid, int error, const QString &reason );

	/* Incoming subscription request. */
	void slotSubscription ( const XMPP::Jid &jid, const QString &type );

	/* the dialog that asked to add the contact was closed   (that dialog is shown in slotSubscription) */
	void slotAddedInfoEventActionActivated ( uint actionId );

	/**
	 * A new item appeared in our roster, synch it with the
	 * contact list.
	 * (or the contact has been updated
	 */
	void slotContactUpdated ( const XMPP::RosterItem & );

	/**
	 * An item has been deleted from our roster,
	 * delete it from our contact pool.
	 */
	void slotContactDeleted ( const XMPP::RosterItem & );


	/* Someone on our contact list had (another) resource come online. */
	void slotResourceAvailable ( const XMPP::Jid &, const XMPP::Resource & );

	/* Someone on our contact list had (another) resource go offline. */
	void slotResourceUnavailable ( const XMPP::Jid &, const XMPP::Resource & );

	/* Displays a new message. */
	void slotReceivedMessage ( const XMPP::Message & );

	/* Gets the user's vCard from the server for editing. */
	void slotEditVCard ();

	/* Get the services list from the server for management. */
	void slotGetServices ();

	/* we received a voice invitation */
	//void slotIncomingVoiceCall(const Jid&);

	/* the unregister task finished */
	void slotUnregisterFinished();

	/* update our capabilities for myself contact */
	void slotUpdateOurCapabilities();

#ifdef LIBJINGLE_SUPPORT
	void loginLibjingle();
	void loginLibjingleResolver(const QHostAddress &address, quint16 port);
#endif

	//void slotIncomingJingleSession(const QString &sessionType, JingleSession *session);
};

/*class JabberMoodAction : public KAction
{
	Q_OBJECT
public:
	JabberMoodAction(const Mood::Type type, QObject *parent);

public slots:
	void triggered();

signals:
	void triggered(const Mood::Type type);

private:
	Mood::Type mType;
};*/


#endif
