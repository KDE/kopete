 /*
  * jabbergroupmembercontact.cpp  -  Regular Kopete Jabber protocol contact
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

#include "jabbergroupmembercontact.h"

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberfiletransfer.h"
#include "jabbergroupchatmanager.h"
#include "jabbermessagemanager.h"
#include "jabbercontactpool.h"
#include "kopetemetacontact.h"

/**
 * JabberGroupMemberContact constructor
 */
JabberGroupMemberContact::JabberGroupMemberContact (const XMPP::RosterItem &rosterItem,
													JabberAccount *account, KopeteMetaContact * mc)
													: JabberBaseContact ( rosterItem, account, mc)
{

	mc->setDisplayName ( rosterItem.jid().resource() );

	setFileCapable ( true );

	mManager = 0;

}

/**
 * JabberGroupMemberContact destructor
 */
JabberGroupMemberContact::~JabberGroupMemberContact ()
{

	delete mManager;
	
}

QPtrList<KAction> *JabberGroupMemberContact::customContextMenuActions ()
{

	return 0;

}

void JabberGroupMemberContact::rename ( const QString &/*newName*/ )
{

}

KopeteMessageManager *JabberGroupMemberContact::manager ( bool canCreate )
{

	if ( mManager )
		return mManager;
		
	if ( !mManager && !canCreate )
		return 0;

	KopeteContactPtrList chatMembers;
	chatMembers.append ( this );

	/*
	 * FIXME: We might have to use the group chat contact here instead of
	 *        the global myself() instance for a correct representation.
	 */
	mManager = new JabberMessageManager ( protocol(), static_cast<JabberBaseContact *>(account()->myself()), chatMembers );
	connect ( mManager, SIGNAL ( destroyed ( QObject * ) ), this, SLOT ( slotMessageManagerDeleted () ) );

	return mManager;

}

void JabberGroupMemberContact::slotMessageManagerDeleted ()
{

	mManager = 0;

}

void JabberGroupMemberContact::handleIncomingMessage ( const XMPP::Message &message )
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
	contactList.append ( manager( true )->user () );

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

		// convert XMPP::Message into KopeteMessage
		newMessage = new KopeteMessage ( message.timeStamp (), this, contactList, body,
										 message.subject (), KopeteMessage::Inbound,
										 KopeteMessage::PlainText, type );
	}

	// append message to manager
	manager( true )->appendMessage ( *newMessage );

	delete newMessage;

}

void JabberGroupMemberContact::sendFile ( const KURL &sourceURL, const QString &/*fileName*/, uint /*fileSize*/ )
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

void JabberGroupMemberContact::slotUserInfo ()
{

}

#include "jabbergroupmembercontact.moc"
