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
	
	Transfer* incomingTransfer();
	
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
	QChar encode_method( Q_UINT8 method );

private:
	QByteArray in;
	int m_error;
	QPtrList<Transfer> inQueue;
};

#endif

