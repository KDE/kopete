/*
    kopetecontactlist.h - Kopete's Contact List backend

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __kopetecontactlist_h__
#define __kopetecontactlist_h__

#include <qobject.h>
#include <qptrlist.h>

#include <kurl.h>

#include "kopetecontact.h"
#include "kopetegroup.h"

class QDomDocument;

class KopeteMetaContact;

/**
 * @author Martijn Klingens <klingens@kde.org>
 */
class KopeteContactList : public QObject
{
	Q_OBJECT

public:
	/**
	 * The contact list is a singleton object. Use this method to retrieve
	 * the instance.
	 */
	static KopeteContactList *contactList();

	~KopeteContactList();

	/**
	 * Find the meta contact that belongs to a given contact. If contact
	 * is not found, null is returned.
	 * For now, just compare the ID field.
	 *
	 * NOTE: Even though the new contact list generally doesn't require this
	 *       method, it's not completely obsolete either, because protocols
	 *       with server-side contact lists ( MSN, Jabber, etc. ) may detect
	 *       new contacts having been added upon reconnect.
	 */
	KopeteMetaContact *findContact( const QString &protocolId,
		const QString &accountId, const QString &contactId );

	/**
	 * Return all meta contacts
	 */
	QStringList contacts() const;

	/**
	 * Return all meta contacts that are reachable
	 */
	QStringList reachableContacts() const;

	/**
	 * Return all contacts that are online
	 */
	QPtrList<KopeteContact> onlineContacts() const;

	/**
	 * Overloaded method of @ref onlineContacts() that only returns
	 * the online contacts for a single protocol
	 */
	QPtrList<KopeteContact> onlineContacts( const QString &protocolId ) const;

	/**
	 * Return all meta contacts that are online
	 */
	QPtrList<KopeteMetaContact> onlineMetaContacts() const;

	/**
	 * Overloaded method of @ref onlineMetaContacts() that only returns
	 * the online meta contacts for a single protocol
	 */
	QPtrList<KopeteMetaContact> onlineMetaContacts( const QString &protocolId ) const;

	/**
	 * Returns all contacts which can accept file transfers
	 */
	QStringList fileTransferContacts() const;

	QStringList contactFileProtocols( const QString &displayName);

	/**
	 * Exposed via DCOP in kopeteiface
	 * Used to send a file to a MetaContact using the highest ranked protocol
	 *
	 * FIXME: We need to change this to us ea unique ID instea dof displayName
	 *
	 * @param displayName Metacontact to send file to
	 * @param sourceURL The file we are sending
	 * @param altFileName (Optional) An alternate filename for the file we are sending
	 * @param fileSize (Optional) The size of the file
	 */
	void sendFile(const QString &displayName, const KURL &sourceURL,
		const QString &altFileName = QString::null, const long unsigned int fileSize = 0L);
	/**
	 * Return all meta contacts with their current status
	 *
	 * FIXME: Do we *need* this one? Sounds error prone to me, because
	 * nicknames can contain parentheses too. - Martijn
	 */
	QStringList contactStatuses() const;

	/**
	 * Return all available groups
	 */
	KopeteGroupList groups() const;

	/**
	 * add a metacontact into the contact list
	 * It handles the groups for it.
	 */
	void addMetaContact( KopeteMetaContact *c );

	void removeMetaContact( KopeteMetaContact *contact );

	/**
	 * Retrieve the list of all available meta contacts.
	 * The returned QPtrList is not the internally used variable, so changes
	 * to it won't propagate into the actual contact list. This can be
	 * useful if you need a subset of the contact list, because you can
	 * simply filter the result set as you wish without worrying about
	 * side effects.
	 * The contained KopeteMetaContacts are obviously _not_ duplicates, so
	 * changing those *will* have the expected result :-)
	 */
	QPtrList<KopeteMetaContact> metaContacts() const;

	/**
	 * Add a group
	 */
	void addGroup(KopeteGroup *);

	/**
	 * Remove a group
	 */
	void removeGroup(KopeteGroup *);

	/**
	 * Get a group
	 */
	KopeteGroup * getGroup(const QString& displayName, KopeteGroup::GroupType type=KopeteGroup::Classic);

	/**
	 * Find a contact by display name. Returns the first match.
	 */
	KopeteMetaContact *findContactByDisplayName( const QString &displayName );

	/**
	 * Open a chat to a contact, and optionally set some initial text
	 */
	void messageContact( const QString &displayName, const QString &messageText = QString::null );

public slots:
//	void slotRemovedFromGroup( KopeteMetaContact *mc, const QString &from );

	/**
	 * Load the contact list
	 *
	 * FIXME: Use a better way, without exposing the XML backend, though.
	 */
	void load() { loadXML(); }
	void save() { saveXML(); }

signals:
	/**
	 * A meta contact was added to the contact list. Interested classes
	 * ( like the listview widgets ) can connect to this signal to receive
	 * the newly added contacts.
	 */
	void metaContactAdded( KopeteMetaContact *mc );
	void metaContactDeleted( KopeteMetaContact *mc );

	void groupAdded( KopeteGroup * );
	void groupRemoved( KopeteGroup * );
	void groupRenamed(KopeteGroup *, const QString & oldname);

	/* Not used yet.... */
//	void addedToGroup( KopeteMetaContact *mc, const QString &to );
//	void removedFromGroup( KopeteMetaContact *mc, const QString &from );

private:
	/**
	 * Current contact list version * 10 ( i.e. '10' is version '1.0' )
	 */
	static const uint ContactListVersion = 10;

	/**
	 * Return a XML representation of the contact list
	 */
	const QDomDocument toXML();

	/**
	 * Load the contact list from XML file
	 */
	void loadXML();

	/**
	 * Save the contact list to XML file
	 */
	void saveXML();

	/**
	 * Private constructor: we are a singleton
	 */
	KopeteContactList();

	/**
	 * The list of meta contacts that are available
	 */
	QPtrList<KopeteMetaContact> m_contacts;

	/**
	 * List of groups
	 */
	 KopeteGroupList m_groupList;

	/**
	 * Our contact list instance
	 */
	static KopeteContactList *s_contactList;

	/**
	 * Convert the contact list from an older version
	 */
	void convertContactList( const QString &fileName, uint fromVersion, uint toVersion );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

