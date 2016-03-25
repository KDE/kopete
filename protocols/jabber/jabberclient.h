
/***************************************************************************
                jabberclient.h - Generic Jabber Client Class
                             -------------------
    begin                : Sat May 25 2005
    copyright            : (C) 2005 by Till Gerken <till@tantalo.net>
                           (C) 2006 by Michaël Larouche <larouche@kde.org>

    Copyright 2006 by Tommi Rantala <tommi.rantala@cs.helsinki.fi>

			   Kopete (C) 2001-2006 Kopete developers
			   <kopete-devel@kde.org>.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERCLIENT_H
#define JABBERCLIENT_H

#include <qobject.h>

// include these because of namespace reasons
#include <im.h>
#include <xmpp.h>
#include <s5b.h>

#include <QtCrypto>

#include <kopete_export.h>

using namespace XMPP;

namespace XMPP { class PrivacyManager; }

/**
 * This class provides an interface to the Iris subsystem. The goal is to
 * abstract the Iris layer and manage it via a single, simple to use class.
 * By default, @ref JabberClient will attempt to establish a connection
 * using XMPP 1.0. This means that apart from the JID and password, no
 * further details are necessary to connect. The server and port will be
 * determined using a SRV lookup. If TLS is possible (meaning, the TLS
 * plugin is available and the server supports TLS) it will automatically
 * be used. Otherwise, a non-encrypted connection will be established.
 * If XMPP 1.0 is not possible, the connection will fall back to the old
 * protocol. By default, this connection is not encrypted. You can, however,
 * use @ref setUseSSL to immediately attempt an SSL connection. This is
 * most useful if you want to establish an SSL connection to a non-standard
 * port, in which case you will also have to use @ref setOverrideHost. In case
 * XMPP 1.0 does not work, an automatic attempt to connect to the standard port
 * 5223 with SSL can be made with @ref setProbeSSL. If the attempt is not
 * sucessful, the connection will fall back to an unencrypted attempt
 * at port 5222.
 * @brief Provides a Jabber client
 * @author Till Gerken
 */
class JABBER_EXPORT JabberClient : public QObject
{

Q_OBJECT

public:
	/**
	 * Error codes indicating problems during operation.
	 */
	enum ErrorCode
	{
		Ok,					/** No error. */
		InvalidPassword,	/** Password used to connect to the server was incorrect. */
		AlreadyConnected,	/** A new connection was attempted while the previous one hasn't been closed. */
		NoTLS,				/** Use of TLS has been forced (see @ref forceTLS) but TLS is not available, either server- or client-side. */
		InvalidPasswordForMUC = 401,	/** A password is require to enter on this Multi-User Chat */
		NicknameConflict = 409,		/** There is already someone with that nick connected to the Multi-User Chat */
		BannedFromThisMUC = 403,	/** You can't join this Multi-User Chat because you were bannished */
		MaxUsersReachedForThisMuc = 503	/** You can't join this Multi-User Chat because it is full */
	};

	JabberClient();
	~JabberClient();

	/**
	 * Connect to a Jabber server.
	 * @param jid JID to connect to.
	 * @param password Password to authenticate with.
	 * @param auth True if authentication should be done, false if not.
	 */
	ErrorCode connect ( const XMPP::Jid &jid, const QString &password, bool auth = true );

	/**
	 * Disconnect from Jabber server.
	 */
	void disconnect ();
	
	/**
	 * Disconnect from Jabber server with reason
	 * @param reason The reason for disconnecting
	 */
	void disconnect (XMPP::Status &reason);

	/**
	 * Returns if this instance is connected to a server.
	 */
	bool isConnected () const;

	/**
	 * Returns the JID associated with this instance.
	 */
	XMPP::Jid jid () const;

	/**
	 * Set flag to ignore TLS warnings. If TLS
	 * warnings are not ignored, the class will emit
	 * @ref tlsWarning and wait for the user to
	 * call @ref continueAfterTLSWarning or
	 * @ref disconnect. Default is false.
	 */
	void setIgnoreTLSWarnings ( bool flag );
	/**
	 * Return if TLS warnings are being ignored.
	 */
	bool ignoreTLSWarnings ();

	/**
	 * Continue after a @ref tlsWarning signal.
	 */
	void continueAfterTLSWarning ();

