/*
    gwaccount.h - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed    
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef GW_ACCOUNT_H
#define GW_ACCOUNT_H

#include <qvaluelist.h>
#include <qdict.h>

#include <kopeteaccount.h>
#include <kopetemessage.h>
#include <kopetepasswordedaccount.h>

#include "gwerror.h"
#include "gwfield.h"
#include "gwmessagemanager.h"

class KActionMenu;
class KopeteContact;
class KopeteGroup;
class KopeteMetaContact;

class GroupWiseContact;
class GroupWiseProtocol;
class KNetworkConnector;
namespace QCA {
	class TLS;
}
class QCATLSHandler;
class ClientStream;
class Client;

/**
 * This represents an account connected to GroupWise
 * @author Will Stephensonconst int GroupWiseAccount::port() const
{
	return pluginData( protocol(), "Port" ).toInt();
}

const QString GroupWiseAccount::server() const

*/

using namespace GroupWise;

class GroupWiseAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT
public:
	GroupWiseAccount( GroupWiseProtocol *parent, const QString& accountID, const char *name = 0 );
	~GroupWiseAccount();
	/**
	 * Construct the context menu used for the status bar icon
	 */
	virtual KActionMenu* actionMenu();
	
	// DEBUG ONLY
	void dumpManagers();
	// DEBUG ONLY
	/**
	 * Creates a protocol specific KopeteContact subclass and adds it to the supplie
	 * KopeteMetaContact
	 */
	virtual bool addContactToMetaContact(const QString& contactId, const QString& displayName, KopeteMetaContact* parentContact);
	/**
	 * Called when Kopete is set globally away
	 */
	virtual void setAway(bool away, const QString& reason);
	/** 
	 * Utility access to the port given by the user
	 */
	const int port() const;
	/** 
	 * Utility access to the server given by the user
	 */
	const QString GroupWiseAccount::server() const;
	/**
	 * Utility access to our protocol
	 */
	GroupWiseProtocol * GroupWiseAccount::protocol() const;
	/**
	 * Utility access to the @ref Client which is the main interface exposed by libgroupwise.
	 * Most protocol actions are carried out using the client's member functions but the possibility exists
	 * to start Tasks directly on the client and respond directly to their signals.
	 */
	Client * client() const;
	/** 
	 * Utility access to a message manager instance for a given GUID
	 */
	GroupWiseMessageManager * messageManager( const KopeteContact* user, KopeteContactPtrList others, KopeteProtocol* protocol, const ConferenceGuid & guid );
	/**
	 * Look up a contact given a DN
	 * Returns 0 if none found
	 */
	GroupWiseContact * contactForDN( const QString & dn );
	/**
	 * Create a conference (start a chat) on the server
	 */
	void createConference( const int clientId, const QStringList& invitees );
	
	/**
	 * Send a message
	 */ 
	void sendMessage( const ConferenceGuid & guid, const KopeteMessage & message );
	
	/**
	 * Invite someone to join a conference
	 */
	void sendInvitation( const ConferenceGuid & guid, const QString & dn, const QString & message );
	
	/**
	 * Check a contact's blocking status
	 * Only works when connected - otherwise always returns false
	 */
	bool isContactBlocked( const QString & m_dn );
	/**
	 * Set up a temporary contact (not on our contact list but is messaging us or involved in a conversation that we have been invited to.
	 */
	GroupWiseContact * createTemporaryContact( const QString & dn );
public slots:

	void slotTestRTFize();
	
	/* Connects to the server. */
	virtual void connectWithPassword ( const QString &password );

	/* Disconnects from the server. */
	virtual void disconnect();
	virtual void disconnect( KopeteAccount::DisconnectReason reason );
signals: 
	void conferenceCreated( const int mmId, const GroupWise::ConferenceGuid & guid );
	void conferenceCreationFailed( const int mmId, const int statusCode );
	void contactTyping( const ConferenceEvent & );
	void contactNotTyping( const ConferenceEvent & );
	void privacyChanged( const QString & dn, bool allowed );
