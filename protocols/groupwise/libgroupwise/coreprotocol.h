#ifndef GW_CORE_PROTOCOL_H
#define GW_CORE_PROTOCOL_H

#include <qcstring.h>
#include <qobject.h>
#include <qptrlist.h>

#include "gwfield.h"

class Transfer;
class Request;

class CoreProtocol : public QObject
{
Q_OBJECT
public:
	/**
	 * Describes the current state of the protocol
	 */
	enum State { NeedMore, Available, ServerError, ServerRedirect };
	
	CoreProtocol();
	
	virtual ~CoreProtocol();
	
	/**
	 * Accept data from the network, and buffer it into a useful message
	 * @param incomingBytes Raw data in wire format.
	 */
	void addIncomingData( const QByteArray& incomingBytes );
	/**
	 * Reset the protocol, clear buffers
	 */
	void reset();
	
	/**
	 * Returns the next incoming transfer from the queue or 0 if none is available
	 */
	Transfer* incomingTransfer();
	
	/** 
	 * Convert a request into an outgoing transfer
	 * emits @ref outgoingData() with each part of the transfer
	 */
	void outgoingTransfer( Request* outgoing );
	
signals:
	/** 
	 * Emitted as the core protocol converts fields to wire ready data
	 */
	void outgoingData( const QCString & );
	/**
	 * Emitted when there is incoming data, parsed into a Transfer
	 */
	void incomingData();
protected slots:
	/**
	 * Just a debug method to test emitting to the socket, atm - should go to the ClientStream
	 */
	void slotOutgoingData( const QCString & );
	
protected:
	/**
	 * Convert incoming wire data into a transfer object and queue
	 * 
	 */ 
	void wireToTransfer( const QByteArray& wire );
	/**
	 * Convert fields to a wire representation.  Emits outgoingData as each field is written.
	 * Calls itself recursively to process nested fields, hence
	 * @param depth Current depth of recursion.  Don't use this parameter yourself!
	 */
	void fieldsToWire( Field::FieldList fields, int depth = 0 );
	/**
	 * Read in an event
	 */
	void readEvent( const Q_UINT32 eventType, QDataStream& wireEvent );
	/**
	 * Read in a response
	 */
	bool readResponse( const QDataStream& wireRequest );
	
	/**
	 * encodes a method number (usually supplied as a #defined symbol) to a char
	 */
	QChar encode_method( Q_UINT8 method );

private:
	QByteArray m_in;
	int m_error;
	QPtrList<Transfer> m_inQueue;
	int m_state;
};

#endif

