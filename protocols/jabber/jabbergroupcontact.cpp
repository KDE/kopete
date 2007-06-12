 /*
  * jabbercontact.cpp  -  Regular Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2004 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2006      by Olivier Goffart <ogoffart @ kde.org>
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

#include "jabbergroupcontact.h"

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberclient.h"
#include "jabberfiletransfer.h"
#include "jabbergroupchatmanager.h"
#include "jabbergroupmembercontact.h"
#include "jabbercontactpool.h"
#include "kopetemetacontact.h"
#include "xmpp_tasks.h"

/**
 * JabberGroupContact constructor
 */
JabberGroupContact::JabberGroupContact (const XMPP::RosterItem &rosterItem, JabberAccount *account, Kopete::MetaContact * mc)
	: JabberBaseContact ( XMPP::RosterItem ( rosterItem.jid().userHost () ), account, mc) , mNick( rosterItem.jid().resource() )
{
	setIcon( "jabber_group" );
	
	// initialize here, we need it set before we instantiate the manager below
	mManager = 0;

	setFileCapable ( false );

	/**
	 * Add our own nick as first subcontact (we need to do that here
	 * because we need to set this contact as myself() of the message
	 * manager).
	 */
	mSelfContact = addSubContact ( rosterItem );

	/**
	 * Instantiate a new message manager without members.
	 */
	mManager = new JabberGroupChatManager ( protocol (), mSelfContact,
											Kopete::ContactPtrList (), XMPP::Jid ( rosterItem.jid().userHost () ) );

	connect ( mManager, SIGNAL ( closing ( Kopete::ChatSession* ) ), this, SLOT ( slotChatSessionDeleted () ) );
	
	connect ( account->myself() , SIGNAL(onlineStatusChanged( Kopete::Contact*, const Kopete::OnlineStatus&, const Kopete::OnlineStatus& ) ) ,
			  this , SLOT(slotStatusChanged()  ) ) ;

	/**
	 * FIXME: The first contact in the list of the message manager
	 * needs to be our own contact. This is a flaw in the Kopete
	 * API because it can't deal with group chat properly.
	 * If we are alone in a room, we are myself() already and members()
	 * is empty. This makes at least the history plugin crash.
	 */
	mManager->addContact ( this );
	
	
	
	/**
	 * Let's construct the window:
	 *  otherwise, the ref count of maznager is equal to zero. 
	 *   and if we receive a message before the window is shown,
	 *   it will be deleted and we will be out of the channel
	 * In all case, there are no reason to don't show it.
	 */
	mManager->view( true , "kopete_chatwindow" );
}

JabberGroupContact::~JabberGroupContact ()
{

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << endl;

	if(mManager) 
	{
		mManager->deleteLater();
	}
	
	for ( Kopete::Contact *contact = mContactList.first (); contact; contact = mContactList.next () )
	{
		/*if(mManager)
		mManager->removeContact( contact );*/
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Deleting KC " << contact->contactId () << endl;
		contact->deleteLater();
	}

	for ( Kopete::MetaContact *metaContact = mMetaContactList.first (); metaContact; metaContact = mMetaContactList.next () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Deleting KMC " << metaContact->metaContactId () << endl;
		metaContact->deleteLater();
	}
}

QPtrList<KAction> *JabberGroupContact::customContextMenuActions ()
{
	QPtrList<KAction> *actionCollection = new QPtrList<KAction>();

	KAction *actionSetNick = new KAction (i18n ("Change nick name"), 0, 0, this, SLOT (slotChangeNick()), this, "jabber_changenick");
	actionCollection->append( actionSetNick );

	return actionCollection;
}

Kopete::ChatSession *JabberGroupContact::manager ( Kopete::Contact::CanCreateFlags canCreate )
{
	if(!mManager && canCreate == Kopete::Contact::CanCreate)
	{
		kdWarning (JABBER_DEBUG_GLOBAL) << k_funcinfo << "somehow, the chat manager was removed, and the contact is still there" << endl;
		mManager = new JabberGroupChatManager ( protocol (), mSelfContact,
				Kopete::ContactPtrList (), XMPP::Jid ( rosterItem().jid().userHost() ) );

		mManager->addContact ( this );
		
		connect ( mManager, SIGNAL ( closing ( Kopete::ChatSession* ) ), this, SLOT ( slotChatSessionDeleted () ) );
		
		//if we have to recreate the manager, we probably have to connect again to the chat.
		slotStatusChanged();
	}
	return mManager;

}

