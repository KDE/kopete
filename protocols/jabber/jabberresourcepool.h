 /*
  * jabberresourcepool.h
  *
  * Copyright (c) 2004 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
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

#ifndef JABBERRESOURCEPOOL_H
#define JABBERRESOURCEPOOL_H

#include <qobject.h>
#include <QList>
#include <im.h>

class JabberResource;
class JabberAccount;

/**
 * @author Till Gerken <till@tantalo.net>
 * @author Michaël Larouche <larouche@kde.org>
 */
class JabberResourcePool : public QObject
{
	Q_OBJECT
public:
	static XMPP::Resource EmptyResource;

	typedef QList<JabberResource*> ResourceList;

	/**
	 * Default constructor
	 */
	JabberResourcePool ( JabberAccount *account );

	/**
	 * Default destructor
	 */
	~JabberResourcePool();

	/**
	 * Notify all relevant contacts in case
	 * a resource has been added, updated or removed.
	 */
	void notifyRelevantContacts ( const XMPP::Jid &jid, bool removed = false );

	/**
	 * Add a resource to the pool
	 */
	void addResource ( const XMPP::Jid &jid, const XMPP::Resource &resource );

	/**
	 * Remove a resource from the pool
	 */
	void removeResource ( const XMPP::Jid &jid, const XMPP::Resource &resource );

	/**
	 * Remove all resources for a given address from the pool
	 * NOTE: Since this method is mainly used for housekeeping,
	 *       it does NOT notify any contacts.
	 */
	void removeAllResources ( const XMPP::Jid &jid );

	/**
	 * Remove all resources from the pool
	 */
	void clear ();

	/**
	 * Lock to a certain resource
	 */
	void lockToResource ( const XMPP::Jid &jid, const XMPP::Resource &resource );

	/**
	 * Remove a resource lock
	 */
	void removeLock ( const XMPP::Jid &jid );

	/**
	 * Return the JabberResource instance for the locked resource, if any.
	 */
	 JabberResource *lockedJabberResource( const XMPP::Jid &jid );

	/**
	 * Return currently locked resource, if any
	 */
	const XMPP::Resource &lockedResource ( const XMPP::Jid &jid );

	/**
	 * Return a usable JabberResource for a given JID.
	 *
	 * @param jid Jid to look for the best resource.
	 * @param honourLock Honour the resource locked by the user.
	 * 
	 * @return a JabberResource instance.
	 */
	JabberResource *bestJabberResource( const XMPP::Jid &jid, bool honourLock = true );

	/**
	 * Return usable resource for a given JID
	 * Matches by bare() (userHost), honors locks for a JID by default
	 */
	const XMPP::Resource &bestResource ( const XMPP::Jid &jid, bool honourLock = true );

	/*
	 * Return a JabberResource for a given JID and resource name
	 * If resource name is empty or not exists, return bestJabberResource
	 */
	JabberResource *getJabberResource ( const XMPP::Jid &jid, const QString &resource );

	/**
	 * Find all resources that exist for a given JID
	 */
	void findResources ( const XMPP::Jid &jid, JabberResourcePool::ResourceList &resourceList );
	void findResources ( const XMPP::Jid &jid, XMPP::ResourceList &resourceList );
	
private slots:
	void slotResourceDestroyed ( QObject *sender );
	void slotResourceUpdated ( JabberResource *resource );
	
private:
	class Private;
	Private * const d;
};

#endif
