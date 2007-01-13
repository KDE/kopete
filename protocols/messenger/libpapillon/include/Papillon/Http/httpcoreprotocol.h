/*
   httpcoreprotocol.h - HTTP core protocol for Papillon 

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003  Justin Karneges

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#ifndef PAPILLON_HTTPCOREPROTOCOL_H
#define PAPILLON_HTTPCOREPROTOCOL_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>

#include <Papillon/Macros>

class QDataStream;

namespace Papillon
{

class HttpTransfer;

/**
 * @class HttpCoreProtocol httpcoreprotocol.h <Papillon/Http/CoreProtocol>
 * @brief Translate raw data into HttpTransfer
 * The sole purpose of this class is to parse in a low-level way the HTTP protocol.
 * it create a HttpTransfer that will be useable afterwards.
 *
 * @author Michaël Larouche <larouche@kde.org>
 * @author SuSE Linux AG
 * @author Justin Karneges
 */
class PAPILLON_EXPORT HttpCoreProtocol : public QObject
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
		 * Transfer is ready.
		 */
		Available,
		/**
		 * No data is currently in for proceeding.
		 */
		NoData,
		/**
		 * Wait for content to proceed.
		 */
		WaitForContent
	};

	/**
	 * @brief Create an new parser instance.
	 */
	HttpCoreProtocol();
	/**
	 * d-tor
	 */
	virtual ~HttpCoreProtocol();

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
	 * @brief Get the current Transfer available.
	 * Use incomingData() to tell when get the incoming Transfer.
	 * @return the incoming Transfer or 0 if none is available.
	 */
	HttpTransfer *incomingTransfer();

	/**
	 * @brief Convert a request into an outgoing transfer
	 * Emit outgoingData() with the raw transfer.
	 * @param outgoing Transfer
	 */
	void outgoingTransfer(HttpTransfer *outgoing);

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
	 * Emitted when there is incoming data, parsed into a Transfer
	 */
	void incomingData();

protected:
	/**
	 * Check that there is data to read, and set the protocol's state if there isn't any.
	 * @param din Currently used QDataStream
	 */
	bool okToProceed(const QDataStream &din);
	/**
	 * Convert incoming raw data into a Transfer object and queue it
	 * @param raw raw data
	 * @return number of bytes from the input that were parsed into a Transfer
	 */
	int rawToTransfer(const QByteArray &raw);

private:
	class Private;
	Private *d;
};

}

#endif
