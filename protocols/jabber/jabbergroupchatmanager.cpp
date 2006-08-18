/*
    jabbergroupchatmanager.cpp - Jabber Message Manager for group chats

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

#include <qptrlist.h>
#include <kdebug.h>
#include <klocale.h>
#include "kopetechatsessionmanager.h"
#include "kopeteview.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberclient.h"
#include "jabbercontact.h"

JabberGroupChatManager::JabberGroupChatManager ( JabberProtocol *protocol, const JabberBaseContact *user,
											 Kopete::ContactPtrList others, XMPP::Jid roomJid, const char *name )
											 : Kopete::ChatSession ( user, others, protocol,  name )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "New message manager for " << user->contactId () << endl;

	mRoomJid = roomJid;
	
	setMayInvite( true );

	// make sure Kopete knows about this instance
	Kopete::ChatSessionManager::self()->registerChatSession ( this );

	connect ( this, SIGNAL ( messageSent ( Kopete::Message &, Kopete::ChatSession * ) ),
			  this, SLOT ( slotMessageSent ( Kopete::Message &, Kopete::ChatSession * ) ) );

	updateDisplayName ();
}

JabberGroupChatManager::~JabberGroupChatManager()
{
}

void JabberGroupChatManager::updateDisplayName ()
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << endl;

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

		jabberMessage.setFrom ( account()->client()->jid() );
		

		XMPP::Jid toJid ( mRoomJid );

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
			jabberMessage.setBody ( i18n ( "This message is encrypted." ) );

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
		//NOTE: this is the obsolete, NOT RECOMMANDED protocol.
		//      iris doesn't implement groupchat yet
		XMPP::Message jabberMessage;
		jabberMessage.setFrom ( account()->client()->jid() );
		jabberMessage.setTo ( contactId );
		jabberMessage.setInvite( mRoomJid.userHost() );
		jabberMessage.setBody( i18n("You have been invited to %1").arg( mRoomJid.userHost() ) );

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

