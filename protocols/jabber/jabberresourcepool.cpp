 /*
  * jabberresourcepool.cpp
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

#include <qptrlist.h>
#include <kdebug.h>
#include "jabberresourcepool.h"
#include "jabberresource.h"
#include "jabbercontactpool.h"
#include "jabberbasecontact.h"
#include "jabberaccount.h"

/**
 * This resource will be returned if no other resource
 * for a given JID can be found. It's an empty offline
 * resource.
 */
XMPP::Resource JabberResourcePool::EmptyResource ( "", XMPP::Status ( "", "", 0, false ) );

JabberResourcePool::JabberResourcePool ( JabberAccount *account )
{

	// automatically delete all resources in the pool upon removal
	mPool.setAutoDelete (true);

	mAccount = account;

}

JabberResourcePool::~JabberResourcePool ()
{
}

void JabberResourcePool::slotResourceDestroyed (QObject *sender)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Resource has been destroyed, collecting the pieces." << endl;

	JabberResource *oldResource = static_cast<JabberResource *>(sender);

	// remove this resource from the lock list if it existed
	mLockList.remove ( oldResource );

}

void JabberResourcePool::slotResourceUpdated ( JabberResource *resource )
{

	QPtrList<JabberBaseContact> list = mAccount->contactPool()->findRelevantSources ( resource->jid () );

	for(JabberBaseContact *mContact = list.first (); mContact; mContact = list.next ())
	{
		mContact->updateResourceList ();
	}

}

void JabberResourcePool::notifyRelevantContacts ( const XMPP::Jid &jid )
{

	QPtrList<JabberBaseContact> list = mAccount->contactPool()->findRelevantSources ( jid );

	for(JabberBaseContact *mContact = list.first (); mContact; mContact = list.next ())
	{
		mContact->reevaluateStatus ();
	}

}

void JabberResourcePool::addResource ( const XMPP::Jid &jid, const XMPP::Resource &resource )
{

	// see if the resource already exists
	for(JabberResource *mResource = mPool.first (); mResource; mResource = mPool.next ())
	{
		if ( (mResource->jid().userHost().lower() == jid.userHost().lower()) && (mResource->resource().name().lower() == resource.name().lower()) )
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Updating existing resource " << resource.name() << " for " << jid.userHost() << endl;

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

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Adding new resource " << resource.name() << " for " << jid.userHost() << endl;

	// create new resource instance and add it to the dictionary
	JabberResource *newResource = new JabberResource(mAccount, jid, resource);
	connect ( newResource, SIGNAL ( destroyed (QObject *) ), this, SLOT ( slotResourceDestroyed (QObject *) ) );
	connect ( newResource, SIGNAL ( updated (JabberResource *) ), this, SLOT ( slotResourceUpdated (JabberResource *) ) );
	mPool.append ( newResource );

	// send notifications out to the relevant contacts that
	// a new resource is available for them
	notifyRelevantContacts ( jid );

}

void JabberResourcePool::removeResource ( const XMPP::Jid &jid, const XMPP::Resource &resource )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing resource " << resource.name() << " from " << jid.userHost() << endl;

	for(JabberResource *mResource = mPool.first (); mResource; mResource = mPool.next ())
	{
		if ( (mResource->jid().userHost().lower() == jid.userHost().lower()) && (mResource->resource().name().lower() == resource.name().lower()) )
		{
			mPool.remove ();
			notifyRelevantContacts ( jid );
			return;
		}
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "WARNING: No match found!" << endl;

}

void JabberResourcePool::removeAllResources ( const XMPP::Jid &jid )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing all resources for " << jid.userHost() << endl;

	for(JabberResource *mResource = mPool.first (); mResource; mResource = mPool.next ())
	{
		if ( mResource->jid().userHost().lower() == jid.userHost().lower() )
		{
			// only remove preselected resource in case there is one
			if ( !jid.resource().isEmpty () && ( jid.resource () == mResource->resource().name () ) )
				mPool.remove ();
		}
	}

}

void JabberResourcePool::clear ()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Clearing the resource pool." << endl;

	/*
	 * Since many contacts can have multiple resources, we can't simply delete
	 * each resource and trigger a notification upon each deletion. This would
	 * cause lots of status updates in the GUI and create unnecessary flicker
	 * and API traffic. Instead, collect all JIDs, clear the dictionary
	 * and then notify all JIDs after the resources have been deleted.
	 */

	QStringList jidList;

	for ( JabberResource *mResource = mPool.first (); mResource; mResource = mPool.next () )
	{
		jidList += mResource->jid().full ();
	}

	/*
	 * Since mPool has autodeletion enabled, this will cause all
	 * items to be deleted. The lock list will be cleaned automatically.
	 */
	mPool.clear ();

	/*
	 * Now go through the list of JIDs and notify each contact
	 * of its status change
	 */
	for ( QStringList::Iterator it = jidList.begin (); it != jidList.end (); ++it )
	{
		notifyRelevantContacts ( XMPP::Jid ( *it ) );
	}

}

