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

#include "jabberbasecontact.h"

#include "xmpp_tasks.h"

#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberresourcepool.h"
#include "kopetemetacontact.h"

/**
 * JabberBaseContact constructor
 */
JabberBaseContact::JabberBaseContact (const XMPP::RosterItem &rosterItem, JabberAccount *account, KopeteMetaContact * mc)
				: KopeteContact (account, rosterItem.jid().full(), mc)
{

	// take roster item and update display name
	updateContact ( rosterItem );

	// since we're not in the account's contact pool yet
	// (we'll only be once we returned from this constructor),
	// we need to force an update to our status here
	reevaluateStatus ();

}

JabberProtocol *JabberBaseContact::protocol ()
{

	return static_cast<JabberProtocol *>(KopeteContact::protocol ());

}

JabberAccount *JabberBaseContact::account ()
{

	return static_cast<JabberAccount *>(KopeteContact::account ());

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

	mRosterItem = item;

	// only update the nickname if its not empty
	if ( !item.name ().isEmpty () )
	{
		metaContact()->setDisplayName ( item.name () );
	}

	/*
	 * FIXME: synchronize group list here!
	 * The tricky part: updating the KMC
	 * group list cannot be done in one
	 * step, additionally each of these steps
	 * will cause the KMC to send out a
	 * synch request which in turn
	 * causes this method to be called
	 * once more, resulting in an endless
	 * loop and a DoS on the server.
	 */

}

void JabberBaseContact::reevaluateStatus ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Determining new status for " << contactId () << endl;

	KopeteOnlineStatus status;
	XMPP::Resource resource = account()->resourcePool()->bestResource ( mRosterItem.jid () );

	// update to online by default
	status = protocol()->JabberKOSOffline;

	if ( !resource.status().isAvailable () )
	{
		// resource is offline
		status = protocol()->JabberKOSOffline;
	}
	else
	{
		if (resource.status ().show () == "")
		{
			status = protocol()->JabberKOSOnline;
		}
		else
		if (resource.status ().show () == "chat")
		{
			status = protocol()->JabberKOSChatty;
		}
		else if (resource.status ().show () == "away")
		{
			status = protocol()->JabberKOSAway;
		}
		else if (resource.status ().show () == "xa")
		{
			status = protocol()->JabberKOSXA;
		}
		else if (resource.status ().show () == "dnd")
		{
			status = protocol()->JabberKOSDND;
		}
		else if (resource.status ().show () == "connecting")
		{
			status = protocol()->JabberKOSConnecting;
		}
	}

	// remove properties first
	removeProperty ( protocol()->propAwayMessage );

	// set away message property
	if ( !resource.status ().status ().isEmpty () )
		setProperty ( protocol()->propAwayMessage, resource.status().status () );

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
