 /*
  * jabbercontactpool.cpp
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
#include "jabbercontactpool.h"
#include "jabbercontact.h"
#include "jabberresourcepool.h"
#include "jabberaccount.h"

JabberContactPool::JabberContactPool ( JabberAccount *account )
{

	// automatically delete all contacts in the pool upon removal
	mPool.setAutoDelete (true);

	mAccount = account;

}

JabberContactPool::~JabberContactPool ()
{
}

JabberContact *JabberContactPool::addContact ( const XMPP::RosterItem &contact, KopeteMetaContact *metaContact, bool dirty )
{

	// see if the contact already exists
	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->contactId().lower() == contact.jid().full().lower() )
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Updating existing contact " << contact.jid().full() << endl;

			// It exists, updateit.
			mContactItem->contact()->updateContact ( contact );
			mContactItem->setDirty ( dirty );

			return mContactItem->contact ();
		}
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Adding new contact " << contact.jid().full() << endl;

	// create new contact instance and add it to the dictionary
	JabberContact *newContact = new JabberContact ( contact, mAccount, metaContact );
	JabberContactPoolItem *newContactItem = new JabberContactPoolItem ( newContact );
	connect ( newContact, SIGNAL ( contactDestroyed ( KopeteContact * ) ), this, SLOT ( slotContactDestroyed ( KopeteContact * ) ) );
	newContactItem->setDirty ( dirty );
	mPool.append ( newContactItem );

	return newContact;

}

void JabberContactPool::removeContact ( const XMPP::Jid &jid )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing contact " << jid.full() << endl;

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->contactId().lower() == jid.full().lower() )
		{
			mPool.removeNode ( mPool.currentNode() );
			return;
		}
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "WARNING: No match found!" << endl;

}

void JabberContactPool::slotContactDestroyed ( KopeteContact *contact )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Contact deleted, collecting the pieces..." << endl;

	JabberContact *jabberContact = static_cast<JabberContact *>( contact );

	// remove contact from the pool
	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact() == jabberContact )
		{
			mPool.removeNode ( mPool.currentNode() );
			break;
		}
	}

	// delete all resources for it
	mAccount->resourcePool()->removeAllResources ( XMPP::Jid ( contact->contactId() ) );

}

void JabberContactPool::clear ()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Clearing the contact pool." << endl;

	/*
	 * Since mPool has autodeletion enabled, this will cause all
	 * items to be deleted.
	 */
	mPool.clear ();

}

void JabberContactPool::setDirty ( const XMPP::Jid &jid, bool dirty )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Setting flag for " << jid.full() << " to " << dirty << endl;

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->contactId().lower() == jid.full().lower() )
		{
			mContactItem->setDirty ( dirty );
			return;
		}
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "WARNING: No match found!" << endl;

}

void JabberContactPool::cleanUp ()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Cleaning dirty items from contact pool." << endl;

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->dirty () )
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing dirty contact " << mContactItem->contact()->contactId () << endl;

			// this will cause the contact instance itself to be deleted
			mPool.removeNode ( mPool.currentNode() );
		}
	}

}

JabberContact *JabberContactPool::findExactMatch ( const XMPP::Jid &jid )
{

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->contactId().lower () == jid.full().lower () )
		{
			return mContactItem->contact ();
		}
	}

	return 0L;

}

JabberContact *JabberContactPool::findRelevantRecipient ( const XMPP::Jid &jid )
{

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->contactId().lower () == jid.userHost().lower () )
		{
			return mContactItem->contact ();
		}
	}

	return 0L;

}

QPtrList<JabberContact> JabberContactPool::findRelevantSources ( const XMPP::Jid &jid )
{
	QPtrList<JabberContact> list;

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( XMPP::Jid ( mContactItem->contact()->contactId() ).userHost().lower () == jid.userHost().lower () )
		{
			list.append ( mContactItem->contact () );
		}
	}

	return list;

}

JabberContactPoolItem::JabberContactPoolItem ( JabberContact *contact )
{
	mDirty = true;
	canDelete = true;
	mContact = contact;
	connect ( contact, SIGNAL ( destroyed () ), this, SLOT ( slotContactDestroyed () ) );
}

JabberContactPoolItem::~JabberContactPoolItem ()
{
	if ( canDelete )
		delete mContact;
}

void JabberContactPoolItem::setDirty ( bool dirty )
{
	mDirty = dirty;
}

bool JabberContactPoolItem::dirty ()
{
	return mDirty;
}

JabberContact *JabberContactPoolItem::contact ()
{
	return mContact;
}

void JabberContactPoolItem::slotContactDestroyed ()
{
	canDelete = false;
}

#include "jabbercontactpool.moc"
