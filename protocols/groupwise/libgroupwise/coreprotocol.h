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
	enum State { NeedMore, Available, ServerError, ServerRedirect, ReadingEvent, NoData };
	
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
	 * Check that there is data to read, and set the protocol's state if there isn't any.
	 */
	bool okToProceed();
	/**
	 * read a line ending in \r\n, including the \r\n
	 */
	bool readGroupWiseLine( QCString & );
	/** 
	 * read a Q_UINT32 giving the number of following bytes, then a string of that length
	 * @return false if the string was broken or there was no data available at all
	 */
	bool safeReadBytes( QCString & data, uint & len );
	/**
	 * Convert incoming wire data into a transfer object and queue
	 * 
	 */ 
	bool wireToTransfer( const QByteArray& wire );
	/**
	 * Convert fields to a wire representation.  Emits outgoingData as each field is written.
	 * Calls itself recursively to process nested fields, hence
	 * @param depth Current depth of recursion.  Don't use this parameter yourself!
	 */
	void fieldsToWire( Field::FieldList fields, int depth = 0 );
	/**
	 * Read in an event
	 * @param wire The raw data received from the wire
	 * @param bytesRead The number of bytes that have already been read from wire using m_din.  Needed to find the correct size of the payload to pass up to the event handlers.
	 */
	bool readEvent( const QByteArray& wire, int bytesRead = 0 );
	/**
	 * Read in a response
	 */
	bool readResponse();
	/** 
	 * Parse received fields and store in m_collatingFields
	 */
	bool readFields( int fieldCount, Field::FieldList * list = 0 );
	/**
	 * encodes a method number (usually supplied as a #defined symbol) to a char
	 */
	QChar encode_method( Q_UINT8 method );
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

