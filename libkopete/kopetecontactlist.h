/*
    kopetecontactlist.h - Kopete's Contact List backend

    Copyright (c) 2002      by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef __KOPETECONTACTLIST_H__
#define __KOPETECONTACTLIST_H__

#include <qobject.h>
#include <qptrlist.h>

//#include <kurl.h>
class KURL;

class QDomDocument;

namespace Kopete
{

class MetaContact;
class Group;
class Contact;

/**
 * @author Martijn Klingens <klingens@kde.org>
 */
class ContactList : public QObject
{
	Q_OBJECT

public:
	/**
	 * The contact list is a singleton object. Use this method to retrieve
	 * the instance.
	 */
	static ContactList *self();

	~ContactList();

	/**
	 * Find the meta contact that a given contact belongs to. If contact
	 * is not found, null is returned.
	 * For now, just compare the ID field.
	 *
	 * NOTE: Even though the new contact list generally doesn't require this
	 *       method, it's not completely obsolete either, because protocols
	 *       with server-side contact lists ( MSN, Jabber, etc. ) may detect
	 *       new contacts having been added upon reconnect.
	 *
	 * @todo return a Kopete::Contact.  the metacontact can be obtained with contact->metaContact
	 */
	MetaContact *findContact( const QString &protocolId,
		const QString &accountId, const QString &contactId );


	/**
	 * Return the metacontact referenced by the given id
	 */
	MetaContact *metaContact( const QString &metaContactId ) const;

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
	QPtrList<Contact> onlineContacts() const;

	/**
	 * Overloaded method of @ref onlineContacts() that only returns
	 * the online contacts for a single protocol
	 */
	QPtrList<Contact> onlineContacts( const QString &protocolId ) const;

	/**
	 * Return all meta contacts that are online
	 */
	QPtrList<MetaContact> onlineMetaContacts() const;

	/**
	 * Overloaded method of @ref onlineMetaContacts() that only returns
	 * the online meta contacts for a single protocol
	 */
	QPtrList<MetaContact> onlineMetaContacts( const QString &protocolId ) const;

	/**
	 * Returns all contacts which can accept file transfers
	 */
	QStringList fileTransferContacts() const;

	QStringList contactFileProtocols( const QString &displayName);

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
	QPtrList<Group> groups() const;

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
	QPtrList<MetaContact> metaContacts() const;

	/**
	 * Get a group.
	 * If a group already exists with the given name and the given type, the existing group will be returned.
 	 * Otherwise, a new group will be created.
	 * @param displayName is the display name to search
	 * @param type is the Kopete::Group::GroupType to search, the default value is Kopetegroup::Normal
	 */
	Group * getGroup( const QString &displayName, int type = 0/*Kopete::Group::Normal*/ );

	/**
	 * return the group with the given unique id. if none is found return 0L
	 */
	Group * getGroup(unsigned int groupId);

	/**
	 * Find a contact by display name. Returns the first match.
	 */
	MetaContact *findContactByDisplayName( const QString &displayName );

	/**
	 * Find a meta contact by its contact id. Returns the first match.
	 */
	MetaContact *findMetaContactByContactId( const QString &contactId );

	/**
	 * return the list of metacontact actually selected in the contactlist UI
	 */
	QPtrList<MetaContact> selectedMetaContacts() const;

	/**
	 * return the list of groups actualy selected in the contactlist UI
	 */
	QPtrList<Group> selectedGroups() const ;
	
public slots:

	/**
	 * Add the metacontact into the contact list
	 * When calling this method, the contact has to be already placed in the correct group.
	 * If the contact is not in a  group, it will be added to the top-level group
	 */
	void addMetaContact( Kopete::MetaContact *c );

	/**
	 * Remove a metacontact from the contactlist.
	 * This method delete itself the metacontact.
	 */
	void removeMetaContact( Kopete::MetaContact *contact );

	/**
	 * Add a group
	 */
	void addGroup(Kopete::Group *);

	/**
	 * Remove a group
	 * this method delete the group
	 */
	void removeGroup(Kopete::Group *);

	/**
	 * Exposed via DCOP in kopeteiface
	 * Used to send a file to a MetaContact using the highest ranked protocol
	 *
	 * FIXME: We need to change this to use a unique ID instead of the displayName
	 *
	 * @param displayName Metacontact to send file to
	 * @param sourceURL The file we are sending
	 * @param altFileName (Optional) An alternate filename for the file we are sending
	 * @param fileSize (Optional) The size of the file
	 */
	void sendFile(const QString &displayName, const KURL &sourceURL,
		const QString &altFileName = QString::null, const long unsigned int fileSize = 0L);

	/**
	 * Open a chat to a contact, and optionally set some initial text
	 */
	void messageContact( const QString &displayName, const QString &messageText = QString::null );

	/**
	 * Set which items are selected in the ContactList GUI.
	 * This method has to be called by the contactlist UI side.
	 * it stores the selected items, and emits signals
	 */
	 void setSelectedItems(QPtrList<MetaContact> metaContacts , QPtrList<Group> groups);

	/**
	 * @internal
	 * Load the contact list
	 *
	 * FIXME: Use a better way, without exposing the XML backend, though.
	 */
	void load();

	void save();

signals:
	/**
	 * A meta contact was added to the contact list. Interested classes
	 * ( like the listview widgets ) can connect to this signal to receive
	 * the newly added contacts.
	 */
	void metaContactAdded( Kopete::MetaContact *mc );
	/**
	 * a metacontact has just been removed
	 */
	void metaContactDeleted( Kopete::MetaContact *mc );

	/**
	 * A group has just been added
	 */
	void groupAdded( Kopete::Group * );
	/**
	 * A group has just been removed
	 */
	void groupRemoved( Kopete::Group * );
	/**
	 * A group has just been renamed
	 */
	void groupRenamed(Kopete::Group *, const QString & oldname);

	/**
	 * A contact has been added to a group
	 */
	void metaContactAddedToGroup( Kopete::MetaContact *mc, Kopete::Group *to );
	/**
	 * A contact has been removed from a group
	 */
	void metaContactRemovedFromGroup( Kopete::MetaContact *mc, Kopete::Group *from );

	/**
	 * This signal is emit when the selection has changed, it is emitted after the following slot
	 * Warning: Do not delete any contacts in slots connected to this signal.  (it is the warning in the QListView::selectionChanged() doc)
	 */
	void selectionChanged();
	/**
	 * This signal is emitted each time the selection has changed. the bool is set to true if only one meta contact has been selected,
	 * and set to false if none, or several contacts are selected
	 * you can connect this signal to KAction::setEnabled if you have an action which is applied to only one contact
	 */
	void metaContactSelected(bool);

private slots:
	/**
	 * Called when the contact list changes. Flags the list dirty and schedules a save for a little while later.
	 */
	void slotSaveLater();
	/**
	 * Called on contactlist load or when KABC has changed, to check if we need to update our contactlist from there.
	 */
	void slotKABCChanged();
private:
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
	ContactList();

	/**
	 * Our contact list instance
	 */
	static ContactList *s_contactList;

	/**
	 * Convert the contact list from an older version
	 */
	void convertContactList( const QString &fileName, uint fromVersion, uint toVersion );

	class KopeteContactListPrivate;
	KopeteContactListPrivate *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

