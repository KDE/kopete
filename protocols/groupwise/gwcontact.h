/*
    gwcontact.h - Kopete GroupWise Protocol

    Copyright (c) 2006      Novell, Inc	http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson <will@stevello.free-online.co.uk>
    
	Blocking status taken from MSN
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Ryan Cumming <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002-2003 by Martijn Klingens <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart <ogoffart@kde.org>
    
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

#ifndef GW_CONTACT_H
#define GW_CONTACT_H

#include <QMap>


#include "kopetecontact.h"
#include "kopetemessage.h"

#include "gwerror.h"
#include "gwfield.h"
#include "gwmessagemanager.h"

class KAction;
class KActionCollection;
namespace Kopete { class Account; }
class GroupWiseAccount;
class GroupWiseProtocol;
namespace Kopete { class MetaContact; }

using namespace GroupWise;

/**
@author Will Stephenson
*/
class GroupWiseContact : public Kopete::Contact
{
	Q_OBJECT
public:
	/** 
	 * Constructor
	 * @param account The GroupWiseAccount this belongs to.
	 * @param uniqueName The userId for this contact.  May be a DN, in which case it will be converted to dotted format for the contactId and stored.
	 * @param parent The Kopete::MetaContact this contact is part of.
	 * @param objectId The contact's numeric object ID.
	 * @param parentId The ID of this contact's parent (folder).
	 * @param sequence This contact's sequence number (The position it appears in within its parent).
	 */
	GroupWiseContact( Kopete::Account* account, const QString &uniqueName, 
			Kopete::MetaContact *parent, 
			const int objectId, const int parentId, const int sequence );

    ~GroupWiseContact();

	/** 
	 * Access this contact's Kopete::Account subclass
	 */
	GroupWiseAccount * account();

	/** 
	 * Access this contact's Kopete::Protocol subclass
	 */
	GroupWiseProtocol * protocol();

	/**
	 * Get the contact's DN (used for communications with the server, not the contactId )
	 */
	QString dn() const;
	
	/**
	 * Update the contact's status and metadata from the supplied fields
	 */
	void updateDetails( const GroupWise::ContactDetails & details );
	
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
	virtual QList<KAction *> *customContextMenuActions();
	using Kopete::Contact::customContextMenuActions;
	
	/**
	 * Returns a Kopete::ChatSession associated with this contact
	 */
	virtual Kopete::ChatSession *manager( Kopete::Contact::CanCreateFlags canCreate =  Kopete::Contact::CannotCreate );

	/**
	 * Access the contact's server properties
	 */
	QMap< QString, QVariant > serverProperties();
	/** 
	 * Updates this contact's group membership and display name on the server
	 */
	void sync( unsigned int);
	/**
	 * Updates this contact's online status, including blocking status
	 */
	void setOnlineStatus(const Kopete::OnlineStatus& status);
	/**
	 * Are this contact's chats being administratively logged?
	 */
	bool archiving() const;
	/**
	 * Is this contact in the process of being deleted
	 */
	bool deleting() const;
	/**
	 * Mark this contact as being deleted
	 */
	void setDeleting( bool deleting );
	/**
	 * Marks this contact as having sent a message whilst apparently offline
	 */
	void setMessageReceivedOffline( bool on );
	/**
	 * Has this contact sent a message whilst apparently offline?
	 */
	bool messageReceivedOffline() const;

public slots:
	/**
	 * Transmits an outgoing message to the server 
	 * Called when the chat window send button has been pressed
	 * (in response to the relevant Kopete::ChatSession signal)
	 */
	void sendMessage( Kopete::Message &message );
	/**
	 * Delete this contact on the server
	 */
	virtual void deleteContact();
	/**
	 * Called when the call to rename the contact on the server has completed
	 */
	void renamedOnServer();
	
protected:
	// debug function to see what message managers we have on the server
	void dumpManagers();
protected slots:
	/**
	 * Show the contact's properties
	 */
	void slotUserInfo();
	/**
	 * Block or unblock the contact, toggle its current blocking state
	 */
	void slotBlock();
	/**
	 * Receive notification that this contact's privacy setting changed - update status
	 */
	void receivePrivacyChanged( const QString &, bool );
protected:
	KActionCollection* m_actionCollection;
	
	int m_objectId;
	int m_parentId;
	int m_sequence;
	QString m_dn;
	QString m_displayName;
	KAction* m_actionPrefs;
	KAction *m_actionBlock;
	// Novell Messenger Properties, as received by the server.  
	// Unfortunately we don't know the domain of the set of keys, so they are not easily mappable to KopeteContactProperties
	QMap< QString, QVariant > m_serverProperties;
	bool m_archiving;
	// HACK: flag used to differentiate between 'all contact list instances gone while we are moving on the server' 
	// and 'all contact list instances gone because we wanted to delete them all'
	bool m_deleting;
    bool m_messageReceivedOffline;
};

#endif
