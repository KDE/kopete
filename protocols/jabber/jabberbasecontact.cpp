 /*
  * jabbercontact.cpp  -  Base class for the Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2004 by Till Gerken <till@tantalo.net>
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

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kopetegroup.h>
#include <kopetecontactlist.h>

#include "jabberbasecontact.h"

#include "xmpp_tasks.h"

#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberresourcepool.h"
#include "kopetemetacontact.h"

/**
 * JabberBaseContact constructor
 */
JabberBaseContact::JabberBaseContact (const XMPP::RosterItem &rosterItem, JabberAccount *account, Kopete::MetaContact * mc)
				: Kopete::Contact (account, rosterItem.jid().full().lower (), mc)
{

	// take roster item and update display name
	updateContact ( rosterItem );

	// since we're not in the account's contact pool yet
	// (we'll only be once we returned from this constructor),
	// we need to force an update to our status here
	// FIXME: libkopete doesn't allow us to use this call
	// here anymore as it causes an invocation of manager()
	// which is still a pure virtual in this constructor.
	// (needs to be done in subclasses instead)
	//reevaluateStatus ();

}

JabberProtocol *JabberBaseContact::protocol ()
{

	return static_cast<JabberProtocol *>(Kopete::Contact::protocol ());

}

JabberAccount *JabberBaseContact::account ()
{

	return static_cast<JabberAccount *>(Kopete::Contact::account ());

}

/* Return if we are reachable (defaults to true because
   we can send on- and offline, only return false if the
   account itself is offline, too) */
bool JabberBaseContact::isReachable ()
{
	if (account()->isConnected())
		return true;

	return false;

}

void JabberBaseContact::updateContact ( const XMPP::RosterItem & item )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Synchronizing local copy of " << contactId() << " with information received from server." << endl;

	mRosterItem = item;

	// if we don't have a meta contact yet, stop processing here
	if ( !metaContact () )
		return;

	// only update the nickname if its not empty
	if ( !item.name ().isEmpty () )
	{
		metaContact()->setDisplayName ( item.name () );
	}

	/*
	 * In this method, as opposed to KC::syncGroups(),
	 * the group list from the server is authoritative.
	 * As such, we need to find a list of all groups
	 * that the meta contact resides in but does not
	 * reside in on the server anymore, as well as all
	 * groups that the meta contact does not reside in,
	 * but resides in on the server.
	 * Then, we'll have to synchronize the KMC using
	 * that information.
	 */
	Kopete::GroupList groupsToRemoveFrom, groupsToAddTo;

	// find all groups our contact is in but that are not in the server side roster
	for ( unsigned i = 0; i < metaContact()->groups().count (); i++ )
	{
		if ( item.groups().find ( metaContact()->groups().at(i)->displayName () ) == item.groups().end () )
			groupsToRemoveFrom.append ( metaContact()->groups().at ( i ) );
	}

	// now find all groups that are in the server side roster but not in the local group
	for ( unsigned i = 0; i < item.groups().count (); i++ )
	{
		bool found = false;
		for ( unsigned j = 0; j < metaContact()->groups().count (); j++)
		{
			if ( metaContact()->groups().at(j)->displayName () == *item.groups().at(i) )
			{
				found = true;
				break;
			}
		}
		
		if ( !found )
		{
			groupsToAddTo.append ( Kopete::ContactList::self()->getGroup ( *item.groups().at(i) ) );
		}
	}

	/*
	 * Special case: if we don't add the contact to any group and the
	 * list of groups to remove from contains the top level group, we
	 * risk removing the contact from the visible contact list. In this
	 * case, we need to make sure at least the top level group stays.
	 */
	if ( ( groupsToAddTo.count () == 0 ) && ( groupsToRemoveFrom.contains ( Kopete::Group::topLevel () ) ) )
	{
		groupsToRemoveFrom.remove ( Kopete::Group::topLevel () );
	}

	for ( Kopete::Group *group = groupsToRemoveFrom.first (); group; group = groupsToRemoveFrom.next () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Removing " << contactId() << " from group " << group->displayName () << endl;
		metaContact()->removeFromGroup ( group, Kopete::MetaContact::DontSyncGroups );
	}
	
	for ( Kopete::Group *group = groupsToAddTo.first (); group; group = groupsToAddTo.next () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Adding " << contactId() << " to group " << group->displayName () << endl;
		metaContact()->addToGroup ( group, Kopete::MetaContact::DontSyncGroups );
	}

}

