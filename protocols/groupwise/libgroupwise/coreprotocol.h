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
	enum State { NeedMore, Available, ServerError, ServerRedirect, ReadingEvent };
	
	/**
	 * Describes the parsing of the last received packet 
	 */
	enum PacketState { FieldsRead, ProtocolError };
	
	CoreProtocol();
	
	virtual ~CoreProtocol();
	
	/**
	 * Reset the protocol, clear buffers
	 */
	void reset();
	
	/**
	 * Accept data from the network, and buffer it into a useful message
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
	void outgoingTransfer( Request* outgoing );
	
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
	 * Read in an eventconst
	 */
	void readEvent(/* const Q_UINT32 eventType */);
	/**
	 * Read in a response
	 */
	bool readResponse();
	/** 
	 * Parse received fields and store in m_collatingFields
	 */
	void readFields( int fieldCount, Field::FieldList * list = 0 );
	/**
	 * encodes a method number (usually supplied as a #defined symbol) to a char
	 */
	QChar encode_method( Q_UINT8 method );
	/**
	 * read a line ending in \r\n, including the \r\n
	 */
	QCString readGroupWiseLine();
private:
	QByteArray m_in;
	QDataStream* m_din; // contains the packet currently being parsed
	int m_error;
	Transfer* m_inTransfer; // the transfer that is being received
	int m_state;		// represents the protocol's overall state
	int m_packetState;	// represents the state of the parsing of the last incoming data received
	// fields from a packet being parsed, before it has been completely received
	//QValueStack<Field::FieldList> m_collatingFields;
	Field::FieldList m_collatingFields;
	int m_collatingEvent; // the event code of an event that is in the process of being collated
};

#endif

