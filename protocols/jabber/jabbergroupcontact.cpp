 /*
  * jabbercontact.cpp  -  Regular Kopete Jabber protocol contact
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

#include "jabbergroupcontact.h"

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberfiletransfer.h"
#include "jabbergroupchatmanager.h"
#include "jabbergroupmembercontact.h"
#include "jabbercontactpool.h"
#include "kopetemetacontact.h"

/**
 * JabberGroupContact constructor
 */
JabberGroupContact::JabberGroupContact (const XMPP::RosterItem &rosterItem, JabberAccount *account, KopeteMetaContact * mc)
				: JabberBaseContact ( XMPP::RosterItem ( rosterItem.jid().userHost () ), account, mc)
{

	// initialize here, we need it set before we instantiate the manager below
	mManager = 0;

	setFileCapable ( false );

	/**
	 * Add our own nick as first subcontact (we need to do that here
	 * because we need to set this contact as user() of the message
	 * manager).
	 */
	JabberBaseContact *subContact = addSubContact ( rosterItem );

	/**
	 * Instantiate a new message manager without members.
	 */
	mManager = new JabberGroupChatManager ( protocol (), subContact,
											KopeteContactPtrList (), XMPP::Jid ( rosterItem.jid().userHost () ) );

	connect ( mManager, SIGNAL ( closing ( KopeteMessageManager* ) ), this, SLOT ( slotMessageManagerDeleted () ) );

	/**
	 * FIXME: The first contact in the list of the message manager
	 * needs to be our own contact. This is a flaw in the Kopete
	 * API because it can't deal with group chat properly.
	 * If we are alone in a room, we are user() already and members()
	 * is empty. This makes at least the history plugin crash.
	 */
	mManager->addContact ( this );

}

JabberGroupContact::~JabberGroupContact ()
{

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << endl;

	delete mManager;

	for ( KopeteContact *contact = mContactList.first (); contact; contact = mContactList.next () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Deleting KC " << contact->contactId () << endl;
		delete contact;
	}

	for ( KopeteMetaContact *metaContact = mMetaContactList.first (); metaContact; metaContact = mMetaContactList.next () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Deleting KMC " << metaContact->metaContactId () << endl;
		delete metaContact;
	}

}

QPtrList<KAction> *JabberGroupContact::customContextMenuActions ()
{

	return 0;

}

void JabberGroupContact::rename ( const QString &/*newName*/ )
{

}

KopeteMessageManager *JabberGroupContact::manager ( bool /*canCreate*/ )
{

	return mManager;

}

void JabberGroupContact::handleIncomingMessage (const XMPP::Message & message)
{
	KopeteMessage::MessageType type;
	KopeteMessage *newMessage = 0L;

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Received Message Type:" << message.type () << endl;

	/**
	 * Don't display empty messages, these were most likely just carrying
	 * event notifications or other payload.
	 */
	if ( message.body().isEmpty () )
		return;

	// message type is always chat in a groupchat
	type = KopeteMessage::Chat;

	KopeteContactPtrList contactList;
	contactList.append ( mManager->user () );

	// check for errors
	if ( message.type () == "error" )
	{
		newMessage = new KopeteMessage( message.timeStamp (), this, contactList,
										i18n("Your message could not be delivered: \"%1\", Reason: \"%2\"").
										arg ( message.body () ).arg ( message.error().text ),
										message.subject(), KopeteMessage::Inbound, KopeteMessage::PlainText, type );
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
			/**
			 * We couldn't find the contact for this message. That most likely means
			 * that it originated from a history backlog or something similar and
			 * the sending person is not in the channel anymore. We need to create
			 * a new contact for this which does not show up in the manager.
			 */
			subContact = addSubContact ( XMPP::RosterItem ( message.from () ), false );
		}

		// convert XMPP::Message into KopeteMessage
		newMessage = new KopeteMessage ( message.timeStamp (), subContact, contactList, body,
										 message.subject (), KopeteMessage::Inbound,
										 KopeteMessage::PlainText, type );
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
	KopeteMetaContact *metaContact = new KopeteMetaContact ();
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

	// remove the contact from the message manager first
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

void JabberGroupContact::slotUserInfo ()
{

}

void JabberGroupContact::slotMessageManagerDeleted ()
{

	mManager = 0;

	if ( account()->isConnected () )
	{
		account()->client()->groupChatLeave ( mRosterItem.jid().host (), mRosterItem.jid().user () );
	}

}

#include "jabbergroupcontact.moc"