	/**
	 * Set the port on which the S5B server should listen.
	 * This is only taken into account if @ref setFileTransfersEnabled
	 * is set to true.
	 * @return True if port could be bound, false if not.
	 */
	bool setS5BServerPort ( int port );
	/**
	 * Returns the port the S5B server listens on.
	 */
	int s5bServerPort () const;

	/**
	 * Force the use of TLS. If TLS connections are forced,
	 * unencrypted connections will not be established.
	 * Default is false.
	 */
	void setForceTLS ( bool flag );
	/**
	 * Returns if TLS connections are forced.
	 */
	bool forceTLS () const;
	
	/**
	 * Force direct SSL connection, also for the
	 * handshake. This is only useful if you know
	 * the server supports it or you want to use
	 * a non-standard port, in which case @ref setOverrideHost
	 * will be useful. Default is false.
	 */
	void setUseSSL ( bool flag );
	/**
	 * Returns if an SSL connection attempt should be made.
	 */
	bool useSSL () const;

	/**
	 * Use only the old protocol (pre-XMPP 1.0). This should only
	 * be used with servers not supporting XMPP 1.0 or with servers
	 * that have a broken login procedure. Default is false. If
	 * a connection attempt is not possible, Iris will automatically
	 * fall back to the old protocol.
	 */
	void setUseXMPP09 ( bool flag );
	/**
	 * Returns if the old protocol should be used.
	 */
	bool useXMPP09 () const;

	/**
	 * Probe port 5223 if an SSL connection is possible. If
	 * a connection is not possible, an unencrypted connection
	 * will be attempted at port 5222. This is only meaningful
	 * if @ref useXMPP09 is true. Default is false.
	 */
	void setProbeSSL ( bool flag );
	/**
	 * Returns if SSL support will be probed.
	 */
	bool probeSSL () const;

	/**
	 * Override the name and port of the server to connect to.
	 * This only has an effect if the old protocol (@ref useXMPP09)
	 * has been enabled. Default is false.
	 */
	void setOverrideHost ( bool flag, const QString &server = "", int port = 5222 );
	/**
	 * Returns if the server name and port are overridden.
	 */
	bool overrideHost () const;

	/**
	 * Allow the transmission of a plain text password. If digested
	 * passwords are supported by the server, they will still be preferred.
	 * Defaults to true.
	 */
	void setAllowPlainTextPassword ( bool flag );
	/**
	 * Returns if plain text passwords are allowed.
	 */
	bool allowPlainTextPassword () const;

	/**
	 * Enable file transfers. Default is false.
	 * @param flag Whether to enable file transfers.
	 * @param localAddress Local address to receive file transfers at. Will be determined automatically if not specified.
	 */
	void setFileTransfersEnabled ( bool flag, const QString &localAddress = QString() );

	/**
	 * Returns the address of the local interface.
	 */
	QString localAddress () const;

	/**
	 * Returns if file transfers are enabled.
	 */
	bool fileTransfersEnabled () const;

	/**
	 * Set client name.
	 */
	void setClientName ( const QString &clientName );
	/**
	 * Return client name.
	 */
	QString clientName () const;

	/**
	 * Set client version.
	 */
	void setClientVersion ( const QString &clientVersion );
	/**
	 * Return client version.
	 */
	QString clientVersion () const;

	/**
	 * Set operating system name.
	 */
	void setOSName ( const QString &osName );
	/**
	 * Return operating system name.
	 */
	QString osName () const;

	/**
	 * Set the caps(JEP-0115: Entity capabilities) node name.
	 * @param node Node name.
	 */
	void setCapsNode( const QString &capsNode );
	/**
	 * Return the caps node name for this client.
	 * @return the caps node name.
	 */
	QString capsNode() const;
	
	/**
	 * Set the caps(JEP-0115: Entity capabilities) node version.
	 * @param capsVersion the node version.
	 */
	void setCapsVersion( const QString &capsVersion );
	/**
	 * Return the caps version for this client.
	 * @return the caps version.
	 */
	QString capsVersion() const;

