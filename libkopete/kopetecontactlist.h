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
	KopeteMetaContact *findContact( const QString &contactId );

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
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

