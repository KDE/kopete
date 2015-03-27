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
#include "jabberchatsession.h"
#include "jabbercontactpool.h"
#include "kopetemetacontact.h"

/**
 * JabberGroupMemberContact constructor
 */
JabberGroupMemberContact::JabberGroupMemberContact (const XMPP::RosterItem &rosterItem,
													JabberAccount *account, Kopete::MetaContact * mc)
													: JabberBaseContact ( rosterItem, account, mc)
{

	mc->setDisplayName ( rosterItem.jid().resource() );
	setNickName ( rosterItem.jid().resource() );

	setFileCapable ( true );

	mManager = 0;

	mRequestOfflineEvent = false;
	mRequestDisplayedEvent = false;
	mRequestDeliveredEvent = false;
	mRequestComposingEvent = false;

	mRequestReceiptDelivery = false;
}

/**
 * JabberGroupMemberContact destructor
 */
JabberGroupMemberContact::~JabberGroupMemberContact ()
{
	if(mManager)
	{
		mManager->deleteLater();
	}
}

QList<KAction*> *JabberGroupMemberContact::customContextMenuActions ()
{

	return 0;

}

Kopete::ChatSession *JabberGroupMemberContact::manager ( Kopete::Contact::CanCreateFlags canCreate )
{

	if ( mManager )
		return mManager;

	if ( !mManager && !canCreate )
		return 0;

	Kopete::ContactPtrList chatMembers;
	chatMembers.append ( this );

	/*
	 * FIXME: We might have to use the groupchat contact here instead of
	 *        the global myself() instance for a correct representation.
	 */
	mManager = new JabberChatSession ( protocol(), static_cast<JabberBaseContact *>(account()->myself()), chatMembers );
	connect ( mManager, SIGNAL (destroyed(QObject*)), this, SLOT (slotChatSessionDeleted()) );

	return mManager;

}

void JabberGroupMemberContact::slotChatSessionDeleted ()
{

	mManager = 0;

}

