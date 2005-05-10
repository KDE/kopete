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

#include "jabberchatsession.h"

#include <qptrlist.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include "kopetemessagemanagerfactory.h"
#include "kopeteviewplugin.h"
#include "kopeteview.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabbercontact.h"

JabberChatSession::JabberChatSession ( JabberProtocol *protocol, const JabberBaseContact *user,
											 Kopete::ContactPtrList others, const QString &resource, const char *name )
											 : Kopete::ChatSession ( user, others, protocol,  name )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "New message manager for " << user->contactId () << endl;

	// make sure Kopete knows about this instance
	Kopete::ChatSessionManager::self()->registerChatSession ( this );

	connect ( this, SIGNAL ( messageSent ( Kopete::Message &, Kopete::ChatSession * ) ),
			  this, SLOT ( slotMessageSent ( Kopete::Message &, Kopete::ChatSession * ) ) );

	connect ( this, SIGNAL ( myselfTyping ( bool ) ), this, SLOT ( slotSendTypingNotification ( bool ) ) );

	// check if the user ID contains a hardwired resource,
	// we'll have to use that one in that case
	XMPP::Jid jid ( user->contactId () );

	mResource = jid.resource().isEmpty () ? resource : jid.resource ();

	updateDisplayName ();

}

void JabberChatSession::updateDisplayName ()
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << endl;

	Kopete::ContactPtrList chatMembers = members ();

	// make sure we do have members in the chat
	if ( !chatMembers.first () )
		return;

	XMPP::Jid jid ( chatMembers.first()->contactId () );

	if ( !mResource.isEmpty () )
		jid.setResource ( mResource );

	QString statusText = i18n("a contact's online status in parenthesis.", " (%1)")
							.arg( chatMembers.first()->onlineStatus().description() );
	if ( jid.resource().isEmpty () )
		setDisplayName ( chatMembers.first()->metaContact()->displayName () + statusText );
	else
		setDisplayName ( chatMembers.first()->metaContact()->displayName () + "/" + jid.resource () + statusText );

}

const JabberBaseContact *JabberChatSession::user () const
{

	return static_cast<const JabberBaseContact *>(Kopete::ChatSession::myself());

}

JabberAccount *JabberChatSession::account () const
{

	return static_cast<JabberAccount *>(Kopete::ChatSession::account ());

}

const QString &JabberChatSession::resource () const
{

	return mResource;

}

void JabberChatSession::appendMessage ( Kopete::Message &msg, const QString &fromResource )
{

	mResource = fromResource;

	updateDisplayName ();

	Kopete::ChatSession::appendMessage ( msg );

}

void JabberChatSession::slotSendTypingNotification ( bool typing )
{

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Sending out typing notification (" << typing << ") to all chat members." << endl;

	if ( !account()->isConnected () )
		return;

	Kopete::Contact *contact;
	QPtrListIterator<Kopete::Contact> listIterator ( members () );

	while ( ( contact = listIterator.current () ) != 0 )
	{
		++listIterator;

		// create JID for us as sender
		XMPP::Jid fromJid ( myself()->contactId () );
		fromJid.setResource ( account()->configGroup()->readEntry( "Resource", QString::null ) );

		// create JID for the recipient
		XMPP::Jid toJid ( contact->contactId () );

		// set resource properly if it has been selected already
		if ( !resource().isEmpty () )
			toJid.setResource ( resource () );

		XMPP::Message message;

		message.setFrom ( fromJid );
		message.setTo ( toJid );

		// store composing event depending on state
		typing ? message.addEvent ( ComposingEvent ) : message.addEvent ( CancelEvent );

		// send message
		account()->client()->sendMessage ( message );

	}

}

void JabberChatSession::slotMessageSent ( Kopete::Message &message, Kopete::ChatSession * )
{

	if( account()->isConnected () )
	{
		XMPP::Message jabberMessage;
		Kopete::Contact *recipient = message.to().first ();

		XMPP::Jid jid ( message.from()->contactId () );
		jid.setResource ( account()->configGroup()->readEntry( "Resource", QString::null ) );
		jabberMessage.setFrom ( jid );

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

		// determine type of the widget and set message type accordingly
		// "kopete_emailwindow" is the default email Kopete::ViewPlugin.  If other email plugins
		// become available, either jabber will have to provide its own selector or libkopete will need
		// a better way of categorising view plugins.
		if ( view()->plugin()->pluginId() == "kopete_emailwindow" )
		{
			jabberMessage.setType ( "normal" );
		}
		else
		{
			jabberMessage.setType ( "chat" );
		}

		// send the message
		account()->client()->sendMessage ( jabberMessage );

		// append the message to the manager
		Kopete::ChatSession::appendMessage ( message );

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

#include "jabberchatsession.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; replace-tabs off; space-indent off;