	/**
	 * Set the disco Identity information for this client.
	 * Create a Disco identity like this:
	 * @code
	 * DiscoItem::Identity identity;
	 * identity.category = "client";
	 * identity.type = "pc";
	 * identity.name = "Kopete";
	 * @endcode
	 *
	 * @param identity DiscoItem::Identity for the client.
	 */
	void setDiscoIdentity(DiscoItem::Identity identity);
	/**
	 * Get the disco Identity information for this client.
	 * @return the DiscoItem::Identity for this client.
	 */
	DiscoItem::Identity discoIdentity() const;

	/**
	 * Set timezone information. Default is UTC.
	 */
	void setTimeZone ( const QString &timeZoneName, int timeZoneOffset );
	/**
	 * Return timezone name.
	 */
	QString timeZoneName () const;
	/**
	 * Return timezone offset.
	 */
	int timeZoneOffset () const;

	/**
	 * This method can be used to implement a penalty
	 * system when a lot of queries need to be sent to the
	 * server. Using the time returned by this method,
	 * the caller can determine a delay until the next
	 * operation in the queue can be carried out.
	 * @brief Return current penalty time in seconds.
	 */
	int getPenaltyTime ();

	/**
	 * Return the XMPP client instance.
	 */
	XMPP::Client *client () const;

	/**
	 * Return client stream instance.
	 */
	XMPP::ClientStream *clientStream () const;

	/**
	 * Return client connector instance.
	 */
	XMPP::AdvancedConnector *clientConnector () const;

	/**
	 * Get the root task for this connection.
	 * You need this instance for every task
	 * you want to start.
	 */
	XMPP::Task *rootTask () const;

	/**
	 * Returns the file transfer manager
	 * instance that deals with current file
	 * transfers.
	 */
	XMPP::FileTransferManager *fileTransferManager () const;
	
	/**
	 * Returns the privacy lists manager
	 */
	PrivacyManager *privacyManager () const;

	/**
	 * Join a groupchat.
	 * @param host Node to join the room at.
	 * @param room Name of room to join.
	 * @param nick Nick name you want to join with.
	 */
	void joinGroupChat ( const QString &host, const QString &room, const QString &nick );

	/**
	 * Join a groupchat that require a password.
	 * @param host Node to join the room at.
	 * @param room Name of room to join.
	 * @param nick Nick name you want to join with.
	 * @param password The password to join the room.
	 */
	void joinGroupChat ( const QString &host, const QString &room, const QString &nick, const QString &password );

	/**
	 * Leave a groupchat.
	 * @param host Node to leave room at.
	 * @param room Name of room to leave.
	 */
	void leaveGroupChat ( const QString &host, const QString &room );
	
	/**
	 * change the status of a groupchat
	 */
	void setGroupChatStatus(const QString &host, const QString &room, const XMPP::Status &);
	/**
	 * change the nick in a groupchat
	 */
	void changeGroupChatNick(const QString &host, const QString &room, const QString &nick, const XMPP::Status &status =XMPP::Status());

	/**
	 * Send a message.
	 */
	void sendMessage ( const XMPP::Message &message );

	/**
	 * Send raw packet to the server.
	 */
	void send ( const QString &packet );

	/**
	 * Request the roster from the Jabber server.
	 */
	void requestRoster ();

signals:
	/**
	 * Connected successfully.
	 */
	void connected ();

	/**
	 * Client stream authenticated. This
	 * signal is emitted when the socket
	 * connection has been successfully
	 * established, before sending the login
	 * packet.
	 */
	void csAuthenticated ();

	/**
	 * Client stream error.
	 */
	void csError ( int error );

	/**
	 * Client stream was disconnected.
	 */
	void csDisconnected ();

	/**
	 * TLS problem encountered.
	 */
	void tlsWarning ( QCA::TLS::IdentityResult, QCA::Validity );

	/**
	 * A new file transfer needs to be handled.
	 * The file transfer can be dealt with by
	 * querying the file transfer manager from
	 * @ref client.
	 */
	void incomingFileTransfer ();

	/**
	 * Fatal error has been encountered,
	 * further operations are not possible.
	 */
	void error ( JabberClient::ErrorCode code );

	/**
	 * Roster has been transmitted and processed.
	 */
	void rosterRequestFinished ( bool success );

	/**
	 * A new contact appeared on the roster.
	 */
	void newContact ( const XMPP::RosterItem &item );

	/**
	 * A contact has been removed from the roster.
	 */
	void contactDeleted ( const XMPP::RosterItem &item );

