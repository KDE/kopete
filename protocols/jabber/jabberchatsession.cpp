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
#include <qlabel.h>
#include <qimage.h>
#include <qtooltip.h>
#include <qfile.h>
#include <qiconset.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include "kopetechatsessionmanager.h"
#include "kopetemessage.h"
#include "kopeteviewplugin.h"
#include "kopeteview.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberclient.h"
#include "jabbercontact.h"
#include "jabberresource.h"
#include "jabberresourcepool.h"
#include "kioslave/jabberdisco.h"


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
	XMPP::Jid jid = user->rosterItem().jid() ;

	mResource = jid.resource().isEmpty () ? resource : jid.resource ();
	slotUpdateDisplayName ();

#ifdef SUPPORT_JINGLE
	KAction *jabber_voicecall = new KAction( i18n("Voice call" ), "voicecall", 0, members().getFirst(), SLOT(voiceCall ()), actionCollection(), "jabber_voicecall" );

	setInstance(protocol->instance());
	jabber_voicecall->setEnabled( false );

	
	Kopete::ContactPtrList chatMembers = members ();
	if ( chatMembers.first () )
	{
		// Check if the current contact support Voice calls, also honour lock by default.
		// FIXME: we should use the active ressource
		JabberResource *bestResource = account()->resourcePool()->  bestJabberResource( static_cast<JabberBaseContact*>(chatMembers.first())->rosterItem().jid() );
		if( bestResource && bestResource->features().canVoice() )
		{
			jabber_voicecall->setEnabled( true );
		}
	}

#endif

 new KAction( i18n( "Send File" ), "attach", 0, this, SLOT( slotSendFile() ), actionCollection(), "jabberSendFile" );

	setXMLFile("jabberchatui.rc");

}

JabberChatSession::~JabberChatSession( )
{
	JabberAccount * a = dynamic_cast<JabberAccount *>(Kopete::ChatSession::account ());
	if( !a ) //When closing kopete, the account is partially destroyed already,  dynamic_cast return 0
		return;
	if ( a->configGroup()->readBoolEntry ("SendEvents", true) &&
			 a->configGroup()->readBoolEntry ("SendGoneEvent", true) )
		sendNotification( XMPP::GoneEvent );
}


void JabberChatSession::slotUpdateDisplayName ()
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << endl;

	Kopete::ContactPtrList chatMembers = members ();

	// make sure we do have members in the chat
	if ( !chatMembers.first () )
		return;

	XMPP::Jid jid = static_cast<JabberBaseContact*>(chatMembers.first())->rosterItem().jid();

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
		{
			sendNotification( XMPP::DeliveredEvent );
		}
		
		if ( account()->configGroup()->readBoolEntry ("SendDisplayedEvent", true) )
		{
			sendNotification( XMPP::DisplayedEvent );
		}
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
			// create JID for the recipient
			XMPP::Jid toJid = contact->rosterItem().jid();
	
			// set resource properly if it has been selected already
			if ( !resource().isEmpty () )
				toJid.setResource ( resource () );
	
			XMPP::Message message;
	
			message.setFrom ( account()->client()->jid() );
			message.setTo ( toJid );
			message.setEventId ( contact->lastReceivedMessageId () );
			// store composing event depending on state
			message.addEvent ( event );
			
			if (view() && view()->plugin()->pluginId() == "kopete_emailwindow" )
			{	
				message.setType ( "normal" );
			}
			else
			{	
				message.setType ( "chat" );
			}

	
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
	XMPP::Jid fromJid = static_cast<const JabberBaseContact*>(myself())->rosterItem().jid();
	fromJid.setResource ( account()->configGroup()->readEntry( "Resource", QString::null ) );

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Sending out typing notification (" << typing << ") to all chat members." << endl;

	typing ? sendNotification( ComposingEvent ) : sendNotification( CancelEvent );
}

void JabberChatSession::slotMessageSent ( Kopete::Message &message, Kopete::ChatSession * )
{

	if( account()->isConnected () )
	{
		XMPP::Message jabberMessage;
		JabberBaseContact *recipient = static_cast<JabberBaseContact*>(message.to().first());

		jabberMessage.setFrom ( account()->client()->jid() );


		XMPP::Jid toJid = recipient->rosterItem().jid();

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
			jabberMessage.setBody ( message.plainBody ());
			if (message.format() ==  Kopete::Message::RichText) 
			{
				JabberResource *bestResource = account()->resourcePool()->bestJabberResource(toJid);
				if( bestResource && bestResource->features().canXHTML() )
				{
					QString xhtmlBody = message.escapedBody();
					
					// According to JEP-0071 8.9  it is only RECOMMANDED to replace \n with <br/>
					//  which mean that some implementation (gaim 2 beta) may still think that \n are linebreak.  
					// and considered the fact that KTextEditor generate a well indented XHTML, we need to remove all \n from it
					//  see Bug 121627
					// Anyway, theses client that do like that are *WRONG*  considreded the example of jep-71 where there are lot of
					// linebreak that are not interpreted.  - Olivier 2006-31-03
					xhtmlBody.replace("\n","");
					
					//&nbsp; is not a valid XML entity
					xhtmlBody.replace("&nbsp;" , "&#160;");
							
					xhtmlBody="<p "+ message.getHtmlStyleAttribute() +">"+ xhtmlBody +"</p>";
					jabberMessage.setXHTMLBody ( xhtmlBody );
				}
        	}
		}

		// determine type of the widget and set message type accordingly
		// "kopete_emailwindow" is the default email Kopete::ViewPlugin.  If other email plugins
		// become available, either jabber will have to provide its own selector or libkopete will need
		// a better way of categorising view plugins.

		// FIXME: the view() is a speedy way to solve BUG:108389. A better solution is to be found
		// but I don't want to introduce a new bug during the bug hunt ;-).
		if (view() && view()->plugin()->pluginId() == "kopete_emailwindow" )
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

 void JabberChatSession::slotSendFile()
      {
              QPtrList<Kopete::Contact>contacts = members();
              static_cast<JabberContact *>(contacts.first())->sendFile();
      }

#include "jabberchatsession.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; replace-tabs off; space-indent off;
