//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RESPONSEPROTOCOL_H
#define RESPONSEPROTOCOL_H

#include "gwerror.h"
#include "gwfield.h"

#include "inputprotocolbase.h"

/**
Handles the parsing of incoming Response messages

@author Kopete Developers
*/
class ResponseProtocol : public InputProtocolBase
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
	
	ResponseProtocol(QObject* parent, const char* name);
	~ResponseProtocol();
	/** 
	 * Attempt to parse the supplied data into an @ref Response object.  
	 * The exact state of the parse attempt can be read using @ref state. 
	 * @param rawData The unparsed data.
	 * @param bytes An integer used to return the number of bytes read.
	 * @return A pointer to an Response object if successfull, otherwise 0.  The caller is responsible for deleting this object.
	 */
	Transfer * parse( const QByteArray &, uint & bytes );
protected:
	/**
	 * read a line ending in \r\n, including the \r\n
	 */
	bool readGroupWiseLine( QCString & );
	/**
	 * Read in a response
	 */
	bool readResponse();
	/** 
	 * Parse received fields and store in m_collatingFields
	 */
	bool readFields( int fieldCount, Field::FieldList * list = 0 );
private:
	// fields from a packet being parsed, before it has been completely received
	//QValueStack<Field::FieldList> m_collatingFields;
	Field::FieldList m_collatingFields;
	int m_packetState;	// represents the state of the parsing of the last incoming data received
};

#endif