void JabberGroupContact::handleIncomingMessage (const XMPP::Message & message)
{
	// message type is always chat in a groupchat
	QString viewType = "kopete_chatwindow";
	Kopete::Message *newMessage = 0L;
	
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Received a message"  << endl;

	/**
	 * Don't display empty messages, these were most likely just carrying
	 * event notifications or other payload.
	 */
	if ( message.body().isEmpty () )
		return;

	manager(CanCreate); //force to create mManager
	
	Kopete::ContactPtrList contactList = manager()->members();

	// check for errors
	if ( message.type () == "error" )
	{
		newMessage = new Kopete::Message( message.timeStamp (), this, contactList,
										i18n("Your message could not be delivered: \"%1\", Reason: \"%2\"").
										arg ( message.body () ).arg ( message.error().text ),
										message.subject(), Kopete::Message::Inbound, Kopete::Message::PlainText, viewType );
	}
	else
	{
		// retrieve and reformat body
		QString body = message.body ();

		if( !message.xencrypted().isEmpty () )
		{
			body = QString ("-----BEGIN PGP MESSAGE-----\n\n") + message.xencrypted () + QString ("\n-----END PGP MESSAGE-----\n");
		}

		// locate the originating contact
		JabberBaseContact *subContact = account()->contactPool()->findExactMatch ( message.from () );

		if ( !subContact )
		{
			kdWarning (JABBER_DEBUG_GLOBAL) << k_funcinfo << "the contact is not in the list   : " <<  message.from().full()<< endl;

			/**
			 * We couldn't find the contact for this message. That most likely means
			 * that it originated from a history backlog or something similar and
			 * the sending person is not in the channel anymore. We need to create
			 * a new contact for this which does not show up in the manager.
			 */
			subContact = addSubContact ( XMPP::RosterItem ( message.from () ), false );
		}

		// convert XMPP::Message into Kopete::Message
		newMessage = new Kopete::Message ( message.timeStamp (), subContact, contactList, body,
										 message.subject (),
										 subContact != mManager->myself() ? Kopete::Message::Inbound : Kopete::Message::Outbound,
										 Kopete::Message::PlainText, viewType );
	}

	// append message to manager
	mManager->appendMessage ( *newMessage );

	delete newMessage;

}

JabberBaseContact *JabberGroupContact::addSubContact ( const XMPP::RosterItem &rosterItem, bool addToManager )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Adding new subcontact " << rosterItem.jid().full () << " to room " << mRosterItem.jid().full () << endl;

	// see if this contact already exists, skip creation otherwise
	JabberBaseContact *subContact = dynamic_cast<JabberGroupMemberContact *>( account()->contactPool()->findExactMatch ( rosterItem.jid () ) );

	if ( subContact )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Contact already exists, not adding again." << endl;
		return subContact;
	}
	
	// Create new meta contact that holds the group chat contact.
	Kopete::MetaContact *metaContact = new Kopete::MetaContact ();
	metaContact->setTemporary ( true );
	mMetaContactList.append ( metaContact );

	// now add contact to the pool, no dirty flag
	subContact = account()->contactPool()->addGroupContact ( rosterItem, false, metaContact, false );

	/**
	 * Add the contact to our message manager first. We need
	 * to check the pointer for validity, because this method
	 * gets called from the constructor, where the manager
	 * does not exist yet.
	 */
	if ( mManager && addToManager )
		mManager->addContact ( subContact );

	// now, add the contact also to our own list
	mContactList.append ( subContact );
	
	connect(subContact , SIGNAL(contactDestroyed(Kopete::Contact*)) , this , SLOT(slotSubContactDestroyed(Kopete::Contact*)));

	return subContact;

}

