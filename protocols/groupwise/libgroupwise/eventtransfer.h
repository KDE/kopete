//
// C++ Interface: eventtransfer
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GW_EVENTTRANSFER_H
#define GW_EVENTTRANSFER_H

#include <qcstring.h>
#include <qdatetime.h>

#include <transfer.h>

/*#define	NMEVT_INVALID_RECIPIENT			101
#define	NMEVT_UNDELIVERABLE_STATUS		102
#define	NMEVT_STATUS_CHANGE				103
#define	NMEVT_CONTACT_ADD				104
#define	NMEVT_CONFERENCE_CLOSED			105
#define	NMEVT_CONFERENCE_JOINED			106
#define	NMEVT_CONFERENCE_LEFT			107
#define	NMEVT_RECEIVE_MESSAGE			108
#define	NMEVT_RECEIVE_FILE				109
#define NMEVT_USER_TYPING				112
#define NMEVT_USER_NOT_TYPING			113
#define NMEVT_USER_DISCONNECT			114
#define NMEVT_SERVER_DISCONNECT			115
#define NMEVT_CONFERENCE_RENAME			116
#define NMEVT_CONFERENCE_INVITE			117
#define NMEVT_CONFERENCE_INVITE_NOTIFY	118
#define NMEVT_CONFERENCE_REJECT			119
#define NMEVT_RECEIVE_AUTOREPLY			121
#define NMEVT_START						NMEVT_INVALID_RECIPIENT
#define NMEVT_STOP						NMEVT_RECEIVE_AUTOREPLY*/
namespace Event {
	
};

/**
@author Kopete Developers
*/
class EventTransfer : public Transfer
{
public:
	enum Event {		InvalidRecipient 		= 101,
						UndeliverableStatus 	= 102,
						StatusChange 			= 103,
						ContactAdd 				= 104,
						ConferenceClosed 		= 105,
						ConferenceJoined 		= 106,
						ConferenceLeft 			= 107,
						ReceiveMessage			= 108,
						ReceiveFile				= 109,
						UserTyping				= 112,
						UserNotTyping			= 113,
						UserDisconnect			= 114,
						ServerDisconnect		= 115,
						ConverenceRename		= 116,
						ConferenceInvite		= 117,
						ConferenceInviteNotify	= 118,
						ConferenceReject		= 119,
						ReceiveAutoReply		= 121,
						Start					= InvalidRecipient,
						Stop					= ReceiveAutoReply
				};
	EventTransfer( const Q_UINT32 eventType, QCString& source, QTime timeStamp );
	~EventTransfer();
	
	TransferType type() { return Transfer::EventTransfer; }
	
	int event();
	QCString source();
	QTime timeStamp();
private:
	int m_eventType;
	QCString m_source;
	QTime m_timeStamp;
};

#endif
