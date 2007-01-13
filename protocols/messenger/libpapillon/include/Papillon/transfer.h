/*
   transfer.h - Represent a transfer between the Messenger server.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONTRANSFER_H
#define PAPILLONTRANSFER_H

// Qt includes
#include <QtCore/QFlags>

// Papillon includes
#include <Papillon/Macros>

class QStringList;
class QByteArray;

namespace Papillon 
{

/**
 * @class Transfer transfer.h <Papillon/Transfer>
 * @brief A transfer represent a information from/to the protocol. This can be a command or a message.
 *
 * A transfer have always a command and arguments. It can have a transaction ID, a payload length, and the payload data.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT Transfer
{
public:
	/**
	 * - NormalTransfer: A normal transfer without transaction id neither payload.
	 * - TransactionTransfer: The transfer has a transaction ID.
	 * - PayloadTransfer: The transfer contains payload data.
	 */
	enum TransferTypeValues
	{
		NormalTransfer,
		TransactionTransfer,
		PayloadTransfer
	};
	Q_DECLARE_FLAGS(TransferType, TransferTypeValues)
	
	/**
	 * @brief Build a new transfer
	 * By default, the transfert is set to NormalTransfer type.
	 *
	 * @param type Type of the transfer. See TransferTypeValues for possibled values.
	 */
	Transfer(const TransferType &type = NormalTransfer);
	~Transfer();

	/**
	 * Return the transfer type.
	 */
	TransferType type() const;

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

Q_DECLARE_OPERATORS_FOR_FLAGS(Transfer::TransferType)

}

#endif
