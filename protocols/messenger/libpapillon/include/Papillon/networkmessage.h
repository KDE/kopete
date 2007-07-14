/*
   networkmessage.h - Represent a network message between the Messenger server.

   Copyright (c) 2006-2007 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONNETWORKMESSAGE_H
#define PAPILLONNETWORKMESSAGE_H

// Qt includes
#include <QtCore/QFlags>

// Papillon includes
#include <Papillon/Macros>

class QStringList;
class QByteArray;

namespace Papillon 
{

/**
 * @class NetworkMessage networkmessage.h <Papillon/NetworkMessage>
 * @brief A network message is the data received and transmitted from/to the Messenger server.
 *
 * A NetworkMessage have always a command and arguments.
 * It can have a transaction ID, a payload length, and the payload data.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT NetworkMessage
{
public:
	/**
	 * NetworkMessageTypeValues is the possible type of message
	 * that we can build.
	 */
	enum NetworkMessageTypeValues
	{
		NormalMessage, ///< A basic network message
		TransactionMessage, ///<A network message with a transaction ID attached to it
		PayloadMessage ///<A network message with data attaced to it.
	};
	Q_DECLARE_FLAGS(NetworkMessageType, NetworkMessageTypeValues)
	
	/**
	 * @brief Build a new NetworkMessage
	 *
	 * The default type for NetworkMessage is NormalMessage.
	 *
	 * @param type The type of NetworkMessage. Default is NormalMessage
	 */
	NetworkMessage(const NetworkMessageType &type = NormalMessage);
	~NetworkMessage();

	/**
	 * Return the NetworkMessage type
	 */
	NetworkMessageType type() const;

	/**
	 * Return the command name.
	 */
	QString command() const;
	/**
	 * Set the command name.
	 * @param command The command name. (ex: USR)
	 */
	void setCommand(const QString &command);
	
	/**
	 * Return the transaction ID if any.
	 */
	QString transactionId() const;
	/**
	 * Set the transaction ID.
	 * The transfer must be set to TransactionTransfer type.
	 * @param transactionId the Transaction ID.
	 */
	 void setTransactionId(const QString &transactionId);

	/**
	 * Return the arguments of the command.
	 */
	QStringList arguments() const;
	/**
	 * Set the arugments for the transfer.
	 * @param arguments Argument list.
	 */
	void setArguments(const QStringList &arguments);
	/**
	 * This is a convience method to set arguments to a transfer.
	 *
	 * It take a QString instead of a QStringList.
	 * @param argumentString A string containing the arguments separated by an empty space.
	 */
	void setArguments(const QString &argumentString);

	/**
	 * Return the length of the payload data.
	 */
	int payloadLength() const;
	/**
	 * Return the payload data.
	 */
	QByteArray payloadData() const;
	/**
	 * Set the payload data to be send.
	 * The transfer must be set to PayloadTransfer type.
	 *
	 * @param data payload data to sent.
	 */
	void setPayloadData(const QByteArray &data);
	
	/**
	 * @brief Return the current transfert in a string.
	 * If the transfer is PayloadTransfer type, the payload length is appended at the end of the line,
	 * if not already in the argument list.
	 * 
	 * This is for text-only transfer that doesn't have payload data. You should use asRawCommand() instead.
	 */
	QString toString() const;

	/**
	 * @brief Return the current transfer as a raw command.
	 * This method should be used only when sending transfer with payload data.
	 */
	QByteArray toRawCommand() const;

private:
	class Private;
	Private *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NetworkMessage::NetworkMessageType)

}

#endif
