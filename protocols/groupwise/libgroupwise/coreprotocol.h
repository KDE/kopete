#ifndef GW_CORE_PROTOCOL_H
#define GW_CORE_PROTOCOL_H

#include <qcstring.h>
#include <qobject.h>

#include "gwfield.h"

class Transfer;

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
	/**
	 * Convert incoming wire data into a structured list of fields
	 * @return A list of fields.
	 */ 
	Field::FieldList processBuf();
	Transfer * incomingTransfer();
	void outgoingTransfer( Transfer * outgoing );
	
signals:
	/** 
	 * Emitted as the core protocol converts fields to wire ready data
	 */
	void outgoingData( const QCString & );
protected slots:
	/**
	 * Just a debug method to test emitting to the socket, atm - should go to the ClientStream
	 */
	void slotOutgoingData( const QCString & );
	
protected:
	void fieldsToWire( Field::FieldList fields );
	QChar encode_method( Q_UINT8 method );

private:
	QByteArray in;
	int m_error;
};

#endif