protected slots:
	/**
	 * Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOnline();
	void slotGoAway( const QString & reason );
	void slotGoOffline();
	void slotGoBusy( const QString & reason );
	void slotGoAppearOffline();
	/**
	 * Set an auto reply message for use when the account is away
	 * TODO: Extend KopeteAwayAction so you can set multiple ones there.
	 */
	void slotSetAutoReply();
	/**
	 * Manage the user's privacy settings
	 */
	void slotPrivacy();
 	void slotKopeteGroupRenamed( KopeteGroup * );
	void slotKopeteGroupRemoved( KopeteGroup * );

	// SERVER SIDE CONTACT LIST PROCESSING
	/**
	 * Called when we receive a FOLDER from the server side contact list
	 * Adds to the Kopete contact list if not already present.
	 */
	void receiveFolder( const FolderItem & folder );
	/**
	 * Called when we receive a CONTACT from the server side contact list
	 * Adds to a folder in the Kopete contact list.
	 */
	void receiveContact( const ContactItem & );
	/**
	 * Called when we receive a CONTACT'S METADATA (including initial status) from the server side contact list,
	 * or in response to an explicity query.  This is necessary to handle some events from the server.  
	 * These events are queued in the account until the data arrives and then we handle the event.
	 */
	void receiveContactUserDetails( const GroupWise::ContactDetails & );
	/**
	 * Called after we create a contact on the server 
	 */
	void receiveContactCreated();
	// SLOTS HANDLING PROTOCOL EVENTS
	/**
	 * Called when the server has a message for us.  
	 * This identifies the sending KopeteContact and passes the message on to it,
	 * in order to locate the MessageManager and finally pass to the GUI.
	 */
	void receiveMessage( const ConferenceEvent & event );
	void receiveAutoReply( const ConferenceEvent & event );
	/**
	 * A contact changed status
	 */
	void receiveStatus( const QString &, Q_UINT16, const QString & );
	/**
	 * Our status changed on the server
	 */
	void changeOurStatus( GroupWise::Status, const QString &, const QString & );
	/**
	 * Called when we've been disconnected for logging in as this user somewhere else
	 */ 
	void slotConnectedElsewhere();
	/** 
	 * Called when we've logged in successfully
	 */
	void slotLoggedIn();
	/** 
	 * Called when a login attempt failed
	 */
	void slotLoginFailed();
	/**
	 * We joined a conference having accepted an invitation, create a message manager
	 */
 	void receiveConferenceJoin( const GroupWise::ConferenceGuid & guid, const QStringList & participants, const QStringList & invitees );
	/**
	 * Someone joined a conference, add them to the appropriate message manager
	 */
 	void receiveConferenceJoinNotify( const ConferenceEvent & );
	/**
	 * Someone left a conference, remove them from the message manager
	 */
 	void receiveConferenceLeft( const ConferenceEvent & );
	/**
	 * The user was invited to join a conference
	 */
 	void receiveInvitation( const ConferenceEvent & );
	/**
	 * Notification that a third party was invited to join conference
	 */
 	void receiveInviteNotify( const ConferenceEvent & );
	/**
	 * Notification that a third party declined an invitation
	 */
 	void receiveInviteDeclined( const ConferenceEvent & );
	/**
	 * A conference was closed by the server because everyone has left or declined invitations
	 * Prevents any further messages to this conference
	 */
// 	void closeConference();
	// SLOTS HANDLING NETWORK EVENTS
	/**
	 * Update the local user's metadata
	 */
	void receiveAccountDetails( const GroupWise::ContactDetails & details );
	/**
	 * The TLS handshake has happened, check the result
	 */
	void slotTLSHandshaken();
	/** The connection is ready for a login */
	void slotTLSReady( int secLayerCode );
	/**
	 * Called when the clientstream is connected, debug only
	 */
	void slotCSConnected();
	/**
	 * Performs necessary actions when the client stream has been disconnected
	 */
	void slotCSDisconnected();
	void slotCSError( int error );
	void slotCSWarning( int warning );
	
	// HOUSEKEEPING
	/**
	 * Because a message manager that we create will get a GUID from the server some time after it is created,
	 * when we get a GUID back after a successful conference create, we signal the GUID and the message manager's internal ID
	 * using conferenceCreated, then listen for a conferenceCreated signal back from the manager, and register it in this slot
	 */
	void slotMessageManagerGotGuid();
	/**
	 * We listen for the destroyed() signal and leave any conferences we
	 * might have been in, and remove it from our map.
	 */
	void slotMessageManagerDestroyed( QObject * );
	
	/** Debug slots */
	void slotConnError();
	void slotConnConnected();
protected:
	/**
	 * Sends a status message to the server - called by the status specific slotGoAway etc
	 */
	void setStatus( GroupWise::Status status, const QString & reason = QString::null );
	/**
	 * Memory management
	 */
	void cleanup();
private:
	// current auto reply message
	QString m_autoReply;
	// Network code 
	KNetworkConnector * m_connector;
	QCA::TLS * m_QCATLS;
	QCATLSHandler *	m_tlsHandler;
	ClientStream * m_clientStream;
	// Client, entry point of libgroupwise
	Client * m_client;
	
	GroupWise::Status m_initialStatus;
	QString m_initialReason;
	GroupWiseMessageManager::Dict m_managers;
};

#endif
