/*
    Kopete Groupwise Protocol
    eventprotocol.h - reads the protocol used by GroupWise for signalling Events

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
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

#ifndef GW_EVENTPROTOCOL_H
#define GW_EVENTPROTOCOL_H

#include "inputprotocolbase.h"

class EventTransfer;
/**
 * This class converts incoming event data into EventTransfer objects.  Since it requires knowledge of the binary event format, which 
 * differs for each event type, it is implemented as a separate class. See also @ref CoreProtocol, which detects event messages in the 
 * data stream and hands them to this class for processing.
 * Event Types
 *
@author SUSE AG
	Ablauf:
	CoreProtocol receives data in addIncomingData, and passes to wireToTransfer.
	wireToTransfer detects an event.
		Passes whole chunk to EventProtocol ( as QByteArray )
		In to EventProtocol - QByteArray
		Returned from EventProtocol - EventTransfer *, bytes read, set state?
	EventProtocol tries to parse data into eventTransfer
	If not complete, sets state to NeedMore and returns 0
	If complete, returns number of bytes read for the event
	If bytes less than length of chunk, CoreProtocol::addIncomingData places the unread bytes back in m_in and calls wireToTransfer again.
	if ResponseProtocol or EventProtocol set state to NeedMore, don't call wireToTransfer again.
	
	What event dependent data does EventTransfer contain?
	
	What if some events contain EXTRA bytes off the end that we don't know about?  Then we will put those bytes back on the buffer, and try and parse those as the start of a new message!!!  Need to react SAFELY then.
	
	What event dependent binary data does each event type contain?

	All Events contain an event code, and a source ( a DN )
	NOTHANDLED indicates that there is no further data and we don't handle events of that type, because they are not sent by the server
	NONE indicates there is no further data
	STATUSTEXT, GUID, MESSAGE indicate a string encoded in the usual GroupWise binary string encoding: a UINT32 containing the string length in little-endian, followed by the string itself, as UTF-8 encoded unicode.  The string length value includes a terminating NUL, so when converting to a QString, subtract one from the string length.
	FLAGS contains a UINT32 containing the server's flags for this conference.  See gwerror.h for the possible values and meanings of these flags.  Only Logging has been observed in practice.

	All events are timestamped with the local time on receipt.

	From gwerror.h:
	enum Event {		InvalidRecipient 		= 101,
							NOTHANDLED
						UndeliverableStatus 	= 102,
							NOTHANDLED * 
						StatusChange 			= 103,
							quint16 STATUS
							STATUSTEXT
						ContactAdd 				= 104,
							NOTHANDLED
						ConferenceClosed 		= 105,
							GUID
						ConferenceJoined 		= 106,
							GUID
							FLAGS
						ConferenceLeft 			= 107,
							GUID
							FLAGS
						ReceiveMessage			= 108,
							GUID
							FLAGS
							MESSAGE
						ReceiveFile				= 109,
							NOTHANDLED
						UserTyping				= 112,
							GUID
						UserNotTyping			= 113,
							GUID
						UserDisconnect			= 114,
							NONE
						ServerDisconnect		= 115,
							NONE
						ConferenceRename		= 116,
							NOTHANDLED
						ConferenceInvite		= 117,
							GUID
							MESSAGE
						ConferenceInviteNotify	= 118,
							GUID
						ConferenceReject		= 119,
							GUID
						ReceiveAutoReply		= 121,
							GUID
							FLAGS
							MESSAGE
						Start					= InvalidRecipient,
						Stop					= ReceiveAutoReply
				};
	Therefore we have GUID, FLAGS, MESSAGE, STATUS, STATUSTEXT.  All transfers have TYPE and SOURCE, and a TIMESTAMP is added on receipt.
*/

class EventProtocol : public InputProtocolBase
{
Q_OBJECT
public:
    EventProtocol(QObject *parent = 0);
    ~EventProtocol();
	/** 
	 * Attempt to parse the supplied data into an @ref EventTransfer object.  
	 * The exact state of the parse attempt can be read using @ref state. 
	 * @param rawData The unparsed data.
	 * @param bytes An integer used to return the number of bytes read.
	 * @return A pointer to an EventTransfer object if successful, otherwise 0.  The caller is responsible for deleting this object.
	 */
	Transfer * parse( QByteArray &, uint & bytes );
protected:
	/**
	 * Reads a conference's flags
	 */
	bool readFlags( quint32 &flags);
};

#endif
