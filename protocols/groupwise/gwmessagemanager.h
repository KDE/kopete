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

/**
@author SUSE AG
*/
class GroupWiseMessageManager : public KopeteMessageManager
{
Q_OBJECT
public:
	GroupWiseMessageManager(const KopeteContact* user, KopeteContactPtrList others, KopeteProtocol* protocol, const QString & guid, int id = 0, const char* name = 0);
	~GroupWiseMessageManager();
	/**
	 * The conference's globally unique identifier, which is given to it by the server
	 */
	QString guid() const { return m_guid; }
	/**
	 * Change the GUID
	 */
	void setGuid( const QString & guid );
	/**
	 * Regenerate the display name (window title), to cope with group chat membership changes.
	 */
	void updateDisplayName();
	/**
	 * Utility account access
	 */
	GroupWiseAccount * account();
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
	 * Sends any messages that were queued while waiting for the conference to be created
	 */
	void dequeMessages();
protected slots:
	/** 
	 * Receive the GUID returned by the server when we start a chat.
	 * @param mmId Message Manager ID, used to determine if this GUID is meant for this message manager
	 * @param guid The GUID allotted us by the server.
	 */
	void receiveGuid( const int mmId, const QString & guid );
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
private:
	QString m_guid; // The conference's globally unique identifier, which is given to it by the server
	int m_flags; // flags for secure connections, central logging and "conference closed" as given by the server
	QValueList< KopeteMessage > m_pendingOutgoingMessages; // messages queued while we wait for the server to tell us the conference is created.
};

#endif
