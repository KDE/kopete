//
// C++ Interface: eventprotocol
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GW_EVENTPROTOCOL_H
#define GW_EVENTPROTOCOL_H

#include <qcstring.h>
#include <qobject.h>

class EventTransfer;
/**
 * This class converts incoming event data into EventTransfer objects.  Since it requires knowledge of the binary event format, which 
 * differs for each event type, it is implemented as a separate class. See also @ref CoreProtocol, which detects event messages in the 
 * data stream and hands them to this class for processing.
 * Event Types
 *
@author SUSE AG
	Ablauf:
	CoreProtocol detects an event.  
		Passes whole chunk to EventProtocol ( as QByteArray )
		In to EventProtocol - QByteArray
		Returned from EventProtocol - EventTransfer *, bytes read, set state?
	EventProtocol tries to parse data into eventTransfer
	If not complete, sets state to NeedMore and returns 0
	If complete, returns number of bytes read for the event
	If bytes -lt length of chunk, CoreProtocol places the unread bytes back in m_in and calls processNext()
	if Core or EventProtocol set state to NeedMore, don't call wireToTransfer again.
	ProcessNext, if there is data in m_in, calls wireToTransfer. 
	
	What event dependent data does EventTransfer need to contain to do everything that the Tasks were doing?  
	Look at the tasks!
	What if some events contain EXTRA bytes off the end that we don't know about?  Then we will put those bytes back on the buffer, and try and parse those as the start of a new message!!!  Need to react SAFELY then.
	
	What event dependent binary data does each event type contain?
	From gwerror.h:
	enum Event {		InvalidRecipient 		= 101,
							NOTHANDLED
						UndeliverableStatus 	= 102,
							NOTHANDLED * 
						StatusChange 			= 103,
							Q_UINT16 STATUS
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

class EventProtocol : public QObject
{
Q_OBJECT
public:
	// 
	enum EventProtocolState { Success, NeedMore, OutOfSync, ProtocolError };
    EventProtocol(QObject *parent = 0, const char *name = 0);
    ~EventProtocol();
	/** 
	 * Attempt to parse the supplied data into an @ref EventTransfer object.  
	 * The exact state of the parse attempt can be read using @ref state. 
	 * @param rawData The unparsed data.
	 * @param bytes An integer used to return the number of bytes read.
	 * @return A pointer to an EventTransfer object if successfull, otherwise 0.  The caller is responsible for deleting this object.
	 */
	EventTransfer * parse( const QByteArray &, uint & bytes );
	/**
	 * Returns a value describing the state of the object.  
	 * If the object is given data to parse that does not begin with a recognised event code, 
	 * it will become OutOfSync, to indicate that the input data probably contains leftover data not processed during a previous parse.
	 */
	uint state() const;
protected:
	/**
	 * Reads a conference's flags
	 */
	bool readFlags( Q_UINT32 &flags);
	/**
	 * Reads an arbitrary string
	 */
	bool readString( QString &message );
	/**
	 * Check that there is data to read, and set the protocol's state if there isn't any.
	 */
	bool okToProceed();
	/** 
	 * read a Q_UINT32 giving the number of following bytes, then a string of that length
	 * @return false if the string was broken or there was no data available at all
	 */
	bool safeReadBytes( QCString & data, uint & len );
private:
	int m_state;
	uint m_bytes;
	QDataStream * m_din;
};

#endif
