/*
    jabberchatsession.cpp - Jabber Chat Session

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
#include "kopetechatsessionmanager.h"
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

	connect ( this, SIGNAL ( onlineStatusChanged(Kopete::Contact*, const Kopete::OnlineStatus&, const Kopete::OnlineStatus& ) ), this, SLOT ( slotUpdateDisplayName () ) );

	// check if the user ID contains a hardwired resource,
	// we'll have to use that one in that case
	XMPP::Jid jid ( user->contactId () );

	mResource = jid.resource().isEmpty () ? resource : jid.resource ();

	slotUpdateDisplayName ();

}

void JabberChatSession::slotUpdateDisplayName ()
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

	slotUpdateDisplayName ();

	Kopete::ChatSession::appendMessage ( msg );

	// We send the notifications for Delivered and Displayed events. More granular management
	// (ie.: send Displayed event when it is really displayed)
	// of these events would require changes in the chatwindow API.
	
	if ( account()->configGroup()->readBoolEntry ("SendEvents", true) )
	{
		if ( account()->configGroup()->readBoolEntry ("SendDeliveredEvent", true) ) 
			sendNotification( DeliveredEvent );
		if ( account()->configGroup()->readBoolEntry ("SendDisplayedEvent", true) )
			sendNotification( DisplayedEvent );
	}
}

void JabberChatSession::sendNotification( XMPP::MsgEvent event )
{
	if ( !account()->isConnected () )
		return;

	JabberContact *contact;
	QPtrListIterator<Kopete::Contact> listIterator ( members () );

	while ( ( contact = dynamic_cast<JabberContact*>( listIterator.current () ) ) != 0 )
	{
		++listIterator;
		if ( contact->isContactRequestingEvent( event ) )
		{
			// create JID for us as sender
			XMPP::Jid fromJid ( myself()->contactId () );
			fromJid.setResource ( account()->pluginData ( protocol (), "Resource" ) );
	
			// create JID for the recipient
			XMPP::Jid toJid ( contact->contactId () );
	
			// set resource properly if it has been selected already
			if ( !resource().isEmpty () )
				toJid.setResource ( resource () );
	
			XMPP::Message message;
	
			message.setFrom ( fromJid );
			message.setTo ( toJid );
	
			// store composing event depending on state
			message.addEvent ( event );
	
			// send message
			account()->client()->sendMessage ( message );
		}
	}
}

void JabberChatSession::slotSendTypingNotification ( bool typing )
{
	if ( !account()->configGroup()->readBoolEntry ("SendEvents", true)
		|| !account()->configGroup()->readBoolEntry("SendComposingEvent", true) ) 
		return;

	// create JID for us as sender
	XMPP::Jid fromJid ( myself()->contactId () );
	fromJid.setResource ( account()->configGroup()->readEntry( "Resource", QString::null ) );

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Sending out typing notification (" << typing << ") to all chat members." << endl;

	typing ? sendNotification( ComposingEvent ) : sendNotification( CancelEvent );
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

		// add request for all notifications
		jabberMessage.addEvent( OfflineEvent );
		jabberMessage.addEvent( ComposingEvent );
		jabberMessage.addEvent( DeliveredEvent );
		jabberMessage.addEvent( DisplayedEvent );
		

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
