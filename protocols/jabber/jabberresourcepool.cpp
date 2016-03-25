 /*
  * jabberresourcepool.cpp
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

#include "jabberresourcepool.h"

#include <kdebug.h>

#include "jabberresource.h"
#include "jabbercontactpool.h"
#include "jabberbasecontact.h"
#include "jabberaccount.h"
#include "jabberprotocol.h"
#include "jabbercapabilitiesmanager.h"

/**
 * This resource will be returned if no other resource
 * for a given JID can be found. It's an empty offline
 * resource.
 */
XMPP::Resource JabberResourcePool::EmptyResource ( "", XMPP::Status ( "", "", 0, false ) );

class JabberResourcePool::Private
{
public:
	Private(JabberAccount *pAccount)
	 : account(pAccount)
	{}
	
	QList<JabberResource*> pool;
	QList<JabberResource*> lockList;

	/**
	 * Pointer to the JabberAccount instance.
	 */
	JabberAccount *account;
};

JabberResourcePool::JabberResourcePool ( JabberAccount *account )
	: d(new Private(account))
{}

JabberResourcePool::~JabberResourcePool ()
{
	// Delete all resources in the pool upon removal
	qDeleteAll(d->pool);
	delete d;
}

void JabberResourcePool::slotResourceDestroyed (QObject *sender)
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Resource has been destroyed, collecting the pieces.";

	JabberResource *oldResource = static_cast<JabberResource *>(sender);

	// remove this resource from the lock list if it existed
	d->lockList.removeAll ( oldResource );
}

void JabberResourcePool::slotResourceUpdated ( JabberResource *resource )
{
	QList<JabberBaseContact*> list = d->account->contactPool()->findRelevantSources ( resource->jid () );

	foreach(JabberBaseContact *mContact, list)
	{
		mContact->updateResourceList ();
	}

	// Update capabilities
	if( !resource->resource().status().caps().node().isEmpty() )
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Updating capabilities for JID: " << resource->jid().full();
		d->account->protocol()->capabilitiesManager()->updateCapabilities( d->account, resource->jid(), resource->resource().status() );
	}
}

void JabberResourcePool::notifyRelevantContacts ( const XMPP::Jid &jid, bool removed )
{
	QList<JabberBaseContact*> list = d->account->contactPool()->findRelevantSources ( jid );

	foreach(JabberBaseContact *mContact, list)
	{
		if ( removed )
			mContact->setSendsDeliveredEvent ( false );

		mContact->reevaluateStatus ();
	}
}

void JabberResourcePool::addResource ( const XMPP::Jid &jid, const XMPP::Resource &resource )
{
	// see if the resource already exists
	foreach(JabberResource *mResource, d->pool)
	{
		if ( (mResource->jid().bare().toLower() == jid.bare().toLower()) && (mResource->resource().name().toLower() == resource.name().toLower()) )
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "Updating existing resource " << resource.name() << " for " << jid.bare();

			// It exists, update it. Don't do a "lazy" update by deleting
			// it here and readding it with new parameters later on,
			// any possible lockings to this resource will get lost.
			mResource->setResource ( resource );

			// we still need to notify the contact in case the status
			// of this resource changed
			notifyRelevantContacts ( jid );

			return;
		}
	}

	kDebug(JABBER_DEBUG_GLOBAL) << "Adding new resource " << resource.name() << " for " << jid.bare();

	// Update initial capabilities if available.
	// Called before creating JabberResource so JabberResource wouldn't ask for disco information. 
	if( !resource.status().caps().node().isEmpty() )
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Initial update of capabilities for JID: " << jid.full();
		d->account->protocol()->capabilitiesManager()->updateCapabilities( d->account, jid, resource.status() );
	}

	// create new resource instance and add it to the dictionary
	JabberResource *newResource = new JabberResource(d->account, jid, resource);
	connect ( newResource, SIGNAL (destroyed(QObject*)), this, SLOT (slotResourceDestroyed(QObject*)) );
	connect ( newResource, SIGNAL (updated(JabberResource*)), this, SLOT (slotResourceUpdated(JabberResource*)) );
	d->pool.append ( newResource );

	// send notifications out to the relevant contacts that
	// a new resource is available for them
	notifyRelevantContacts ( jid );
}

