 /*
  * jabbercontactpool.cpp
  *
  * Copyright (c) 2004 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2006      by Olivier Goffart  <ogoffart at kde.org>
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

#include "jabbercontactpool.h"

#include <qptrlist.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kopeteaccountmanager.h>
#include <kopetecontactlist.h>
#include "kopeteuiglobal.h"
#include "jabberprotocol.h"
#include "jabberbasecontact.h"
#include "jabbercontact.h"
#include "jabbergroupcontact.h"
#include "jabbergroupmembercontact.h"
#include "jabberresourcepool.h"
#include "jabberaccount.h"
#include "jabbertransport.h"

JabberContactPool::JabberContactPool ( JabberAccount *account )
{

	// automatically delete all contacts in the pool upon removal
	mPool.setAutoDelete (true);

	mAccount = account;

}

JabberContactPool::~JabberContactPool ()
{
}

JabberContactPoolItem *JabberContactPool::findPoolItem ( const XMPP::RosterItem &contact )
{

	// see if the contact already exists
	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->rosterItem().jid().full().lower() == contact.jid().full().lower() )
		{
			return mContactItem;
		}
	}

	return 0;

}

JabberContact *JabberContactPool::addContact ( const XMPP::RosterItem &contact, Kopete::MetaContact *metaContact, bool dirty )
{
	// see if the contact already exists
	JabberContactPoolItem *mContactItem = findPoolItem ( contact );
	if ( mContactItem)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Updating existing contact " << contact.jid().full() << "   -  " <<   mContactItem->contact() << endl;

		// It exists, update it.
		mContactItem->contact()->updateContact ( contact );
		mContactItem->setDirty ( dirty );

		JabberContact *retval = dynamic_cast<JabberContact *>(mContactItem->contact ());

		if ( !retval )
		{
			KMessageBox::error ( Kopete::UI::Global::mainWidget (),
								 "Fatal error in the Jabber contact pool. Please restart Kopete and submit a debug log "
								 "of your session to http://bugs.kde.org.",
								 "Fatal Jabber Error" );
		}

		return retval;
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Adding new contact " << contact.jid().full() << endl;
	
	JabberTransport *transport=0l;
	QString legacyId;
	//find if the contact should be added to a transport.
	if(mAccount->transports().contains(contact.jid().domain()))
	{
		transport=mAccount->transports()[contact.jid().domain()];
		legacyId=transport->legacyId( contact.jid() );
	}
		
	// create new contact instance and add it to the dictionary
	JabberContact *newContact = new JabberContact ( contact, transport ? (Kopete::Account*)transport : (Kopete::Account*)mAccount, metaContact , legacyId );
	JabberContactPoolItem *newContactItem = new JabberContactPoolItem ( newContact );
	connect ( newContact, SIGNAL ( contactDestroyed ( Kopete::Contact * ) ), this, SLOT ( slotContactDestroyed ( Kopete::Contact * ) ) );
	newContactItem->setDirty ( dirty );
	mPool.append ( newContactItem );

	return newContact;

}

JabberBaseContact *JabberContactPool::addGroupContact ( const XMPP::RosterItem &contact, bool roomContact, Kopete::MetaContact *metaContact, bool dirty )
{

	XMPP::RosterItem mContact ( roomContact ? contact.jid().userHost () : contact.jid().full() );

	// see if the contact already exists
	JabberContactPoolItem *mContactItem = findPoolItem ( mContact );
	if ( mContactItem)
	{
		if(mContactItem->contact()->inherits(roomContact ?
				 (const char*)("JabberGroupContact") : (const char*)("JabberGroupMemberContact") ) )
		{
			
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Updating existing contact " << mContact.jid().full() << endl;
			
			// It exists, update it.
			mContactItem->contact()->updateContact ( mContact );
			mContactItem->setDirty ( dirty );
	
			//we must tell to the originating function that no new contact has been added
			return 0L;//mContactItem->contact ();
		}
		else
		{
			//this happen if we receive a MUC invitaiton:  when the invitaiton is received, it's from the muc itself
			//and then kopete will create a temporary contact for it. but it will not be a good contact.
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Bad contact will be removed and re-added " << mContact.jid().full() << endl;
			Kopete::MetaContact *old_mc=mContactItem->contact()->metaContact();
			delete mContactItem->contact();
			mContactItem = 0L;
			if(old_mc->contacts().isEmpty() && old_mc!=metaContact)
			{
				Kopete::ContactList::self()->removeMetaContact( old_mc );
			}
			
		}

	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Adding new contact " << mContact.jid().full() << endl;

	// create new contact instance and add it to the dictionary
	JabberBaseContact *newContact;

	if ( roomContact )
		newContact = new JabberGroupContact ( contact, mAccount, metaContact );
	else
		newContact = new JabberGroupMemberContact ( contact, mAccount, metaContact );

	JabberContactPoolItem *newContactItem = new JabberContactPoolItem ( newContact );

	connect ( newContact, SIGNAL ( contactDestroyed ( Kopete::Contact * ) ), this, SLOT ( slotContactDestroyed ( Kopete::Contact * ) ) );

	newContactItem->setDirty ( dirty );
	mPool.append ( newContactItem );

	return newContact;

}

void JabberContactPool::removeContact ( const XMPP::Jid &jid )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing contact " << jid.full() << endl;

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->rosterItem().jid().full().lower() == jid.full().lower() )
		{
			/*
			 * The following deletion will cause slotContactDestroyed()
			 * to be called, which will clean the up the list.
			 */
			if(mContactItem->contact())
			{
				Kopete::MetaContact *mc=mContactItem->contact()->metaContact();
				delete mContactItem->contact ();
				if(mc && mc->contacts().isEmpty())
				{
					Kopete::ContactList::self()->removeMetaContact(mc) ;
				}
			}
			return;
		}
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "WARNING: No match found!" << endl;

}

