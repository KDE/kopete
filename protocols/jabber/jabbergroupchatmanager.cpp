/*
    jabbergroupchatmanager.cpp - Jabber Message Manager for groupchats

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

#include "jabbergroupchatmanager.h"

#include <kdebug.h>
#include <klocale.h>
#include "kopetechatsessionmanager.h"
#include "kopeteview.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberclient.h"
#include "jabbercontact.h"

JabberGroupChatManager::JabberGroupChatManager ( JabberProtocol *protocol, const JabberBaseContact *user,
											 Kopete::ContactPtrList others, XMPP::Jid roomJid )
											 : Kopete::ChatSession ( user, others, protocol )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "New message manager for " << user->contactId ();

	mRoomJid = roomJid;
	
	setMayInvite( true );

	// make sure Kopete knows about this instance
	Kopete::ChatSessionManager::self()->registerChatSession ( this );

	connect ( this, SIGNAL (messageSent(Kopete::Message&,Kopete::ChatSession*)),
			  this, SLOT (slotMessageSent(Kopete::Message&,Kopete::ChatSession*)) );

	updateDisplayName ();
}

JabberGroupChatManager::~JabberGroupChatManager()
{
}

void JabberGroupChatManager::updateDisplayName ()
{
	kDebug ( JABBER_DEBUG_GLOBAL ) ;

	setDisplayName ( mRoomJid.full () );

}

const JabberBaseContact *JabberGroupChatManager::user () const
{

	return static_cast<const JabberBaseContact *>(Kopete::ChatSession::myself());

}

JabberAccount *JabberGroupChatManager::account () const
{

	return user()->account();

}

void JabberGroupChatManager::slotMessageSent ( Kopete::Message &message, Kopete::ChatSession * )
{

	if( account()->isConnected () )
	{
		XMPP::Message jabberMessage;

		//XMPP::Jid jid = static_cast<const JabberBaseContact*>(message.from())->rosterItem().jid() ;
		//jabberMessage.setFrom ( jid );

		XMPP::Jid toJid ( mRoomJid );

		jabberMessage.setTo ( toJid );

		jabberMessage.setSubject ( message.subject () );
		jabberMessage.setTimeStamp ( message.timestamp () );

		if ( ! account()->oldEncrypted() && message.plainBody().indexOf ( "-----BEGIN PGP MESSAGE-----" ) != -1 )
		{
			/*
			 * This message is encrypted, so we need to set
			 * a fake body indicating that this is an encrypted
			 * message (for clients not implementing this
			 * functionality) and then generate the encrypted
			 * payload out of the old message body.
			 */

			// please don't translate the following string
			jabberMessage.setBody ( "This message is signed or encrypted." );

			QString encryptedBody = message.plainBody().trimmed();

			// remove PGP header and footer from message
			encryptedBody.truncate ( encryptedBody.length () - QString("-----END PGP MESSAGE-----").length () - 2 );
			encryptedBody = encryptedBody.right ( encryptedBody.length () - encryptedBody.indexOf ( "\n\n" ) - 2 );

			// assign payload to message
			jabberMessage.setXEncrypted ( encryptedBody );
        }
        else
        {
			// this message is not encrypted
			jabberMessage.setBody ( message.plainBody () );
        }

		jabberMessage.setType ( "groupchat" );

		// send the message
		account()->client()->sendMessage ( jabberMessage );

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

void JabberGroupChatManager::inviteContact( const QString & contactId )
{
	if( account()->isConnected () )
	{
		//NOTE: this is the obsolete, NOT RECOMMENDED protocol.
		//      iris doesn't implement groupchat yet
		//NOTE: This code is duplicated in JabberProtocol::handleURL
		XMPP::Message jabberMessage;
//		XMPP::Jid jid = static_cast<const JabberBaseContact*>(account()->myself())->rosterItem().jid() ;
//		jabberMessage.setFrom ( jid );
		jabberMessage.setTo ( contactId );
		jabberMessage.setInvite( mRoomJid.bare() );
		jabberMessage.setBody( i18n("You have been invited to %1", mRoomJid.bare() ) );

		// send the message
		account()->client()->sendMessage ( jabberMessage );
	}
	else
	{
		account()->errorConnectFirst ();
	}
}


#include "jabbergroupchatmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