void JabberGroupContact::removeSubContact ( const XMPP::RosterItem &rosterItem )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Removing subcontact " << rosterItem.jid().full () << " from room " << mRosterItem.jid().full () << endl;

	// make sure that subcontacts are only removed from the room contact, which has no resource
	if ( !mRosterItem.jid().resource().isEmpty () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "WARNING: Trying to remove subcontact from subcontact!" << endl;
		return;
	}

	// find contact in the pool
	JabberGroupMemberContact *subContact = dynamic_cast<JabberGroupMemberContact *>( account()->contactPool()->findExactMatch ( rosterItem.jid () ) );

	if ( !subContact )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "WARNING: Subcontact couldn't be located!" << endl;
		return;
	}
	
	if(mManager && subContact->contactId() == mManager->myself()->contactId() )
	{
		//HACK WORKAROUND FIXME KDE4
		//impossible to remove myself, or we will die
		//subContact->setNickName( mNick ); //this is even worse than nothing
		return;
	}

	// remove the contact from the message manager first
	if(mManager)
		mManager->removeContact ( subContact );

	// remove the contact's meta contact from our internal list
	mMetaContactList.remove ( subContact->metaContact () );

	// remove the contact from our internal list
	mContactList.remove ( subContact );

	// delete the meta contact first
	delete subContact->metaContact ();

	// finally, delete the contact itself from the pool
	account()->contactPool()->removeContact ( rosterItem.jid () );

}

void JabberGroupContact::sendFile ( const KURL &sourceURL, const QString &/*fileName*/, uint /*fileSize*/ )
{
	QString filePath;

	// if the file location is null, then get it from a file open dialog
	if ( !sourceURL.isValid () )
		filePath = KFileDialog::getOpenFileName( QString::null , "*", 0L, i18n ( "Kopete File Transfer" ) );
	else
		filePath = sourceURL.path(-1);

	QFile file ( filePath );

	if ( file.exists () )
	{
		// send the file
		new JabberFileTransfer ( account (), this, filePath );
	}

}

void JabberGroupContact::slotChatSessionDeleted ()
{

	mManager = 0;

	if ( account()->isConnected () )
	{
		account()->client()->leaveGroupChat ( mRosterItem.jid().host (), mRosterItem.jid().user () );
	}
	
	//deleteLater(); //we will be deleted later when the the account will know we have left

}

void JabberGroupContact::slotStatusChanged( )
{
	if( !account()->isConnected() )
	{
		//we need to remove all contact, because when we connect again, we will not receive the notificaion they are gone.
		QPtrList<Kopete::Contact> copy_contactlist=mContactList;
		for ( Kopete::Contact *contact = copy_contactlist.first (); contact; contact = copy_contactlist.next () )
		{
			removeSubContact( XMPP::Jid(contact->contactId()) );
		}
		return;
	}
	
	
	if( !isOnline() )
	{
		//HACK WORKAROUND   XMPP::client->d->groupChatList must contains us.
		account()->client()->joinGroupChat( rosterItem().jid().host() , rosterItem().jid().user() , mNick );
	}
	
	//TODO: away message
	XMPP::Status newStatus = account()->protocol()->kosToStatus( account()->myself()->onlineStatus() );
	account()->client()->setGroupChatStatus( rosterItem().jid().host() , rosterItem().jid().user() , newStatus );
}

void JabberGroupContact::slotChangeNick( )
{
	
	bool ok;
	QString futureNewNickName = KInputDialog::getText( i18n( "Change nickanme - Jabber Plugin" ),
			i18n( "Please enter the new nick name you want to have on the room <i>%1</i>" ).arg(rosterItem().jid().userHost()),
			mNick, &ok );
	if ( !ok || !account()->isConnected())
		return;
	
	mNick=futureNewNickName;
	
	XMPP::Status status = account()->protocol()->kosToStatus( account()->myself()->onlineStatus() );
	account()->client()->changeGroupChatNick( rosterItem().jid().host() , rosterItem().jid().user()  , mNick , status);

}

void JabberGroupContact::slotSubContactDestroyed( Kopete::Contact * deadContact )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "cleaning dead subcontact " << deadContact->contactId() << " from room " << mRosterItem.jid().full () << endl;

	mMetaContactList.remove ( deadContact->metaContact () );
	mContactList.remove ( deadContact );

}

#include "jabbergroupcontact.moc"