void JabberResourcePool::lockToResource ( const XMPP::Jid &jid, const XMPP::Resource &resource )
{

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Locking " << jid.full() << " to " << resource.name() << endl;

	// remove all existing locks first
	removeLock ( jid );

	// find the resource in our dictionary that matches
	for(JabberResource *mResource = mPool.first (); mResource; mResource = mPool.next ())
	{
		if ( (mResource->jid().userHost().lower() == jid.full().lower()) && (mResource->resource().name().lower() == resource.name().lower()) )
		{
			mLockList.append ( mResource );
			return;
		}
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "WARNING: No match found!" << endl;

}

void JabberResourcePool::removeLock ( const XMPP::Jid &jid )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing resource lock for " << jid.userHost() << endl;

	// find the resource in our dictionary that matches
	for(JabberResource *mResource = mPool.first (); mResource; mResource = mPool.next ())
	{
		if ( (mResource->jid().userHost().lower() == jid.userHost().lower()) )
		{
			mLockList.remove (mResource);
		}
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "No locks found." << endl;

}

const XMPP::Resource &JabberResourcePool::lockedResource ( const XMPP::Jid &jid )
{

	// check if the JID already carries a resource, then we will have to use that one
	if ( !jid.resource().isEmpty () )
	{
		// we are subscribed to a JID, find the according resource in the pool
		for ( JabberResource *mResource = mPool.first (); mResource; mResource = mPool.next () )
		{
			if ( ( mResource->jid().userHost().lower () == jid.userHost().lower () ) && ( mResource->resource().name () == jid.resource () ) )
			{
				return mResource->resource ();
			}
		}

		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "WARNING: No resource found in pool, returning as offline." << endl;

		return EmptyResource;
	}

	// see if we have a locked resource
	for(JabberResource *mResource = mLockList.first (); mResource; mResource = mLockList.next ())
	{
		if ( mResource->jid().userHost().lower() == jid.userHost().lower() )
		{
			kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Current lock for " << jid.userHost () << " is '" << mResource->resource().name () << "'" << endl;
			return mResource->resource ();
		}
	}

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "No lock available for " << jid.userHost () << endl;

	// there's no locked resource, return an empty resource
	return EmptyResource;

}

const XMPP::Resource &JabberResourcePool::bestResource ( const XMPP::Jid &jid, bool honourLock )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Determining best resource for " << jid.full () << endl;

	// FIXME: the code below would be more efficient if it would cache results

	if ( honourLock )
	{
		// if we are locked to a certain resource, always return that one
		const XMPP::Resource &mResource = lockedResource ( jid );
		if ( !mResource.name().isEmpty() )
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "We have a locked resource '" << mResource.name () << "' for " << jid.full () << endl;
			return mResource;
		}
	}

	JabberResource *bestResource = 0L;
	JabberResource *currentResource = 0L;

	for(currentResource = mPool.first (); currentResource; currentResource = mPool.next ())
	{
		// make sure we are only looking up resources for the specified JID
		if ( currentResource->jid().userHost().lower() != jid.userHost().lower() )
		{
			continue;
		}

		// take first resource if no resource has been chosen yet
		if(!bestResource)
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Taking '" << currentResource->resource().name () << "' as first available resource." << endl;

			bestResource = currentResource;
			continue;
		}

		if(currentResource->resource().priority() > bestResource->resource().priority())
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Using '" << currentResource->resource().name () << "' due to better priority." << endl;

			// got a better match by priority
			bestResource = currentResource;
		}
		else
		{
			if(currentResource->resource().priority() == bestResource->resource().priority())
			{
				if(currentResource->resource().status().timeStamp() > bestResource->resource().status().timeStamp())
				{
					kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Using '" << currentResource->resource().name () << "' due to better timestamp." << endl;

					// got a better match by timestamp (priorities are equal)
					bestResource = currentResource;
				}
			}
		}
	}

	return (bestResource == 0L) ? EmptyResource : bestResource->resource();

}

void JabberResourcePool::findResources ( const XMPP::Jid &jid, JabberResourcePool::ResourceList &resourceList )
{

	for(JabberResource *mResource = mPool.first (); mResource; mResource = mPool.next ())
	{
		if ( mResource->jid().userHost().lower() == jid.userHost().lower() )
		{
			// we found a resource for the JID, let's see if the JID already contains a resource
			if ( !jid.resource().isEmpty() && ( jid.resource().lower() != mResource->resource().name().lower() ) )
				// the JID contains a resource but it's not the one we have in the dictionary,
				// thus we have to ignore this resource
				continue;

			resourceList.append ( mResource );
		}
	}

}

void JabberResourcePool::findResources ( const XMPP::Jid &jid, XMPP::ResourceList &resourceList )
{

	for(JabberResource *mResource = mPool.first (); mResource; mResource = mPool.next ())
	{
		if ( mResource->jid().userHost().lower() == jid.userHost().lower() )
		{
			// we found a resource for the JID, let's see if the JID already contains a resource
			if ( !jid.resource().isEmpty() && ( jid.resource().lower() != mResource->resource().name().lower() ) )
				// the JID contains a resource but it's not the one we have in the dictionary,
				// thus we have to ignore this resource
				continue;

			resourceList.append ( mResource->resource () );
		}
	}

}

#include "jabberresourcepool.moc"
