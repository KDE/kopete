//
// C++ Interface: gwmessagemanager
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GWMESSAGEMANAGER_H
#define GWMESSAGEMANAGER_H

#include <qptrqueue.h>
#include <kopetemessage.h>
#include <kopetemessagemanager.h>

#include "gwerror.h"

class QLabel;
class KAction;
class KActionMenu;
class KDialogBase;
class GroupWiseAccount;
class GroupWiseContact;
class GroupWiseSearch;
/**
 * Specialised message manager, which tracks the GUID used by GroupWise to uniquely identify a given chat, and provides invite actions and logging and security indicators.  To instantiate call @ref GroupWiseAccount::messageManager().
 * @author SUSE AG
*/

using namespace GroupWise;

class GroupWiseMessageManager : public KopeteMessageManager
{
Q_OBJECT

friend class GroupWiseAccount;

public:
	/**
	 * Specialised dictionary that only keys on the first CONF_GUID_END characters of a conference GUID
	 * INNER CLASS derived from an INSTANCE of a TEMPLATE CLASS with an OVERLOADED OPERATOR []
	 */
	class Dict : public QMap< ConferenceGuid, GroupWiseMessageManager * >
	{
		// QMap::insert isn't virtual 
		public:
		void insert( const ConferenceGuid & key, GroupWiseMessageManager * item );
		GroupWiseMessageManager * operator[]( const ConferenceGuid & key );
		void remove( const ConferenceGuid & k );
	};


	~GroupWiseMessageManager();
	/**
	 * The conference's globally unique identifier, which is given to it by the server
	 */
	ConferenceGuid guid() const { return m_guid; }
	/**
	 * Change the GUID
	 */
	void setGuid( const ConferenceGuid & guid );
	/**
	 * Utility account access
	 */
	GroupWiseAccount * account();
	/**
	 * Accessors and mutators for secure chat, logged chat, and closed conference flags
	 */
	void setSecure( bool secure );
	void setLogging( bool logged );
	void setClosed();
	bool secure();
	bool logging();
	bool closed();
	/**
	 * Add invitees to the conference
	 */
	void addInvitee( const KopeteContact * );
	/**
	 * Add members to the conference
	 */
	void joined( GroupWiseContact * );
	/** 
	 * Remove members from conference
	 */
	void left( GroupWiseContact * );
	/** 
	 * An invitation was declined
	 */
	void inviteDeclined( GroupWiseContact * );
	/**
	 * Check whether the conversation being administratively logged and update the UI to indicate this
	 */
	void updateArchiving();
	/**
	 * Reimplemented from KopeteMessageManager - invites contacts via DND
	 */
	virtual void inviteContact(const QString& );
signals:
	/**
	 * Tell the contact we got a GUID so it can route incoming messages here.
	 */
	void conferenceCreated();
	/**
	 * Tell the contact that the server wouldn't create a conference
	 */
	// NOT DECIDED IF WE NEED THIS YET
protected:
	/**
	 * Start the process of creating a conference for this GWMM on the server.
	 */
	void GroupWiseMessageManager::createConference();
	/** 
	 * Sends any messages and invitations that were queued while waiting for the conference to be created
	 */
	void dequeueMessagesAndInvites();
protected slots:
	/** 
	 * Receive the GUID returned by the server when we start a chat.
	 * @param mmId Message Manager ID, used to determine if this GUID is meant for this message manager
	 * @param guid The GUID allotted us by the server.
	 */
	void receiveGuid( const int mmId, const GroupWise::ConferenceGuid & guid );
	/**
	 * An attempt to create a conference on the server failed.
	 * @param mmId Message Manager ID to see if the failure refers to this message manager
	 */
	void slotCreationFailed( const int mmId,const int statusCode );
	 
	void slotSendTypingNotification ( bool typing );
	void slotMessageSent( KopeteMessage &message, KopeteMessageManager * );
	// TODO: slots for us leaving conference, us inviting someone, someone joining, someone leaving, someone sending an invitation, getting typing?
	void slotGotTypingNotification( const ConferenceEvent & );
	void slotGotNotTypingNotification( const ConferenceEvent & );
	/**
	 * Popupulate the menu of invitable contacts
	 */
	void slotActionInviteAboutToShow();
	/**
	 * Invite a contact to join this chat
	 */
	void slotInviteContact( KopeteContact * );
	/** 
	 * Show the search dialog to invite another contact to the chat
	 */
	void slotInviteOtherContact();
	/**
	 * Process the response from the search dialog; send the actual invitation
	 */
	void slotSearchedForUsers();

	void slotShowSecurity();
	void slotShowArchiving();
private:
	
	GroupWiseMessageManager(const KopeteContact* user, KopeteContactPtrList others, KopeteProtocol* protocol, const ConferenceGuid & guid, int id = 0, const char* name = 0);
	
	ConferenceGuid m_guid; // The conference's globally unique identifier, which is given to it by the server
	int m_flags; // flags for secure connections, central logging and "conference closed" as given by the server
	
	QValueList< KopeteMessage > m_pendingOutgoingMessages; // messages queued while we wait for the server to tell us the conference is created.
	KopeteContactPtrList m_pendingInvites; // people we wanted to invite to the conference, queued while waiting for the conference to be created.
	KActionMenu *m_actionInvite;
	QPtrList<KAction> m_inviteActions;
	// labels showing secure and logging status
	KAction *m_secure;
	KAction *m_logging;
	// search widget and dialog used for inviting contacts
	GroupWiseSearch * m_search;
	KDialogBase * m_searchDlg;
	// contacts who have been invited to join but have not yet joined the chat
	KopeteContactPtrList m_invitees;
	// track the number of members actually in the chat
	uint m_memberCount;

};

#endif
