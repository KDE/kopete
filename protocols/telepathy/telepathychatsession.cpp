/*
 * telepathychatsession.cpp - Telepathy Chat Session.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#include "telepathychatsession.h"

// Qt includes
#include <QtCore/QPointer>

// KDE includes
#include <kdebug.h>
#include <klocale.h>

// Kopete includes
#include <kopetechatsessionmanager.h>

// QtTapioca includes
#include <QtTapioca/Contact>

// Local includes
#include "telepathyprotocol.h"
#include "telepathyaccount.h"
#include "telepathycontact.h"

using namespace QtTapioca;

class TelepathyChatSession::Private
{
public:
	QPointer<QtTapioca::TextChannel> textChannel;
};

TelepathyChatSession::TelepathyChatSession(const Kopete::Contact *user, Kopete::ContactPtrList others, Kopete::Protocol *protocol)
 : Kopete::ChatSession(user, others, protocol), d(new Private)
{
	Kopete::ChatSessionManager::self()->registerChatSession(this);

	connect(this, SIGNAL(messageSent(Kopete::Message&, Kopete::ChatSession*)), this, SLOT(sendMessage(Kopete::Message&)));
}

TelepathyChatSession::~TelepathyChatSession()
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo;

	// End text channel session
	d->textChannel->close();

	delete d;
}

QtTapioca::TextChannel* TelepathyChatSession::textChannel()
{
	Q_ASSERT( !d->textChannel.isNull() );

	return d->textChannel;
}

void TelepathyChatSession::setTextChannel(QtTapioca::TextChannel *textChannel)
{
	// Disconnect previous signals connection
	if( !d->textChannel.isNull() )
	{
		d->textChannel->disconnect();
	}

	d->textChannel = textChannel;

	// Connect signal/slots
	connect(d->textChannel, SIGNAL(messageReceived(QtTapioca::TextChannel::Message)), this, SLOT(telepathyMessageReceived(QtTapioca::TextChannel::Message)));

	connect(d->textChannel, SIGNAL(messageDeliveryError(QtTapioca::TextChannel::Message, QtTapioca::TextChannel::Message::DeliveryError)), this, SLOT(telepathyMessageDeliveryError(QtTapioca::TextChannel::Message, QtTapioca::TextChannel::Message::DeliveryError)));

	connect(d->textChannel, SIGNAL(messageSent(QtTapioca::TextChannel::Message)), this, SLOT(telepathyMessageSent(QtTapioca::TextChannel::Message)));
}

void TelepathyChatSession::telepathyMessageReceived(const QtTapioca::TextChannel::Message &message)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo;

	// Create a new Kopete::Message
	Kopete::Message::MessageType messageType = Kopete::Message::TypeNormal;
	
	if( message.type() == QtTapioca::TextChannel::Message::Action )
	{
		messageType = Kopete::Message::TypeAction;
	}

	Kopete::Message newMessage( members().first(), myself() );
	newMessage.setPlainBody( message.contents() );
	newMessage.setDirection( Kopete::Message::Inbound );
	newMessage.setType( messageType );

	appendMessage( newMessage );
}

void TelepathyChatSession::telepathyMessageSent(const QtTapioca::TextChannel::Message &message)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Message contents: " << message.contents();

	Kopete::Message::MessageType messageType = Kopete::Message::TypeNormal;
	
	if( message.type() == QtTapioca::TextChannel::Message::Action )
	{
		messageType = Kopete::Message::TypeAction;
	}

	Kopete::Message newMessage( myself(), members() );
	newMessage.setPlainBody( message.contents() );
	newMessage.setDirection( Kopete::Message::Outbound );
	newMessage.setType( messageType );

	// Append successfully sent message to chat window and notify other components of success
	appendMessage( newMessage );
	messageSucceeded();
}

void TelepathyChatSession::telepathyMessageDeliveryError(const QtTapioca::TextChannel::Message &message, QtTapioca::TextChannel::Message::DeliveryError error)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo;
	QString internalErrorMessage, errorMessageText;
	switch(error)
	{
		case TextChannel::Message::ContactOffline:
			internalErrorMessage = i18n("Contact is offline.");
			break;
		case TextChannel::Message::InvalidContact:
			internalErrorMessage = i18n("Contact is invalid.");
			break;
		case TextChannel::Message::PermissionDenied:
			internalErrorMessage = i18n("You do not have enough permission to send a message to this contact.");
			break;
		case TextChannel::Message::MessageTooLong:
			internalErrorMessage = i18n("Message is too long.");
			break;
		case TextChannel::Message::Unknown:
			internalErrorMessage = i18n("Unknown reason");
			break;
	}

	// The following message:
	// "test
	//  testsfaefe"
	// could not be delivered. Reason: Contact is offline.
	errorMessageText = i18n("The following message:\n \"%1\"\ncould not be delivered. Reason: %2", message.contents(), internalErrorMessage);

	Kopete::Message errorMessage( myself(), members() );
	errorMessage.setPlainBody( errorMessageText );
	errorMessage.setDirection( Kopete::Message::Internal );

	appendMessage( errorMessage );
}

void TelepathyChatSession::sendMessage(Kopete::Message &message)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Sending: " << message.plainBody();

	// TODO: Support other type of message (when QtTapioca will support it)
	QtTapioca::TextChannel::Message messageSend( message.plainBody() );
	
	textChannel()->sendMessage( messageSend );
}

#include "telepathychatsession.moc"
