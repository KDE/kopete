/*
   coreprotocol.h - Messenger core protocol for Papillon 

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

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

#ifndef PAPILLON_CORE_PROTOCOL_H
#define PAPILLON_CORE_PROTOCOL_H

#include <QObject>
#include <QByteArray>

#include <papillon_macros.h>

class QDataStream;

namespace Papillon
{

class Transfer;

class PAPILLON_EXPORT CoreProtocol : public QObject
{
	Q_OBJECT
public:
	enum State { NeedMore, Available, NoData, OutOfSync, WaitForPayload };

	CoreProtocol();

	virtual ~CoreProtocol();

	/**
	 * Reset the protocol, clear buffers
	 */
	void reset();

	/**
	 * Accept data from the network, and buffer it into a useful message
	 * This requires parsing out each packet from the incoming data
	 * @param incomingBytes Raw data
	 */
	void addIncomingData(const QByteArray &incomingBytes);

	/**
	 * @return the incoming transfer or 0 if none is available.
	 */
	Transfer incomingTransfer();

	/**
	 * Convert a request into an outgoing transfer
	 * emits @ref outgoingData() with each part of the transfer
	 */
	void outgoingTransfer(const Transfer &outgoing);

	/**
	 * Get the state of the protocol
	 */
	int state();

signals:
	/**
	 * Emitted as the core protocol converts fields to wire ready data
	 */
	void outgoingData(const QByteArray &);

	/**
	 * Emitted when there is incoming data, parsed into a Transfer
	 */
	void incomingData();
protected slots:
	/**
	 * Just a debug method to test emitting to the socket, atm - should go to the ClientStream
	 */
	void slotOutgoingData(const QByteArray &);

protected:
	/**
	 * Check that there is data to read, and set the protocol's state if there isn't any.
	 */
	bool okToProceed(const QDataStream &din);
	/**
	 * Convert incoming raw data into a Transfer object and queue it
	 * @return number of bytes from the input that were parsed into a Transfer
	 */
	int rawToTransfer(const QByteArray &raw);

	/**
	 * Check if the command is a payload command.
	 */
	bool isPayloadCommand(const QString &command);

private:
	class Private;
	Private *d;
};

}

#endif
