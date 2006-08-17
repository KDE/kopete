/*
    kopetecontactlist.h - Kopete's Contact List backend

    Copyright (c) 2002      by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @ kde.org>

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

#ifndef KOPETECONTACTLIST_H__
#define KOPETECONTACTLIST_H__

#include <qobject.h>
#include <qptrlist.h>

#include "kopete_export.h"

class KURL;
class QDomDocument;


namespace Kopete 
{

class MetaContact;
class Group;
class Contact;


/**
 * @brief manage contacts and metacontact
 *
 * The contactList is a singleton you can uses with @ref ContactList::self()
 *
 * it let you get a list of metacontact with metaContacts()
 * Only metacontact which are on the contactlist are returned.
 *
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 */
class KOPETE_EXPORT ContactList : public QObject
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
	 * @brief return a list of all metacontact of the contactlist
	 * Retrieve the list of all available meta contacts.
	 * The returned QPtrList is not the internally used variable, so changes
	 * to it won't propagate into the actual contact list. This can be
	 * useful if you need a subset of the contact list, because you can
	 * simply filter the result set as you wish without worrying about
	 * side effects.
	 * The contained MetaContacts are obviously _not_ duplicates, so
	 * changing those *will* have the expected result :-)
	 */
	QPtrList<MetaContact> metaContacts() const;
	
	/**
	 * @return all groups
	 */
	QPtrList<Group> groups() const;

	/**
	 * Return the metacontact referenced by the given id. is none is found, return 0L
	 * @sa MetaContact::metaContactId()
	 */
	MetaContact *metaContact( const QString &metaContactId ) const;

	/**
	 * return the group with the given unique id. if none is found return 0L
	 */
	Group * group(unsigned int groupId) const;
	
	
	/**
	 * @brief find a contact in the contactlist.
	 * Browse in each metacontact of the list to find the contact with the given ID.
	 * @param protocolId the @ref Plugin::pluginId() of the protocol ("MSNProtocol")
	 * @param accountId the @ref Account::accountId()
	 * @param contactId the @ref Contact::contactId()
	 * @return the contact with the parameters, or 0L if not found.
	 */
	Contact *findContact( const QString &protocolId, const QString &accountId, const QString &contactId ) const;

	/**
	 * Find a contact by display name. Returns the first match.
	 */
	MetaContact *findMetaContactByDisplayName( const QString &displayName ) const;

	/**
	 * Find a meta contact by its contact id. Returns the first match.
	 */
	MetaContact *findMetaContactByContactId( const QString &contactId ) const;

	/**
	 * @brief find a group with his displayName
	 * If a group already exists with the given name and the given type, the existing group will be returned.
 	 * Otherwise, a new group will be created.
	 * @param displayName is the display name to search
	 * @param type is the Group::GroupType to search, the default value is group::Normal
	 * @return always a valid Group
	 */
	Group * findGroup( const QString &displayName, int type = 0/*Group::Normal*/ );

	/**
	 * return the list of metacontact actually selected in the contactlist UI
	 */
	QPtrList<MetaContact> selectedMetaContacts() const;

	/**
	 * return the list of groups actualy selected in the contactlist UI
	 */
	QPtrList<Group> selectedGroups() const ;

	/**
	  * return the metacontact that represent the user itself.
	  * This metacontact should be the parent of every Kopete::Account::myself() contacts.
	  *
	  * This metacontact is not in the contactlist.
	  */
	MetaContact* myself();

	
public slots:

	/**
	 * Add the metacontact into the contact list
	 * When calling this method, the contact has to be already placed in the correct group.
	 * If the contact is not in a  group, it will be added to the top-level group.
	 * It is also better if the MetaContact could also be completely created, i.e: all contacts already in it
	 */
	void addMetaContact( Kopete::MetaContact *c );

	/**
	 * Remove a metacontact from the contactlist.
	 * This method delete itself the metacontact.
	 */
	void removeMetaContact( Kopete::MetaContact *contact );

	/**
	 * Add a group
	 * each group must be added on the list after his creation.
	 */
	void addGroup(Kopete::Group *);

	/**
	 * Remove a group
	 * this method delete the group
	 */
	void removeGroup(Kopete::Group *);

	/**
	 * Set which items are selected in the ContactList GUI.
	 * This method has to be called by the contactlist UI side.
	 * it stores the selected items, and emits signals
	 */
	 void setSelectedItems(QPtrList<MetaContact> metaContacts , QPtrList<Group> groups);

	/**
	  * Apply the global identity.
	  */
	void loadGlobalIdentity();

signals:
	/**
	 * A meta contact was added to the contact list. Interested classes
	 * ( like the listview widgets ) can connect to this signal to receive
	 * the newly added contacts.
	 */
	void metaContactAdded( Kopete::MetaContact *mc );
	
	/**
	 * A metacontact has just been removed.  and will be soon deleted
	 */
	void metaContactRemoved( Kopete::MetaContact *mc );

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

	/**
	 * This signal is emitted each time a global identity field change.
	 * HOWTO use:
	 *
	 * - Connect signal globalIdentityChanged(const QString &key, const QVariant 
	 *    &value) to a slot in your derivate Account class (the best 
	 *    place to put it).
	 * - In the slot:
	 *    - Check the key you want to be sync with global identity.
	 *    - Update the myself contact and/or update on server.
	 * 
	 * For now, when photo is changed, it always send the photo file path.
	 *
	 * Connect signal in your Account constructor:
	 * @code
	 * connect(Kopete::ContactList::self(), SIGNAL(globalIdentityChanged(const QString&, const QVariant&)), SLOT(slotglobalIdentityChanged(const QString&, const QVariant&)));
	 * @endcode
	 *
	 * Example of a typical implemented slot:
	 * @code
	 * void slotGlobalIdentityChanged(const QString &key, const QVariant &value)
	 * {
	 *	if(key == Kopete::Global::Properties::self()->nickName().key())
	 *	{
	 * 	    myself()->setProperty(protocol()->propNickname, value.toString());
	 * 	    this->slotUpdateUserInfo();
	 *	}
	 *	else if(key == Kopete::Global::Properties::self()->photo().key())
	 *	{
	 * 	    myself()->setProperty(protocol()->propPhotoUrl, value.toString());
	 * 	   this->slotUpdateDisplayPicture();
	 *	}
	 * }
	 * @endcode
	 */
	void globalIdentityChanged( const QString &key, const QVariant &value );

private slots:
	/**
	 * Called when the contact list changes. Flags the list dirty and schedules a save for a little while later.
	 */
	void slotSaveLater();
	/**
	 * Called on contactlist load or when KABC has changed, to check if we need to update our contactlist from there.
	 */
	void slotKABCChanged();
	
	/**
	 * Called when the myself displayName changed.
	 */
	void slotDisplayNameChanged();

	/**
	 * Called when the myself photo changed.
	 */
	void slotPhotoChanged();

private:
	
	/**
	 * Convert the contact list from an older version
	 */
	void convertContactList( const QString &fileName, uint fromVersion, uint toVersion );
	
	
	/**
	 * Private constructor: we are a singleton
	 */
	ContactList();
	
	static ContactList *s_self;
	class Private;
	Private *d;
	
public: //TODO I think all theses method should be moved to the decop interface.
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

public slots:
	/**
	 * @internal
	 * Load the contact list
	 *
	 * FIXME: Use a better way, without exposing the XML backend, though.
	 */
	void load();

	void save();
	
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
};

} //END namespace Kopete


#endif


