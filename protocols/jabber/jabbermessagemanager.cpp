/*
    jabbermessagemanager.cpp - Jabber Message Manager

    Copyright (c) 2004 by Till Gerken            <till@tantalo.net>

    Kopete    (c) 2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "jabbermessagemanager.h"

#include "kopetemessagemanagerfactory.h"
#include "kopeteview.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabbercontact.h"

JabberMessageManager::JabberMessageManager ( JabberProtocol *protocol, const JabberContact *user,
											 KopeteContactPtrList others, const QString &resource, const char *name )
											 : KopeteMessageManager ( user, others, protocol, 0, name )
{

	// make sure Kopete knows about this instance
	KopeteMessageManagerFactory::factory()->addKopeteMessageManager ( this );

	connect ( this, SIGNAL ( messageSent ( KopeteMessage &, KopeteMessageManager * ) ),
			  this, SLOT ( slotMessageSent ( KopeteMessage &, KopeteMessageManager * ) ) );

	// check if the user ID contains a hardwired resource,
	// we'll have to use that one in that case
	XMPP::Jid jid ( user->contactId () );

	mResource = jid.resource().isEmpty () ? resource : jid.resource ();

	updateDisplayName ();

}

JabberMessageManager::~JabberMessageManager ()
{

}

void JabberMessageManager::updateDisplayName ()
{
	KopeteContactPtrList chatMembers = members ();

	XMPP::Jid jid ( chatMembers.first()->contactId () );

	if ( !mResource.isEmpty () )
		jid.setResource ( mResource );

	setDisplayName ( jid.full () );

}

const JabberContact *JabberMessageManager::user () const
{

	return static_cast<const JabberContact *>(KopeteMessageManager::user());

}

JabberAccount *JabberMessageManager::account () const
{

	return static_cast<JabberAccount *>(KopeteMessageManager::account ());

}

const QString &JabberMessageManager::resource () const
{

	return mResource;

}

void JabberMessageManager::appendMessage ( KopeteMessage &msg, const QString &fromResource )
{

	mResource = fromResource;

	updateDisplayName ();

	KopeteMessageManager::appendMessage ( msg );

}

void JabberMessageManager::slotMessageSent ( KopeteMessage &message, KopeteMessageManager * )
{

	if( account()->isConnected () )
	{
		XMPP::Message jabberMessage;
		KopeteContact *recipient = message.to().first ();

		jabberMessage.setFrom ( XMPP::Jid ( message.from()->contactId () ) );

		XMPP::Jid toJid ( recipient->contactId () );

		if( !resource().isEmpty () )
			toJid.setResource ( resource() );

		jabberMessage.setTo ( toJid );

		jabberMessage.setSubject ( message.subject () );
		jabberMessage.setTimeStamp ( message.timestamp () );

		if ( message.plainBody().find ( "-----BEGIN PGP MESSAGE-----" ) != -1 )
		{
			/*
			 * This message is encrypted, so we need to set
			 * a fake body indicating that this is an encrypted
			 * message (for clients not implementing this
			 * functionality) and then generate the encrypted
			 * payload out of the old message body.
			 */

			// please don't translate the following string
			jabberMessage.setBody ( "This message is encrypted.", false );

			QString encryptedBody = message.plainBody ();

			// remove PGP header and footer from message
			encryptedBody.truncate ( encryptedBody.length () - QString("-----END PGP MESSAGE-----").length () - 2 );
			encryptedBody = encryptedBody.right ( encryptedBody.length () - encryptedBody.find ( "\n\n" ) - 2 );

			// assign payload to message
			jabberMessage.setXEncrypted ( encryptedBody );
        }
        else
        {
			// this message is not encrypted
			jabberMessage.setBody ( message.plainBody (), false );
        }

		// determine type of the widget and set message type accordingly
        if ( view()->viewType () == KopeteMessage::Chat)
		{
			jabberMessage.setType ("chat");
		}
        else
		{
			jabberMessage.setType ("normal");
		}

		// send the message
		account()->client()->sendMessage ( jabberMessage );

		// append the message to the manager
		KopeteMessageManager::appendMessage ( message );

		// tell the manager that we sent successfully
		messageSucceeded ();
	}
	else
	{
		account()->errorConnectFirst ();

		// FIXME: there is no messageFailed() yet,
		// but we need to stop the animation etc.
		messageSucceeded ();
	}

}

#include "jabbermessagemanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