void JabberContactPool::slotContactDestroyed ( Kopete::Contact *contact )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Contact deleted, collecting the pieces..." << endl;

	JabberBaseContact *jabberContact = static_cast<JabberBaseContact *>( contact ); 
	//WARNING  this ptr is not usable, we are in the Kopete::Contact destructor

	// remove contact from the pool
	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact() == jabberContact )
		{
			mPool.remove ();
			break;
		}
	}

	// delete all resources for it
	if(contact->account()==(Kopete::Account*)(mAccount))
		mAccount->resourcePool()->removeAllResources ( XMPP::Jid ( contact->contactId() ) );
	else
	{
		//this is a legacy contact. we have no way to get the real Jid at this point, we can only guess it.
		QString contactId= contact->contactId().replace('@','%') + "@" + contact->account()->myself()->contactId();
		mAccount->resourcePool()->removeAllResources ( XMPP::Jid ( contactId ) ) ;
	}

}

void JabberContactPool::clear ()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Clearing the contact pool." << endl;

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		/*
		 * The following deletion will cause slotContactDestroyed()
		 * to be called, which will clean the up the list.
		 * NOTE: this is a very inefficient way to clear the list
		 */
		delete mContactItem->contact ();
	}

}

void JabberContactPool::setDirty ( const XMPP::Jid &jid, bool dirty )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Setting flag for " << jid.full() << " to " << dirty << endl;

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->rosterItem().jid().full().lower() == jid.full().lower() )
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

			/*
			 * The following deletion will cause slotContactDestroyed()
			 * to be called, which will clean the up the list.
			 */
			delete mContactItem->contact ();
		}
	}

}

JabberBaseContact *JabberContactPool::findExactMatch ( const XMPP::Jid &jid )
{

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->rosterItem().jid().full().lower () == jid.full().lower () )
		{
			return mContactItem->contact ();
		}
	}

	return 0L;

}

JabberBaseContact *JabberContactPool::findRelevantRecipient ( const XMPP::Jid &jid )
{

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->rosterItem().jid().full().lower () == jid.userHost().lower () )
		{
			return mContactItem->contact ();
		}
	}

	return 0L;

}

QPtrList<JabberBaseContact> JabberContactPool::findRelevantSources ( const XMPP::Jid &jid )
{
	QPtrList<JabberBaseContact> list;

	for(JabberContactPoolItem *mContactItem = mPool.first (); mContactItem; mContactItem = mPool.next ())
	{
		if ( mContactItem->contact()->rosterItem().jid().userHost().lower () == jid.userHost().lower () )
		{
			list.append ( mContactItem->contact () );
		}
	}

	return list;

}

JabberContactPoolItem::JabberContactPoolItem ( JabberBaseContact *contact )
{
	mDirty = true;
	mContact = contact;
}

JabberContactPoolItem::~JabberContactPoolItem ()
{
}

void JabberContactPoolItem::setDirty ( bool dirty )
{
	mDirty = dirty;
}

bool JabberContactPoolItem::dirty ()
{
	return mDirty;
}

JabberBaseContact *JabberContactPoolItem::contact ()
{
	return mContact;
}

#include "jabbercontactpool.moc"
