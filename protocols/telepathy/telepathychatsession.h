/*
 * telepathychatsession.h - Telepathy Chat Session.
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
#ifndef TELEPATHY_CHATSESSION_H
#define TELEPATHY_CHATSESSION_H

#include <kopetechatsession.h>

#include <QtTapioca/TextChannel>

namespace Kopete
{
	class Contact;
	class Protocol;
}

class TelepathyAccount;

/**
 * @brief Managa a single text chat session between 2 peers.
 * @author Michaël Larouche <larouche@kde.org>
 */
class TelepathyChatSession : public Kopete::ChatSession
{
	Q_OBJECT
public:
	TelepathyChatSession(const Kopete::Contact *user, Kopete::ContactPtrList others, Kopete::Protocol *protocol);
	~TelepathyChatSession();

	/**
	 * @brief Set the required text channel from QtTapioca
	 * @param textChannel Text channel from QtTapioca
	 */
	void setTextChannel(QtTapioca::TextChannel *textChannel);

private slots:
	/**
	 * @brief Received a message from the contact
	 * @param message Actual message from the contact
	 */
	void telepathyMessageReceived(const QtTapioca::TextChannel::Message &message);

	/**
	 * @brief Message has been sent sucessfully
	 * @param message Message succesfully sent.
	 */
	void telepathyMessageSent(const QtTapioca::TextChannel::Message &message);

	/**
	 * @brief Message delivery has failed
	 * @param message Message who has failed to be deliveried.
	 * @param error Error code
	 */
	void telepathyMessageDeliveryError(const QtTapioca::TextChannel::Message &message, QtTapioca::TextChannel::Message::DeliveryError error);

	/**
	 * @brief Send the following message
	 * @param message Kopete::Message to send.
	 */
	void sendMessage(Kopete::Message &message);

private:
	/**
	 * @brief Get the current instance of TextChannel.
	 */
	QtTapioca::TextChannel *textChannel();

private:
	class Private;
	Private *d;
};

#endif
