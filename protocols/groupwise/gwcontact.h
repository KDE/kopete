/*
    gwcontact.h - Kopete GroupWise Protocol

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

#ifndef GW_CONTACT_H
#define GW_CONTACT_H

#include <qdict.h>
#include <qmap.h>

#include "kopetecontact.h"
#include "kopetemessage.h"

#include "gwerror.h"
#include "gwfield.h"

class KAction;
class KActionCollection;
class KopeteAccount;
class GroupWiseAccount;
class GroupWiseMessageManager;
class GroupWiseProtocol;
class KopeteMetaContact;

using namespace GroupWise;
/**
 * Represents an instance of a contact in the server side contact list
 */
struct ContactListInstance
{
	int objectId;
	int parentId;
	int sequence;
};

typedef QValueList< ContactListInstance > CLInstanceList;
/**
@author Will Stephenson
*/
class GroupWiseContact : public KopeteContact
{
	Q_OBJECT
public:
	/** 
	 * Constructor
	 * @param account The GroupWiseAccount this belongs to.
	 * @param uniqueName The unique identifier for this contact, in GroupWise terms, the DN.
	 * @param parent The KopeteMetaContact this contact is part of.
	 * @param objectId The contact's numeric object ID.
	 * @param parentId The ID of this contact's parent (folder).
	 * @param sequence This contact's sequence number (The position it appears in within its parent).
	 */
	GroupWiseContact( KopeteAccount* account, const QString &uniqueName, 
			KopeteMetaContact *parent, 
			const int objectId, const int parentId, const int sequence );

    ~GroupWiseContact();

	/** 
	 * Access this contact's KopeteAccount subclass
	 */
	GroupWiseAccount * account();

	/** 
	 * Access this contact's KopeteProtocol subclass
	 */
	GroupWiseProtocol * protocol();

	/**
	 * Update the contact's status and metadata from the supplied fields
	 */
	void updateDetails( const ContactDetails & details );
	
	virtual bool isReachable();
	/**
	 * Serialize the contact's data into a key-value map
	 * suitable for writing to a file
	 */
    virtual void serialize(QMap< QString, QString >& serializedData,
			QMap< QString, QString >& addressBookData);
	/**
	 * Return the actions for this contact
	 */
	virtual QPtrList<KAction> *customContextMenuActions();
	
	/**
	 * Returns a KopeteMessageManager associated with this contact
	 */
	virtual KopeteMessageManager *manager( bool canCreate = false );

	/** 
	 * Locate or create a messagemanager for the specified group of contacts
	 */
	GroupWiseMessageManager *manager ( KopeteContactPtrList chatMembers, bool canCreate = false );

	/**
	 * Received a message from the server.
	 * Find the conversation that this message belongs to, and display it there.
	 * @param autoReply Indicates that the message is an auto reply - doesn't contain any RTF.
	 */
	void handleIncomingMessage( const ConferenceEvent & event, bool autoReply );
	
	/**
	 * Add this contact to a conference
	 */
	void joinConference( const QString & guid );

	/**
	 * Remove this contact from a conference
	 */
	void leaveConference( const QString & guid );
	
	/**
	 * Access the contact's server properties
	 */
	QMap< QString, QString > serverProperties();
	
	// CONTACT LIST MANAGEMENT FUNCTIONS
	/**
	 *	These functions simulate the server side contact list structure enough to allow Kopete to manipulate it correctly
	 */
	/**
	 * Add an instance to this contact
	 */
	void addCLInstance( const ContactListInstance & );
	void removeCLInstance( const int objectId );
	bool hasCLObjectId( const int objectId ) const;
	CLInstanceList instances() const;
	
public slots:
	/**
	 * Transmits an outgoing message to the server 
	 * Called when the chat window send button has been pressed
	 * (in response to the relevant KopeteMessageManager signal)
	 */
	void sendMessage( KopeteMessage &message );
	/**
	 * Delete this contact on the server
	 */
	virtual void slotDeleteContact();
	/**
	 * Receive notification that an instance of this contact on the server was deleted
	 * If all the instance of the contact are deleted, the contact will delete itself with deleteLater()
	 */
	void receiveContactDeleted( const ContactItem & );

protected:
	/**
	 * Returns the KopeteMessageManager for the GroupWise conference with the supplied GUID, or creates a new one.
	 */
	GroupWiseMessageManager *manager( const QString & guid, bool canCreate = false );
	// debug function to see what message managers we have on the server
	void dumpManagers();
protected slots:
	/**
	 * Show the contact's properties
	 */
	void slotUserInfo();
	/**
	 * A message manager was instantiated as a conference on the server, so record it.
	 */
	void slotConferenceCreated();
	/**
	 * Notify the contact that a KopeteMessageManager was
	 * destroyed - probably by the chatwindow being closed
	 */
	void slotMessageManagerDeleted( QObject *sender );
	
protected:
	KActionCollection* m_actionCollection;
	
	int m_objectId;
	int m_parentId;
	int m_sequence;
	
	KAction* m_actionPrefs;
	// all the message managers that this contact is currently chatting via
	QDict< GroupWiseMessageManager > m_msgManagers;
	// a list of all the instances that this contact appears in the server side contact list
	CLInstanceList m_instances;
	// Novell Messenger Properties, as received by the server.  
	// Unfortunately we don't the domain of the set of keys, so they are not easily mappable to KopeteContactProperties
	QMap< QString, QString > m_serverProperties;
};

#endif
