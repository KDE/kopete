 /*
  * jabbercontactpool.h
  *
  * Copyright (c) 2004 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#ifndef JABBERCONTACTPOOL_H
#define JABBERCONTACTPOOL_H

#include <qobject.h>
#include <im.h>
#include <jabbercontact.h>

class KopeteMetaContact;
class KopeteContact;
class JabberContactPoolItem;

/**
 * @author Till Gerken <till@tantalo.net>
 */
class JabberContactPool : public QObject
{

Q_OBJECT

public:
	/**
	 * Default constructor
	 */
	JabberContactPool ( JabberAccount *account );

	/**
	 * Default destructor
	 */
	~JabberContactPool();

	/**
	 * Add a contact to the pool
	 */
	JabberContact *addContact ( const XMPP::RosterItem &contact, KopeteMetaContact *metaContact, bool dirty = true );

	/**
	 * Remove a contact from the pool
	 */
	void removeContact ( const XMPP::Jid &jid );

	/**
	 * Remove all contacts from the pool
	 */
	void clear ();

	/**
	 * Sets the "dirty" flag for a certain contact
	 */
	void setDirty ( const XMPP::Jid &jid, bool dirty );

	/**
	 * Remove all dirty elements from the pool
	 * (used after connecting to delete removed items from the roster)
	 */
	void cleanUp ();

	/**
	 * Find an exact match in the pool by full JID.
	 */
	JabberContact *findExactMatch ( const XMPP::Jid &jid );

	/**
	 * Find a relevant recipient for a given JID.
	 * This will match user@domain for a given user@domain/resource,
	 * but NOT user@domain/resource for a given user@domain.
	 */
	JabberContact *findRelevantRecipient ( const XMPP::Jid &jid );

	/**
	 * Find relevant sources for a given JID.
	 * This will match user@domain/resource for a given user@domain.
	 */
	QPtrList<JabberContact> findRelevantSources ( const XMPP::Jid &jid );

private slots:
	void slotContactDestroyed ( KopeteContact *contact );

private:
	QPtrList<JabberContactPoolItem> mPool;
	JabberAccount *mAccount;

};

class JabberContactPoolItem : QObject
{
Q_OBJECT
public:
	JabberContactPoolItem ( JabberContact *contact );
	~JabberContactPoolItem ();

	void setDirty ( bool dirty );
	bool dirty ();
	JabberContact *contact ();

private:
	bool mDirty;
	JabberContact *mContact;
};

#endif
