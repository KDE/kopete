/*
   messengercoreprotocol.h - Messenger core protocol for Papillon 

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#ifndef MESSANGERCOREPROTOCOL_H
#define MESSANGERCOREPROTOCOL_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>

#include <Papillon/Macros>

class QDataStream;

namespace Papillon
{

class NetworkMessage;

/**
 * @class MessengerCoreProtocol messengercoreprotocol.h <Papillon/MessengerCoreProtocol>
 * @brief Translate raw data into NetworkMessage
 * The sole purpose of this class is to parse in a low-level way the Messenger protocol.
 * it create NetworkMessage that Task will be able to use.
 *
 * @author Michaël Larouche <larouche@kde.org>
 * @author SuSE Linux AG
 * @author Justin Karneges <justin@affinix.com>
 */
class PAPILLON_EXPORT MessengerCoreProtocol : public QObject
{
	Q_OBJECT
public:
	/**
	 * State is used internally to tell where we are in the parsing.
	 */
	enum State 
	{
		/**
		 * Need more data to proceed.
		 */
		NeedMore,
		/**
		 * NetworkMessage is ready.
		 */
		Available,
		/**
		 * No data is currently in for proceeding.
		 */
		NoData,
		/**
		 * We parsed a payload command and we are waiting for payload data.
		 */
		WaitForPayload
	};

	/**
	 * @brief Create an new parser instance.
	 */
	MessengerCoreProtocol();
	/**
	 * d-tor
	 */
	virtual ~MessengerCoreProtocol();

	/**
	 * @brief Reset the protocol, clear buffers
	 */
	void reset();

	/**
	 * @brief Add data to be proceeded by this class.
	 * Accept data from the network, and buffer it into a useful message
	 * This requires parsing out each packet from the incoming data
	 * @param incomingBytes Raw data
	 */
	void addIncomingData(const QByteArray &incomingBytes);

	/**
	 * @brief Get the current NetworkMessage available.
	 * Use incomingData() to tell when get the incoming NetworkMessage.
	 * @return the incoming NetworkMessage or 0 if none is available.
	 */
	NetworkMessage *incomingNetworkMessage();

	/**
	 * @brief Convert a request into an outgoing networkMessage
	 * Emit outgoingData() with the raw networkMessage.
	 * @param outgoing NetworkMessage
	 */
	void outgoingNetworkMessage(NetworkMessage *outgoing);

	/**
	 * @brief Get the state of the protocol
	 * @return Current state
	 * @see State
	 */
	int state();

signals:
	/**
	 * Emitted as the core protocol converts fields to wire ready data
	 * @param data Outgoing raw data
	 */
	void outgoingData(const QByteArray &data);

	/**
	 * Emitted when there is incoming data, parsed into a NetworkMessage
	 */
	void incomingData();

protected:
	/**
	 * Check that there is data to read, and set the protocol's state if there isn't any.
	 * @param din Currently used QDataStream
	 */
	bool okToProceed(const QDataStream &din);
	/**
	 * Convert incoming raw data into a NetworkMessage object and queue it
	 * @param raw raw data
	 * @return number of bytes from the input that were parsed into a NetworkMessage
	 */
	int rawToNetworkMessage(const QByteArray &raw);

	/**
	 * Check if the command is a payload command.
	 * @param command command to check.
	 */
	bool isPayloadCommand(const QString &command);

private:
	class Private;
	Private *d;
};

}

#endif // MESSANGERCOREPROTOCOL_H
