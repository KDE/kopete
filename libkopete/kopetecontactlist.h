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
#include <qstringlist.h>

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
	 * is not found, a new meta contact is created instead.
	 * For now, just compare the ID field.
	 *
	 * NOTE: Even though the new contact list generally doesn't require this
	 *       method, it's not completely obsolete either, because protocols
	 *       with server-side contact lists ( MSN, Jabber, etc. ) may detect
	 *       new contacts having been added upon reconnect.
	 */
	KopeteMetaContact *findContact( const QString &protocolId,
		const QString &identityId, const QString &contactId );

	/**
	 * Return all meta contacts
	 */
	QStringList contacts() const;

	/**
	 * Return all meta contacts that are reachable
	 */
	QStringList reachableContacts() const;

	/**
	 * Return all meta contacts that are online
	 */
	QStringList onlineContacts() const;

	/**
	 * Return all meta contacts with their current status
	 *
	 * FIXME: Do we *need* this one? Sounds error prone to me, because
	 * nicknames can contain parentheses too. - Martijn
	 */
	QStringList contactStatuses() const;

	/**
	 * Load the contact list
	 *
	 * FIXME: Use a better way, without exposing the XML backend, though.
	 */
	void load() { loadXML(); }
	void save() { saveXML(); }

	/**
	 * Return all available groups
	 */
	QStringList groups() const;

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


	void addGroup(QString g);
	void removeGroup(QString g);

public slots:
	void slotRemovedFromGroup( KopeteMetaContact *mc, const QString &from );

signals:
	/**
	 * A meta contact was added to the contact list. Interested classes
	 * ( like the listview widgets ) can connect to this signal to receive
	 * the newly added contacts.
	 */
	void metaContactAdded( KopeteMetaContact *mc );
	void metaContactDeleted( KopeteMetaContact *mc );

	void groupAdded( const QString &group );
	void groupRemoved( const QString &group );


	/* Not used yet.... */
	void addedToGroup( KopeteMetaContact *mc, const QString &to );
	void removedFromGroup( KopeteMetaContact *mc, const QString &from );
	
private:
	/**
	 * Return a XML representation of the contact list
	 */
	QString toXML();

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
	QStringList m_groupStringList;

	/**
	 * Our contact list instance
	 */
	static KopeteContactList *s_contactList;
	
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