void JabberBaseContact::reevaluateStatus ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Determining new status for " << contactId () << endl;

	Kopete::OnlineStatus status;
	XMPP::Resource resource = account()->resourcePool()->bestResource ( mRosterItem.jid () );

	status = protocol()->resourceToKOS ( resource );

	/*
	 * Set away message property.
	 * We just need to read it from the current resource.
	 */
	if ( !resource.status ().status ().isEmpty () )
	{
		setProperty ( protocol()->propAwayMessage, resource.status().status () );
	}
	else
	{
		removeProperty ( protocol()->propAwayMessage );
	}

	/*
	 * Set available resources.
	 * This is a bit more complicated: We need to generate
	 * all images dynamically from the KOS icons and store
	 * them into the mime factory, then plug them into
	 * the richtext.
	 */
	JabberResourcePool::ResourceList resourceList;
	account()->resourcePool()->findResources ( XMPP::Jid ( contactId () ), resourceList );

	QString resourceListStr = "<table cellspacing=\"0\">";

	for ( JabberResourcePool::ResourceList::iterator it = resourceList.begin (); it != resourceList.end (); ++it )
	{
		// icon, resource name and priority
		resourceListStr += QString ( "<tr><td><img src=\"kopete-onlinestatus-icon:%1\" /> <b>%2</b> (Priority: %3)</td></tr>" ).
						   arg ( protocol()->resourceToKOS(*it).mimeSourceFor ( account () ) ).
						   arg ( (*it).name () ).
						   arg ( (*it).priority () );

		// resource timestamp
		resourceListStr += QString ( "<tr><td>%1: %2</td></tr>" ).
						   arg ( i18n ( "Timestamp" ) ).
						   arg ( (*it).status().timeStamp().toString ( Qt::LocalDate ) );
		
		// message, if any
		if ( !(*it).status().status().stripWhiteSpace().isEmpty () )
		{
			resourceListStr += QString ( "<tr><td>%1: %2</td></tr>" ).
							   arg ( i18n ( "Message" ) ).
							   arg ( (*it).status().status () );
		}
	}
	
	resourceListStr += "</table>";
	
	setProperty ( protocol()->propAvailableResources, resourceListStr );

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "New status for " << contactId () << " is " << status.description () << endl;
	setOnlineStatus ( status );

}

QString JabberBaseContact::fullAddress ()
{

	XMPP::Jid jid ( contactId () );

	if ( jid.resource().isEmpty () )
	{
		jid.setResource ( account()->resourcePool()->bestResource ( jid ).name () );
	}

	return jid.full ();

}

XMPP::Jid JabberBaseContact::bestAddress ()
{

	// see if we are subscribed with a preselected resource
	if ( !mRosterItem.jid().resource().isEmpty () )
	{
		// we have a preselected resource, so return our default full address
		return mRosterItem.jid ();
	}

	// construct address out of user@host and current best resource
	XMPP::Jid jid = mRosterItem.jid ();
	jid.setResource ( account()->resourcePool()->bestResource( mRosterItem.jid() ).name () );

	return jid;

}

void JabberBaseContact::serialize (QMap < QString, QString > &serializedData, QMap < QString, QString > & /* addressBookData */ )
{

	// Contact id and display name are already set for us, only add the rest
	serializedData["identityId"] = account()->accountId();

	serializedData["groups"] = mRosterItem.groups ().join (QString::fromLatin1 (","));
}

#include "jabberbasecontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