void JabberResourcePool::removeResource ( const XMPP::Jid &jid, const XMPP::Resource &resource )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Removing resource " << resource.name() << " from " << jid.bare();

	foreach(JabberResource *mResource, d->pool)
	{
		if ( (mResource->jid().bare().toLower() == jid.bare().toLower()) && (mResource->resource().name().toLower() == resource.name().toLower()) )
		{
			JabberResource *deletedResource = d->pool.takeAt( d->pool.indexOf(mResource) );
			delete deletedResource;

			notifyRelevantContacts ( jid, true );
			return;
		}
	}

	kDebug(JABBER_DEBUG_GLOBAL) << "WARNING: No match found!";
}

void JabberResourcePool::removeAllResources ( const XMPP::Jid &jid )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Removing all resources for " << jid.bare();

	foreach(JabberResource *mResource, d->pool)
	{
		if ( mResource->jid().bare().toLower() == jid.bare().toLower() )
		{
			// only remove preselected resource in case there is one
			if ( jid.resource().isEmpty () || ( jid.resource().toLower () == mResource->resource().name().toLower () ) )
			{
				kDebug(JABBER_DEBUG_GLOBAL) << "Removing resource " << jid.bare() << "/" << mResource->resource().name ();
				JabberResource *deletedResource = d->pool.takeAt( d->pool.indexOf(mResource) );
				delete deletedResource;
			}
		}
	}
}

void JabberResourcePool::clear ()
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Clearing the resource pool.";

	/*
	 * Since many contacts can have multiple resources, we can't simply delete
	 * each resource and trigger a notification upon each deletion. This would
	 * cause lots of status updates in the GUI and create unnecessary flicker
	 * and API traffic. Instead, collect all JIDs, clear the dictionary
	 * and then notify all JIDs after the resources have been deleted.
	 */

	QStringList jidList;

	foreach(JabberResource *mResource, d->pool)
	{
		jidList += mResource->jid().full ();
	}

	/*
	 * The lock list will be cleaned automatically.
	 */
	qDeleteAll(d->pool);
	d->pool.clear ();

	/*
	 * Now go through the list of JIDs and notify each contact
	 * of its status change
	 */
	for ( QStringList::Iterator it = jidList.begin (); it != jidList.end (); ++it )
	{
		notifyRelevantContacts ( XMPP::Jid ( *it ), true );
	}

}

void JabberResourcePool::lockToResource ( const XMPP::Jid &jid, const XMPP::Resource &resource )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Locking " << jid.bare() << " to " << resource.name();

	// remove all existing locks first
	removeLock ( jid );

	// find the resource in our dictionary that matches
	foreach(JabberResource *mResource, d->pool)
	{
		if ( (mResource->jid().bare().toLower() == jid.bare().toLower()) && (mResource->resource().name().toLower() == resource.name().toLower()) )
		{
			d->lockList.append ( mResource );
			return;
		}
	}

	kDebug(JABBER_DEBUG_GLOBAL) << "WARNING: No match found!";
}

void JabberResourcePool::removeLock ( const XMPP::Jid &jid )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Removing resource lock for " << jid.bare();

	// find the resource in our dictionary that matches
	foreach(JabberResource *mResource, d->pool)
	{
		if ( (mResource->jid().bare().toLower() == jid.bare().toLower()) )
		{
			d->lockList.removeAll (mResource);
		}
	}

	kDebug(JABBER_DEBUG_GLOBAL) << "No locks found.";
}

JabberResource *JabberResourcePool::lockedJabberResource( const XMPP::Jid &jid )
{
	// check if the JID already carries a resource, then we will have to use that one
	if ( !jid.resource().isEmpty () )
	{
		// we are subscribed to a JID, find the according resource in the pool
		foreach(JabberResource *mResource, d->pool)
		{
			if ( ( mResource->jid().bare().toLower () == jid.bare().toLower () ) && ( mResource->resource().name () == jid.resource () ) )
			{
				return mResource;
			}
		}

		kDebug ( JABBER_DEBUG_GLOBAL ) << "WARNING: No resource found in pool, returning as offline.";

		return 0L;
	}

	// see if we have a locked resource
	foreach(JabberResource *mResource, d->lockList)
	{
		if ( mResource->jid().bare().toLower() == jid.bare().toLower() )
		{
			kDebug (JABBER_DEBUG_GLOBAL) << "Current lock for " << jid.bare() << " is '" << mResource->resource().name () << "'";
			return mResource;
		}
	}

	kDebug (JABBER_DEBUG_GLOBAL) << "No lock available for " << jid.bare();

	// there's no locked resource, return an empty resource
	return 0L;
}

