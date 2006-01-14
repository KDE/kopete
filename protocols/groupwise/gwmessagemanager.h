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
#include <kopetechatsession.h>

#include "gwerror.h"

class QLabel;
class KAction;
class KActionMenu;
class KDialogBase;
class GroupWiseAccount;
class GroupWiseContact;
class GroupWiseContactSearch;
/**
 * Specialised message manager, which tracks the GUID used by GroupWise to uniquely identify a given chat, and provides invite actions and logging and security indicators.  To instantiate call @ref GroupWiseAccount::chatSession().
 * @author SUSE AG
*/

using namespace GroupWise;

class GroupWiseChatSession : public Kopete::ChatSession
{
Q_OBJECT

friend class GroupWiseAccount;

public:
	/**
     * The destructor emits leavingConference so that the account can tell the server that the user has left the chat
	 */
	~GroupWiseChatSession();
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
	void addInvitee( const Kopete::Contact * );
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
	 * Reimplemented from Kopete::ChatSession - invites contacts via DND
	 */
	virtual void inviteContact(const QString& );
signals:
	/**
	 * Tell the contact we got a GUID so it can route incoming messages here.
	 */
	void conferenceCreated();
	/**
	 * Tell the account that the GroupWiseChatSession is closing so it can tell the server that the user has left the conference
	 */
	void leavingConference( GroupWiseChatSession * );
protected:
	/**
	 * Start the process of creating a conference for this GWMM on the server.
	 */
	void createConference();
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
	void slotMessageSent( Kopete::Message &message, Kopete::ChatSession * );
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
	void slotInviteContact( Kopete::Contact * );
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
	
	GroupWiseChatSession(const Kopete::Contact* user, Kopete::ContactPtrList others, Kopete::Protocol* protocol, const ConferenceGuid & guid, int id = 0, const char* name = 0);
	
	ConferenceGuid m_guid; // The conference's globally unique identifier, which is given to it by the server
	int m_flags; // flags for secure connections, central logging and "conference closed" as given by the server
	
	QValueList< Kopete::Message > m_pendingOutgoingMessages; // messages queued while we wait for the server to tell us the conference is created.
	Kopete::ContactPtrList m_pendingInvites; // people we wanted to invite to the conference, queued while waiting for the conference to be created.
	KActionMenu *m_actionInvite;
	QPtrList<KAction> m_inviteActions;
	// labels showing secure and logging status
	KAction *m_secure;
	KAction *m_logging;
	// search widget and dialog used for inviting contacts
	GroupWiseContactSearch * m_search;
	KDialogBase * m_searchDlg;
	// contacts who have been invited to join but have not yet joined the chat
	Kopete::ContactPtrList m_invitees;
	// track the number of members actually in the chat
	uint m_memberCount;
	
	/**
	 * return an unique identifier for that kmm
	 * @todo check it!
	*/
	uint mmId() const;
	uint m_mmId;

};

#endif