void JabberGroupMemberContact::handleIncomingMessage ( const XMPP::Message &message )
{
	// message type is always chat in a groupchat
	QString viewType = "kopete_chatwindow";
	Kopete::Message *newMessage = 0L;

	kDebug (JABBER_DEBUG_GLOBAL) << "Received Message Type:" << message.type ();

	Kopete::ChatSession *kmm = manager( Kopete::Contact::CanCreate );
	if(!kmm)
		return;

	if ( message.type () != "error" )
	{
		if (!message.invite().isEmpty())
		{
			/*QString room=message.invite();
			QString originalBody=message.body().isEmpty() ? QString() :
				i18n( "The original message is : <i>\" %1 \"</i><br />" , Qt::escape(message.body()));
			QString mes=i18n("<qt><i>%1</i> has invited you to join the conference <b>%2</b><br />%3<br />"
			                 "If you want to accept and join, just <b>enter your nickname</b> and press OK.<br />"
			                 "If you want to decline, press Cancel.</qt>",
			                 message.from().full(), room , originalBody);
			
			bool ok=false;
			QString futureNewNickName = KInputDialog::getText( i18n( "Invited to a conference - Jabber Plugin" ),
				mes, QString() , &ok , (mManager ? dynamic_cast<QWidget*>(mManager->view(false)) : 0) );
			if ( !ok || !account()->isConnected() || futureNewNickName.isEmpty() )
				return;
			
			XMPP::Jid roomjid(room);
			account()->client()->joinGroupChat( roomjid.domain() , roomjid.node() , futureNewNickName );*/
			return;
		}
		else if (message.body().isEmpty())
		// Then here could be event notifications
		{
			if (message.containsEvent ( XMPP::CancelEvent ) || (message.chatState() != XMPP::StateNone && message.chatState() != XMPP::StateComposing) )
				mManager->receivedTypingMsg ( this, false );
			else if (message.containsEvent ( XMPP::ComposingEvent )|| message.chatState() == XMPP::StateComposing )
				mManager->receivedTypingMsg ( this, true );

			if (message.containsEvent ( XMPP::DisplayedEvent ) )
			{
				//mManager->receivedEventNotification ( i18n("Message has been displayed") );
			}
			else if (message.containsEvent ( XMPP::DeliveredEvent ) )
			{
				//mManager->receivedEventNotification ( i18n("Message has been delivered") );
				mManager->receivedMessageState( message.eventId().toUInt(), Kopete::Message::StateSent );
				mSendsDeliveredEvent = true;
			}
			else if (message.containsEvent ( XMPP::OfflineEvent ) )
			{
				//mManager->receivedEventNotification( i18n("Message stored on the server, contact offline") );
				mManager->receivedMessageState( message.eventId().toUInt(), Kopete::Message::StateSent );
			}
			else if (message.chatState() == XMPP::StateGone )
			{
				/*if(mManager->view( Kopete::Contact::CannotCreate ))
				{   //show an internal message if the user has not already closed his window
					Kopete::Message m=Kopete::Message ( this, mManager->members() );
					m.setPlainBody( i18n("%1 has ended his/her participation in the chat session.", metaContact()->displayName()) );
					m.setDirection( Kopete::Message::Internal );
					m.setImportance(Kopete::Message::Low);
					
					mManager->appendMessage ( m, message.from().resource () );
				}*/
			}

			// XEP-0184: Message Delivery Receipts
			if ( message.messageReceipt() == ReceiptReceived )
			{
				//mManager->receivedEventNotification ( i18n("Message has been delivered") );
				mManager->receivedMessageState( message.messageReceiptId().toUInt(), Kopete::Message::StateSent );
				mSendsDeliveredEvent = true;
			}
		}
		else
		// Then here could be event notification requests
		{
			mRequestComposingEvent = message.containsEvent ( XMPP::ComposingEvent );
			mRequestOfflineEvent = message.containsEvent ( XMPP::OfflineEvent );
			mRequestDeliveredEvent = message.containsEvent ( XMPP::DeliveredEvent );
			mRequestDisplayedEvent = message.containsEvent ( XMPP::DisplayedEvent);

			// XEP-0184: Message Delivery Receipts
			mRequestReceiptDelivery = ( message.messageReceipt() == ReceiptRequest );
		}
	}

	/**
	 * Don't display empty messages, these were most likely just carrying
	 * event notifications or other payload.
	 */
	if ( message.body().isEmpty () )
		return;
	
	Kopete::ContactPtrList contactList = kmm->members();

	// check for errors
	if ( message.type () == "error" )
	{
		newMessage = new Kopete::Message( this, contactList );
		newMessage->setTimestamp( message.timeStamp() );
		newMessage->setPlainBody( i18n("Your message could not be delivered: \"%1\", Reason: \"%2\"",
										  message.body (), message.error().text ) );
		newMessage->setSubject( message.subject() );
		newMessage->setDirection( Kopete::Message::Inbound );
		newMessage->setRequestedPlugin( viewType );
	}
	else
	{
		// store message id for outgoing notifications
		mLastReceivedMessageId = message.id ();
		
		// retrieve and reformat body
		QString body = message.body ();

		if( !message.xencrypted().isEmpty () )
		{
			body = QString ("-----BEGIN PGP MESSAGE-----\n\n") + message.xencrypted () + QString ("\n-----END PGP MESSAGE-----\n");
		}
		else if( !message.xsigned().isEmpty () )
		{
			body = QString ("-----BEGIN PGP MESSAGE-----\n\n") + message.xsigned () + QString ("\n-----END PGP MESSAGE-----\n");
		}

		// convert XMPP::Message into Kopete::Message
		if( message.containsHTML() )
		{
			kDebug ( JABBER_DEBUG_GLOBAL ) << "Received a xHTML message";
			newMessage = new Kopete::Message ( this, contactList );
			newMessage->setTimestamp( message.timeStamp() );
			newMessage->setHtmlBody( message.html().toString() );
			newMessage->setDirection( Kopete::Message::Inbound );
			newMessage->setSubject( message.subject() );
			newMessage->setRequestedPlugin( viewType );
			newMessage->setImportance( Kopete::Message::Low );
		}
		else if ( !body.isEmpty () )
		{
			kDebug ( JABBER_DEBUG_GLOBAL ) << "Received a plain text message";
			newMessage = new Kopete::Message ( this, contactList );
			newMessage->setTimestamp( message.timeStamp() );
			newMessage->setPlainBody( body );
			newMessage->setDirection( Kopete::Message::Inbound );
			newMessage->setSubject( message.subject() );
			newMessage->setRequestedPlugin( viewType );
			newMessage->setImportance( Kopete::Message::Low );
		}
	}

	// append message to manager
	if ( newMessage ) {
		kmm->appendMessage ( *newMessage );

		delete newMessage;
	}

}

bool JabberGroupMemberContact::isContactRequestingEvent( XMPP::MsgEvent event )
{
	if ( event == OfflineEvent )
		return mRequestOfflineEvent;
	else if ( event == DeliveredEvent )
		return mRequestDeliveredEvent;
	else if ( event == DisplayedEvent )
		return mRequestDisplayedEvent;
	else if ( event == ComposingEvent )
		return mRequestComposingEvent;
	else if ( event == CancelEvent )
		return mRequestComposingEvent;
	else
		return false;
}

bool JabberGroupMemberContact::isContactRequestingReceiptDelivery()
{
	return mRequestReceiptDelivery;
}

QString JabberGroupMemberContact::lastReceivedMessageId () const
{
	return mLastReceivedMessageId;
}

void JabberGroupMemberContact::sendFile ( const KUrl &sourceURL, const QString &/*fileName*/, uint /*fileSize*/ )
{
	QString filePath;

	// if the file location is null, then get it from a file open dialog
	if ( !sourceURL.isValid () )
		filePath = KFileDialog::getOpenFileName( KUrl(), "*", 0L, i18n ( "Kopete File Transfer" ) );
	else
		filePath = sourceURL.path(KUrl::RemoveTrailingSlash);

	QFile file ( filePath );

	if ( file.exists () )
	{
		// send the file
		new JabberFileTransfer ( account (), this, filePath );
	}

}


#include "jabbergroupmembercontact.moc"
