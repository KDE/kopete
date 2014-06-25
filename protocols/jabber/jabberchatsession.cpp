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

#include <qlabel.h>
#include <qimage.h>

#include <qfile.h>
#include <qicon.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <kaction.h>
#include <kactioncollection.h>
#include "kopetechatsessionmanager.h"
#include "kopetemessage.h"
#include "kopeteviewplugin.h"
#include "kopeteview.h"
#include "kopetemetacontact.h"

#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberclient.h"
#include "jabbercontact.h"
#include "jabberresource.h"
#include "jabberresourcepool.h"
#include "kioslave/jabberdisco.h"


JabberChatSession::JabberChatSession ( JabberProtocol *protocol, const JabberBaseContact *user,
											 Kopete::ContactPtrList others, const QString &resource )
											 : Kopete::ChatSession ( user, others, protocol ),
											 mTypingNotificationSent ( false )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "New message manager for " << user->contactId ();

	setComponentData(protocol->componentData());

	// make sure Kopete knows about this instance
	Kopete::ChatSessionManager::self()->registerChatSession ( this );

	connect ( this, SIGNAL (messageSent(Kopete::Message&,Kopete::ChatSession*)),
			  this, SLOT (slotMessageSent(Kopete::Message&,Kopete::ChatSession*)) );

	connect ( this, SIGNAL (myselfTyping(bool)), this, SLOT (slotSendTypingNotification(bool)) );

	connect ( this, SIGNAL (onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)), this, SLOT (slotUpdateDisplayName()) );

	// check if the user ID contains a hardwired resource,
	// we'll have to use that one in that case
	XMPP::Jid jid = user->rosterItem().jid() ;

	mResource = jid.resource().isEmpty () ? resource : jid.resource ();
	slotUpdateDisplayName ();

#ifdef JINGLE_SUPPORT
	KAction *jingleSessionGui = new KAction(i18n("Show audio calls"), members().first());
	jingleSessionGui->setIcon(KIcon("voicecall"));
	connect(jingleSessionGui, SIGNAL(triggered(bool)), SLOT (slotJingleSessionGui()));
	setComponentData(protocol->componentData());

	KAction *jingleSession = new KAction(i18n("Start audio call"), members().first());
	jingleSession->setIcon(KIcon("voicecall"));
	connect(jingleSession, SIGNAL(triggered(bool)), SLOT (slotJingleSession()));
	setComponentData(protocol->componentData());

	Kopete::ContactPtrList chatMembers = members();
	if (!chatMembers.isEmpty())
	{
		JabberResource *bestResource = account()->resourcePool()->bestJabberResource( static_cast<JabberBaseContact*>(chatMembers.first())->rosterItem().jid() );
		if (bestResource)
		{
			jingleSessionGui->setEnabled( bestResource->features().canJingle() );
			jingleSession->setEnabled( bestResource->features().canJingle() );
		}
		else
		{
			jingleSessionGui->setEnabled(false);
			jingleSession->setEnabled(false);
		}
	}


	/*KAction *jingleaudiocall = new KAction(i18n("Jingle Audio call" ), members().first());
	connect(jingleaudiocall, SIGNAL(triggered(bool)), SLOT (slotJingleAudioCall()));
	setComponentData(protocol->componentData());
	jingleaudiocall->setEnabled( false );

	KAction *jinglevideocall = new KAction(i18n("Jingle Video call" ), members().first());
	connect(jinglevideocall, SIGNAL(triggered(bool)), SLOT (slotJingleVideoCall()));
	setComponentData(protocol->componentData());
	jinglevideocall->setEnabled( false );*/

	//Kopete::ContactPtrList chatMembers = members ();
	//if ( chatMembers.first () )
	//{
		// Check if the current contact support Audio calls, also honor lock by default.
		// FIXME: we should use the active resource
		//jingleaudiocall->setEnabled( bestResource->features().canJingleAudio() );
		//jinglevideocall->setEnabled( bestResource->features().canJingleVideo() );
	//}

	//FIXME : Toolbar does not show any action (either for MSN or XMPP)
	//	  It should be corrected in trunk.
	//actionCollection()->addAction( "jabberJingleaudiocall", jingleaudiocall );
	//actionCollection()->addAction( "jabberJinglevideocall", jinglevideocall );
	actionCollection()->addAction( "jingleSession", jingleSession );
	actionCollection()->addAction( "jingleSessionsGui", jingleSessionGui );

