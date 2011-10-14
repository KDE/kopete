/*
    gwaccount.h - Kopete GroupWise Protocol

    Copyright (c) 2006,2007 Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Based on Testbed
    Copyright (c) 2003-2007 by Will Stephenson		 <wstephenson@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

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

#include <QtCore/QList>
#include <QtCrypto>

#include <kaction.h>

#include <kopetechatsessionmanager.h>
#include <kopetecontact.h>

#include "gwerror.h"

#include <kopetepasswordedaccount.h>

class KActionMenu;

namespace Kopete {
	class Group;
	class MetaContact; 
}

class GroupWiseContact;
class GroupWiseChatSession;
class GroupWiseProtocol;
class KNetworkConnector;
namespace QCA {
	class TLS;
}
class QCATLSHandler;
class ClientStream;
namespace GroupWise {
	class Client;
}
class GWContactList;

using namespace GroupWise;

/**
 * This represents an account on a Novell GroupWise Messenger Server
 */
class GroupWiseAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT
public:
	GroupWiseAccount( GroupWiseProtocol *parent, const QString& accountID, const char *name = 0 );
	~GroupWiseAccount();

	/**
	 * Construct the context menu used for the status bar icon
	 */
	virtual void fillActionMenu( KActionMenu *actionMenu );

	// DEBUG ONLY
	void dumpManagers();
	// DEBUG ONLY
	/**
	 * Creates a protocol specific Kopete::Contact subclass and adds it to the supplied
	 * Kopete::MetaContact
	 */
	virtual bool createContact(const QString& contactId, Kopete::MetaContact* parentContact);
	/**
	 * Delete a contact on the server
	 */
	void deleteContact( GroupWiseContact * contact );
	/**
	 * Called when Kopete is set globally away
	 */
	virtual void setAway(bool away, const QString& reason);
	/**
	 * Utility access to the port given by the user
	 */
	int port() const;
	/**
	 * Utility access to the server given by the user
	 */
	const QString server() const;
	/**
	 * Utility access to our protocol
	 */
	GroupWiseProtocol * protocol() const;
	/**
	 * Utility access to the @ref Client which is the main interface exposed by libgroupwise.
	 * Most protocol actions are carried out using the client's member functions but the possibility exists
	 * to start Tasks directly on the client and respond directly to their signals.
	 */
	Client * client() const;
	/**
	 * Utility to create or access a message manager instance for a given GUID and set of contacts
	 */
	GroupWiseChatSession * chatSession( Kopete::ContactPtrList others, const ConferenceGuid & guid, Kopete::Contact::CanCreateFlags canCreate );
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
	void sendMessage( const ConferenceGuid & guid, const Kopete::Message & message );

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

	/**
	 * Check whether sync is not currently needed
	 */
	bool dontSync();
	
	void syncContact( GroupWiseContact * contact );
	
public slots:

	void slotTestRTFize();

	/* Connects to the server. */
	void connectWithPassword ( const QString &password );

	/* Disconnects from the server. */
	virtual void disconnect();
	virtual void disconnect( Kopete::Account::DisconnectReason reason );

	/** Set the online status for the account. Reimplemented from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                      const OnlineStatusOptions& options = None );
	/**
	 * Set the status message for the account. Reimplemented from Kopete::Account
	 */
	void setStatusMessage( const Kopete::StatusMessage &statusMessage );

signals:
	void conferenceCreated( const int mmId, const GroupWise::ConferenceGuid & guid );
	void conferenceCreationFailed( const int mmId, const int statusCode );
	void contactTyping( const ConferenceEvent & );
	void contactNotTyping( const ConferenceEvent & );
	void privacyChanged( const QString & dn, bool allowed );

	
protected slots:
    void slotMessageSendingFailed();
	/**
	 * Set an auto reply message for use when the account is away
	 * TODO: Extend Kopete::AwayAction so you can set multiple ones there.
	 */
	void slotSetAutoReply();
	/**
	 * Manage the user's privacy settings
	 */
	void slotPrivacy();
	
	/**
	 * Show a dialog to join a chatroom without first adding it to the contact list
	 */
	void slotJoinChatRoom();

	/**
	 * Slot informing GroupWise when a group is renamed
	 */
 	void slotKopeteGroupRenamed( Kopete::Group * );
	/**
	 * Slot informing GroupWise when a group is removed
	 */
	void slotKopeteGroupRemoved( Kopete::Group * );

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
	 * or in response to an explicitly query.  This is necessary to handle some events from the server.
	 * These events are queued in the account until the data arrives and then we handle the event.
	 */
	void receiveContactUserDetails( const GroupWise::ContactDetails & );
	/**
	 * Called after we create a contact on the server
	 */
	void receiveContactCreated();
	/**
	 * Handles the response to deleting a contact on the server
	 */
	void receiveContactDeleted( const ContactItem & instance );

	// SLOTS HANDLING PROTOCOL EVENTS
	/**
	* Received a message from the server.
	* Find the conversation that this message belongs to, and display it there.
	* @param event contains event type, sender, content, flags.  Type is used to handle autoreplies, normal messages, and [system] broadcasts.
	*/
	void handleIncomingMessage( const ConferenceEvent & );
	/**
	 * A contact changed status
	 */
	void receiveStatus( const QString &, quint16, const QString & );
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
	 * We listen for the destroyed() signal and leave any conferences we
	 * might have been in, and remove it from our map.
	 */
	void slotLeavingConference( GroupWiseChatSession * );

	/** Debug slots */
	void slotConnError();
	void slotConnConnected();
protected:
	/**
	 * Sends a status message to the server - called by the status specific slotGoAway etc
	 */
	//void setStatus( GroupWise::Status status, const QString & reason = QString() );

	int handleTLSWarning (QCA::TLS::IdentityResult identityResult,
		QCA::Validity validityResult, QString server, QString accountId);

	GroupWiseChatSession * findChatSessionByGuid( const GroupWise::ConferenceGuid & guid );
	/**
	 * reconcile any changes to the contact list which happened offline
	 */
	void reconcileOfflineChanges();
	/**
	 * Memory management
	 */
	void cleanup();
private:
	// action menu and its actions
	KAction * m_actionAutoReply;
	KAction * m_actionManagePrivacy;
	KAction * m_actionJoinChatRoom;
	// Network code
	KNetworkConnector * m_connector;
	QCA::Initializer m_qcaInit;
	QCA::TLS * m_QCATLS;
	QCATLSHandler *	m_tlsHandler;
	ClientStream * m_clientStream;
	// Client, entry point of libgroupwise
	Client * m_client;

	QString m_password;
	QString m_initialReason;
	QList<GroupWiseChatSession*> m_chatSessions;
	bool m_dontSync;
	GWContactList * m_serverListModel;
};

/**
 * @internal
 * An action that selects an OnlineStatus and provides a status message, but not using Kopete::Away, because the status message relates only to this status.
 */
/*class OnlineStatusMessageAction : public KAction
{
	Q_OBJECT
  public:
	OnlineStatusMessageAction ( const Kopete::OnlineStatus& status, const QString &text, const QString &message, const QIcon &pix, QObject *parent=0, const char *name=0);
  signals:
	void activated( const Kopete::OnlineStatus& status, const QString & );
  private slots:
	void slotActivated();
  private:
	Kopete::OnlineStatus m_status;
	QString m_message;
};
*/
#endif
