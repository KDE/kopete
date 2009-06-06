/*
    Kopete Yahoo Protocol
    
    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>
    
    Based on code 
    Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
    Copyright (C) 2003  Justin Karneges <justin@affinix.com>
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOO_CORE_PROTOCOL_H
#define YAHOO_CORE_PROTOCOL_H

#include <QObject>

class Transfer;
class YMSGProtocol;
class QByteArray;

class CoreProtocol : public QObject
{
Q_OBJECT
public:
	enum State { NeedMore, Available, NoData, OutOfSync };

	CoreProtocol();
	
	virtual ~CoreProtocol();
	
	/**
	 * Reset the protocol, clear buffers
	 */
	void reset();
	
	/**
	 * Accept data from the network, and buffer it into a useful message
	 * This requires parsing out each FLAP, etc. from the incoming data
	 * @param incomingBytes Raw data in wire format.
	 */
	void addIncomingData( const QByteArray& incomingBytes );
	
	/**
	 * @return the incoming transfer or 0 if none is available.
	 */
	Transfer* incomingTransfer();
	
	/** 
	 * Convert a request into an outgoing transfer
	 * emits @ref outgoingData() with each part of the transfer
	 */
	void outgoingTransfer( Transfer* outgoing );
	
	/**
	 * Get the state of the protocol 
	 */
	int state();
	
signals:
	/** 
	 * Emitted as the core protocol converts fields to wire ready data
	 */
	void outgoingData( const QByteArray& );
	
	/**
	 * Emitted when there is incoming data, parsed into a Transfer
	 */
	void incomingData();
protected slots:
	/**
	 * Just a debug method to test emitting to the socket, atm - should go to the ClientStream
	 */
	void slotOutgoingData( const QByteArray & );
	
protected:
	/**
	 * Check that there is data to read, and set the protocol's state if there isn't any.
	 */
	bool okToProceed( QDataStream & );
	/**
	 * Convert incoming wire data into a Transfer object and queue it
	 * @return number of bytes from the input that were parsed into a Transfer
	 */ 
	int wireToTransfer( const QByteArray& wire );

private:
	QByteArray m_in;	// buffer containing unprocessed bytes we received
	int m_error;
	Transfer* m_inTransfer; // the transfer that is being received
	int m_state;		// represents the protocol's overall state
	YMSGProtocol* m_YMSGProtocol;

};

#endif