#endif

	setXMLFile("jabberchatui.rc");

}

JabberChatSession::~JabberChatSession( )
{
	JabberAccount * a = dynamic_cast<JabberAccount *>(Kopete::ChatSession::account ());
	if( !a ) //When closing kopete, the account is partially destroyed already,  dynamic_cast return 0
		return;
	if ( a->configGroup()->readEntry ("SendEvents", true) &&
			 a->configGroup()->readEntry ("SendGoneEvent", true) )
		sendNotification( Gone );
}


void JabberChatSession::slotUpdateDisplayName ()
{
	kDebug ( JABBER_DEBUG_GLOBAL ) ;

	Kopete::ContactPtrList chatMembers = members ();

	// make sure we do have members in the chat
	if ( chatMembers.isEmpty() )
		return;

	XMPP::Jid jid = static_cast<JabberBaseContact*>(chatMembers.first())->rosterItem().jid();

	if ( !mResource.isEmpty () )
		jid = jid.withResource ( mResource );

	QString statusText = i18nc("a contact's online status in parenthesis.", " (%1)",
							  chatMembers.first()->onlineStatus().description() );
	if ( jid.resource().isEmpty () )
		setDisplayName ( chatMembers.first()->metaContact()->displayName () + statusText );
	else
		setDisplayName ( chatMembers.first()->metaContact()->displayName () + '/' + jid.resource () + statusText );

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

	if ( account()->configGroup()->readEntry ("SendEvents", true) )
	{
		if ( account()->configGroup()->readEntry ("SendDeliveredEvent", true) )
		{
			sendNotification( Delivered );
		}

		if ( account()->configGroup()->readEntry ("SendDisplayedEvent", true) )
		{
			sendNotification( Displayed );
		}
	}
}

