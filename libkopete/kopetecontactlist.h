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
#include <qdom.h>
#include <qstringlist.h>

class KopeteMetaContact;

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
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
	 * FIXME: Also take protocol and identity into account!
	 */
#warning Obsolete findContact
	KopeteMetaContact *findContact( const QString &contactId );

	/**
	 * add a metacontact into the contact list
	 * It always create a new one
	 */
	void addMetaContact(KopeteMetaContact *);

#warning Overload addContact to support ids
	/**
	 * Return all meta contacts
	 */
	static QStringList meta_all();

	/**
	 * Return all meta contacts that are reachable
	 */
	static QStringList meta_reachable();

	/**
	 * Return all meta contacts that are online
	 */
	static QStringList meta_online();

	/**
	 * Return all meta contacts with their current status
	 */
	static QStringList meta_status();

    /**
	 * Load the contact list from XML file [NON-FINISHED YET]
	 */
    void loadXML();

	/**
	 * Save the contact list to XML file [NON-FINISHED YET]
	 */
    void saveXML();

private:
	/**
	 * Private constructor: we are a singleton
	 */
	KopeteContactList();
	
	/**
	 * The list of contacts embodied in the meta contact
	 */
	QPtrList<KopeteMetaContact> m_contacts;

	/**
	 * Our contact list instance
	 */
	static KopeteContactList *s_contactList;
	/**
	 * The DOM representation of the contact list.
	 */
	QDomDocument *m_dom;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