	/**
	 * A roster item has changed.
	 */
	void contactUpdated ( const XMPP::RosterItem &item );

	/**
	 * New resource is available for a contact.
	 */
	void resourceAvailable ( const XMPP::Jid &jid, const XMPP::Resource &resource );

	/**
	 * An existing resource has been removed.
	 */
	void resourceUnavailable ( const XMPP::Jid &jid, const XMPP::Resource &resource );

	/**
	 * A new message has been received.
	 */
	void messageReceived ( const XMPP::Message &message );

	/**
	 * Group chat has been joined.
	 */
	void groupChatJoined ( const XMPP::Jid &jid );

	/**
	 * Group chat has been left.
	 */
	void groupChatLeft ( const XMPP::Jid &jid );

	/**
	 * A presence to a groupchat has been signalled.
	 */
	void groupChatPresence ( const XMPP::Jid &jid, const XMPP::Status &status );

	/**
	 * An error was encountered joining or processing a groupchat.
	 */
	void groupChatError ( const XMPP::Jid &jid, int error, const QString &reason );

	/**
	 * New subscription request.
	 */
	void subscription ( const XMPP::Jid &jid, const QString &type );
	
	/**
	 * Dispatches a debug message. Debug messages
	 * include incoming and outgoing XML packets
	 * as well as internal status messages.
	 */
	void debugMessage ( const QString &message );
	void incomingXML (const QString &msg);
	void outgoingXML (const QString &msg);

private:
	class Private;
	Private * const d;

	/**
	 * Delete all member classes and reset the class to a predefined state.
	 */
	void cleanUp ();

	/** 
	 * Return current instance of the S5B server.
	 */
	XMPP::S5BServer *s5bServer ();
	/** 
	 * Add an address that the S5B server should handle.
	 */
	void addS5BServerAddress ( const QString &address );
	/** 
	 * Remove an address that the S5B server currently handles.
	 */
	void removeS5BServerAddress ( const QString &address );

private slots:
	/* S5B server object has been destroyed. */
	void slotS5BServerGone ();

	/* update the penalty timer */
	void slotUpdatePenaltyTime ();

	/* Login if the connection was OK. */
	void slotCSNeedAuthParams (bool user, bool pass, bool realm);

	/* Called from Psi: tells us when we're logged in OK. */
	void slotCSAuthenticated ();

	/* Called from Psi: tells us when we've been disconnected from the server. */
	void slotCSDisconnected ();

	/* Called from Psi: alerts us to a protocol warning. */
	void slotCSWarning (int);

	/* Called from Psi: alerts us to a protocol error. */
	void slotCSError (int);

	/* Called from Psi: tells us when we're connected. */
	void slotCSConnected ();

	/* Called from Psi: report certificate status */
	void slotTLSHandshaken ();

	/* Called from Psi: roster request finished */
	void slotRosterRequestFinished ( bool success, int statusCode, const QString &statusString );

	/* Called from Psi: incoming file transfer */
	void slotIncomingFileTransfer ();

	/* A new item appeared in our roster */
	void slotNewContact (const RosterItem &);

	/* An item has been deleted from our roster. */
	void slotContactDeleted (const RosterItem &);

	/* Update a contact's details. */
	void slotContactUpdated (const RosterItem &);

	/* Someone on our contact list had (another) resource come online. */
	void slotResourceAvailable (const Jid &, const Resource &);

	/* Someone on our contact list had a resource go offline. */
	void slotResourceUnavailable (const Jid &, const Resource &);

	/* Incoming message. */
	void slotReceivedMessage (const Message &);
	
	/* Called from Psi: debug messages from the backend. */
	void slotPsiDebug (const QString & msg);
	void slotIncomingXML (const QString &msg);
	void slotOutgoingXML (const QString &msg);

	/* Slots for handling groupchats. */
	void slotGroupChatJoined (const Jid & jid);
	void slotGroupChatLeft (const Jid & jid);
	void slotGroupChatPresence (const Jid & jid, const Status & status);
	void slotGroupChatError (const Jid & jid, int error, const QString & reason);

	/* Incoming subscription request. */
	void slotSubscription (const Jid & jid, const QString & type);
	
	/* the session task is finished */
	void slotSessionStarted();

};

#endif