const XMPP::Resource &JabberResourcePool::lockedResource ( const XMPP::Jid &jid )
{
	JabberResource *resource = lockedJabberResource( jid );
	return (resource) ? resource->resource() : EmptyResource;
}

JabberResource *JabberResourcePool::bestJabberResource( const XMPP::Jid &jid, bool honourLock )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Determining best resource for " << jid.full ();

	if ( honourLock )
	{
		// if we are locked to a certain resource, always return that one
		JabberResource *mResource = lockedJabberResource ( jid );
		if ( mResource )
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "We have a locked resource '" << mResource->resource().name () << "' for " << jid.full ();
			return mResource;
		}
	}

	JabberResource *bestResource = 0L;
	JabberResource *currentResource = 0L;

	foreach(currentResource, d->pool)
	{
		// make sure we are only looking up resources for the specified JID
		if ( currentResource->jid().bare().toLower() != jid.bare().toLower() )
		{
			continue;
		}

		// take first resource if no resource has been chosen yet
		if(!bestResource)
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "Taking '" << currentResource->resource().name () << "' as first available resource.";

			bestResource = currentResource;
			continue;
		}

		if(currentResource->resource().priority() > bestResource->resource().priority())
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "Using '" << currentResource->resource().name () << "' due to better priority.";

			// got a better match by priority
			bestResource = currentResource;
		}
		else
		{
			if(currentResource->resource().priority() == bestResource->resource().priority())
			{
				if(currentResource->resource().status().timeStamp() > bestResource->resource().status().timeStamp())
				{
					kDebug(JABBER_DEBUG_GLOBAL) << "Using '" << currentResource->resource().name () << "' due to better timestamp.";

					// got a better match by timestamp (priorities are equal)
					bestResource = currentResource;
				}
			}
		}
	}

	return (bestResource) ? bestResource : 0L;
}

const XMPP::Resource &JabberResourcePool::bestResource ( const XMPP::Jid &jid, bool honourLock )
{
	JabberResource *bestResource = bestJabberResource( jid, honourLock);
	return (bestResource) ? bestResource->resource() : EmptyResource;
}

//TODO: Find Resources based on certain Features.
void JabberResourcePool::findResources ( const XMPP::Jid &jid, JabberResourcePool::ResourceList &resourceList )
{
	foreach(JabberResource *mResource, d->pool)
	{
		if ( mResource->jid().bare().toLower() == jid.bare().toLower() )
		{
			// we found a resource for the JID, let's see if the JID already contains a resource
			if ( !jid.resource().isEmpty() && ( jid.resource().toLower() != mResource->resource().name().toLower() ) )
				// the JID contains a resource but it's not the one we have in the dictionary,
				// thus we have to ignore this resource
				continue;

			resourceList.append ( mResource );
		}
	}
}

void JabberResourcePool::findResources ( const XMPP::Jid &jid, XMPP::ResourceList &resourceList )
{
	foreach(JabberResource *mResource, d->pool)
	{
		if ( mResource->jid().bare().toLower() == jid.bare().toLower() )
		{
			// we found a resource for the JID, let's see if the JID already contains a resource
			if ( !jid.resource().isEmpty() && ( jid.resource().toLower() != mResource->resource().name().toLower() ) )
				// the JID contains a resource but it's not the one we have in the dictionary,
				// thus we have to ignore this resource
				continue;

			resourceList.append ( mResource->resource () );
		}
	}
}

JabberResource *JabberResourcePool::getJabberResource ( const XMPP::Jid &jid, const QString &resource )
{
	if ( resource.isEmpty() )
		return bestJabberResource(jid);

	foreach(JabberResource *mResource, d->pool)
	{
		if ( mResource->jid().bare().toLower() == jid.bare().toLower() && jid.resource().toLower() == resource )
		{
			// we found a resource for the JID, let's see if the JID already contains a resource
			if ( !jid.resource().isEmpty() && ( jid.resource().toLower() != mResource->resource().name().toLower() ) )
				// the JID contains a resource but it's not the one we have in the dictionary,
				// thus we have to ignore this resource
				continue;

			return mResource;
		}
	}

	return bestJabberResource(jid);
}

#include "jabberresourcepool.moc"