void JabberChatSession::sendNotification( Event event )
{
	if ( !account()->isConnected () )
		return;

	XMPP::MsgEvent msg_event;
	XMPP::ChatState new_state;
	bool send_msg_event=false;
	bool send_msg_delivery=false;
	bool send_state=false;

	switch(event)
	{
		case Delivered:
			send_msg_event=true;
			send_msg_delivery=true;
			msg_event=DeliveredEvent;
			break;
		case Displayed:
			send_msg_event=true;
			msg_event=DisplayedEvent;
			break;
		case Composing:
			send_msg_event=true;
			msg_event=ComposingEvent;
			send_state=true;
			new_state=XMPP::StateComposing;
			break;
		case CancelComposing:
			send_msg_event=true;
			msg_event=CancelEvent;
			send_state=true;
			//new_state=XMPP::StatePaused; //paused give some bad notification on some client. while this mean in kopete that we stopped typing.
			new_state=XMPP::StateActive;
			break;
		case Inactive:
			send_state=true;
			new_state=XMPP::StateInactive;
			break;
		case Gone:
			send_state=true;
			new_state=XMPP::StateGone;
			break;
		default:
			break;
	}

	if(send_msg_event)
	{
		send_msg_event=false;
		foreach(Kopete::Contact *c , members())
		{
			JabberBaseContact *contact = static_cast<JabberBaseContact*>(c);
			if(contact->isContactRequestingEvent( msg_event ))
			{
				send_msg_event=true;
				break;
			}
		}
	}
	if(send_msg_delivery)
	{
		send_msg_delivery=false;
		foreach(Kopete::Contact *c, members())
		{
			JabberBaseContact *contact = static_cast<JabberBaseContact*>(c);
			if(contact->isContactRequestingReceiptDelivery())
			{
				send_msg_delivery=true;
				break;
			}
		}
	}
/*	if(send_state)
	{
		send_state=false;
		foreach(JabberContact *contact , members())
		{
			JabberContact *c;
			if(contact->feature.canChatState()  )
			{
				send_state=true;
				break;
			}
		}
	}*/

	if( !members().isEmpty() && (send_state || send_msg_event || send_msg_delivery) )
	{
		// create JID for the recipient
		JabberBaseContact *recipient = static_cast<JabberBaseContact*>(members().first());
		XMPP::Jid toJid = recipient->rosterItem().jid();
		const QString &lastReceivedMessageId = recipient->lastReceivedMessageId ();

		// set resource properly if it has been selected already
		if ( !resource().isEmpty () )
			toJid = toJid.withResource ( resource () );

		if (send_state || send_msg_delivery)
		{

			XMPP::Message message;

			message.setTo ( toJid );
			if(send_msg_event)
			{
				message.setEventId ( lastReceivedMessageId );
				// store composing event depending on state
				message.addEvent ( msg_event );
			}
			if(send_state)
			{
				message.setChatState( new_state );
			}

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

		// XEP-0184: Message Delivery Receipts (same as DeliveredEvent)
		if (send_msg_delivery)
		{
			XMPP::Message message;
			message.setTo ( toJid );
			message.setMessageReceipt ( ReceiptReceived );
			message.setMessageReceiptId ( lastReceivedMessageId );
			account()->client()->sendMessage ( message );
		}
	}
}

void JabberChatSession::slotSendTypingNotification ( bool typing )
{
	if ( !account()->configGroup()->readEntry ("SendEvents", true)
		|| !account()->configGroup()->readEntry("SendComposingEvent", true) )
		return;

	if ( typing && mTypingNotificationSent ) {
		return;
	}

	// send only one Composing notification; CancelComposing and slotMessageSent reset this
	mTypingNotificationSent = typing;

	kDebug ( JABBER_DEBUG_GLOBAL ) << "Sending out typing notification (" << typing << ") to all chat members.";

	typing ? sendNotification( Composing ) : sendNotification( CancelComposing );
}

void JabberChatSession::slotMessageSent ( Kopete::Message &message, Kopete::ChatSession * )
{

	if( account()->isConnected () )
	{
		XMPP::Message jabberMessage;
		JabberBaseContact *recipient = static_cast<JabberBaseContact*>(message.to().first());

		XMPP::Jid toJid = recipient->rosterItem().jid();

		if( !resource().isEmpty () )
			toJid = toJid.withResource ( resource() );

		jabberMessage.setTo ( toJid );

		jabberMessage.setId( QString::number( message.id() ) );
		jabberMessage.setSubject ( message.subject () );
		jabberMessage.setTimeStamp ( message.timestamp () );

		JabberResource *jresource = account()->resourcePool()->getJabberResource(toJid, resource());

		bool xsigned = message.classes().contains ( "signed" );
		bool xencrypted = message.classes().contains ( "encrypted" );

		bool fxsigned = jresource && jresource->features().test ( QStringList ( "jabber:x:signed" ) );
		bool fxencrypted = jresource && jresource->features().test ( QStringList ( "jabber:x:encrypted" ) );

		if ( ( ( xsigned && fxsigned ) || ( xencrypted && fxencrypted ) || ! account()->oldEncrypted() ) && message.plainBody().indexOf ( "-----BEGIN PGP MESSAGE-----" ) != -1 )
		{
			/*
			 * This message is encrypted, so we need to set
			 * a fake body indicating that this is an encrypted
			 * message (for clients not implementing this
			 * functionality) and then generate the encrypted
			 * payload out of the old message body.
			 */

			// please don't translate the following string
			if ( xsigned && xencrypted )
				jabberMessage.setBody ( "This message is signed and encrypted." );
			else if ( xsigned )
				jabberMessage.setBody ( message.plainBody().trimmed() );
			else if ( xencrypted )
				jabberMessage.setBody ( "This message is encrypted." );
			else
				jabberMessage.setBody ( "This message is signed or encrypted." );

			QString encryptedBody = message.plainBody().trimmed();

			// remove PGP header and footer from message
			encryptedBody.truncate ( encryptedBody.length () - QString("-----END PGP MESSAGE-----").length () - 2 );
			encryptedBody = encryptedBody.right ( encryptedBody.length () - encryptedBody.indexOf ( "\n\n" ) - 2 );

			// assign payload to message
			if ( xsigned && ! xencrypted )
				jabberMessage.setXSigned ( encryptedBody );
			else
				jabberMessage.setXEncrypted ( encryptedBody );
		}
		else
		{
			// this message is not encrypted
			jabberMessage.setBody ( message.plainBody ());
			if (message.format() ==  Qt::RichText)
			{
				JabberResource *bestResource = account()->resourcePool()->bestJabberResource(toJid);
				if( bestResource && bestResource->features().test(
						QStringList("http://jabber.org/protocol/xhtml-im")) )
				{
					QString xhtmlBody = message.escapedBody();

					// According to JEP-0071 8.9  it is only RECOMMENDED to replace \n with <br/>
					//  which mean that some implementation (gaim 2 beta) may still think that \n are linebreak.
					// and considered the fact that KTextEditor generate a well indented XHTML, we need to remove all \n from it
					//  see Bug 121627
					// Anyway, theses client that do like that are *WRONG*  considreded the example of jep-71 where there are lot of
					// linebreak that are not interpreted.  - Olivier 2006-31-03
					xhtmlBody.remove('\n');

					//&nbsp; is not a valid XML entity
					xhtmlBody.replace("&nbsp;" , "&#160;");

					xhtmlBody="<body xmlns=\"http://www.w3.org/1999/xhtml\">" + xhtmlBody + "</body>";

					QDomDocument doc;
					doc.setContent(xhtmlBody, true);
					jabberMessage.setHTML( XMPP::HTMLElement( doc.documentElement() ) );
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
		jabberMessage.setChatState( XMPP::StateActive );

		// XEP-0184: Message Delivery Receipts
		if( jresource && jresource->features().test(QStringList("urn:xmpp:receipts")) )
		{
			jabberMessage.setMessageReceipt( ReceiptRequest );
		}

		// send the message
		account()->client()->sendMessage ( jabberMessage );

		if ( recipient->sendsDeliveredEvent() )
			message.setState( Kopete::Message::StateSending );

		// append the message to the manager
		Kopete::ChatSession::appendMessage ( message );

		// tell the manager that we sent successfully
		messageSucceeded ();

		mTypingNotificationSent = false;
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
	QList<Kopete::Contact*>contacts = members();
	contacts.first()->sendFile();
}


#ifdef JINGLE_SUPPORT
void JabberChatSession::slotJingleAudioCall()
{
	QList<Kopete::Contact*> contacts = members();
	static_cast<JabberContact *>(contacts.first())->startJingleAudioCall();
}

void JabberChatSession::slotJingleVideoCall()
{
	QList<Kopete::Contact*> contacts = members();
	static_cast<JabberContact *>(contacts.first())->startJingleVideoCall();
}

void JabberChatSession::slotJingleSessionGui()
{
	QList<Kopete::Contact*> contacts = members();
	static_cast<JabberContact *>(contacts.first())->showSessionsGui();
}

void JabberChatSession::slotJingleSession()
{
	QList<Kopete::Contact*> contacts = members();
	static_cast<JabberContact *>(contacts.first())->startJingleSession();
}

#endif

#include "jabberchatsession.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; replace-tabs off; space-indent off;
