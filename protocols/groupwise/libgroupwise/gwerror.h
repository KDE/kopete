// taken from nmuser.h

#ifndef GW_ERROR_H
#define GW_ERROR_H

#include <qdatetime.h>
#include <qglobal.h>
#include <qmap.h>
#include <qstring.h>

typedef Q_UINT16 NMERR_T;
#define GROUPWISE_DEBUG_GLOBAL 14220

#define BLANK_GUID "[00000000-00000000-00000000-0000-0000]"
#define CONF_GUID_END 27

/*#define	NM_STATUS_UNKNOWN			0
#define	NM_STATUS_OFFLINE			1
#define NM_STATUS_AVAILABLE			2
#define	NM_STATUS_BUSY				3
#define	NM_STATUS_AWAY				4
#define	NM_STATUS_AWAY_IDLE			5
#define	NM_STATUS_INVALID			6
*/

namespace GroupWise
{
	enum Status {	Unknown = 0,
					Offline = 1,
					Available = 2,
					Busy = 3,
					Away = 4,
					AwayIdle = 5,
					Invalid = 6
				};
				
	enum Error {	None = 0,
					ErrorBase = 0x2000L,
					BadParm,
					TCPWrite,
					TCPRead,
					Protocol,
					ServerRedirect,
					ConferenceNotFound,
					ConferenceNotInstantiated,
					FolderExists
				};
	
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
						ConferenceRename		= 116,
						ConferenceInvite		= 117,
						ConferenceInviteNotify	= 118,
						ConferenceReject		= 119,
						ReceiveAutoReply		= 121,
						Start					= InvalidRecipient,
						Stop					= ReceiveAutoReply
				};
	
	enum ConferenceFlags { 	Logging = 0x00000001,
							Secure  = 0x00000002,
							Closed  = 0x10000000
						 };
	
	// helpful structs used to pass data between the client library and the application using it
	struct ConferenceEvent 
	{
		Event type;
		QString guid;
		QString user;
		QDateTime timeStamp;
		Q_UINT32 flags;
		QString message;
	};
	
	struct FolderItem
	{
		uint id; 
		uint sequence;
		uint parentId;
		QString name;
	};
	
	struct ContactItem
	{
		uint id;
		uint parentId;
		uint sequence;
		QString dn;
		QString displayName;
	};

	struct ContactDetails
	{
		QString cn, 
				dn, 
				givenName, 
				surname, 
				fullName, 
				awayMessage, 
				authAttribute;
		int status;
		QMap< QString, QString > properties;
	};
	
	struct OutgoingMessage
	{
		QString guid;
		QString message;
		QString rtfMessage;
	};
};

// temporary typedef pending implementation

// #define NMERR_BASE							0x2000L
// #define NM_OK								0L
// #define NMERR_BAD_PARM						(NMERR_BASE + 0x0001)
// #define NMERR_TCP_WRITE						(NMERR_BASE + 0x0002)
// #define NMERR_TCP_READ						(NMERR_BASE + 0x0003)
// #define NMERR_PROTOCOL						(NMERR_BASE + 0x0004)
// #define NMERR_SERVER_REDIRECT				(NMERR_BASE + 0x0005)
// #define NMERR_CONFERENCE_NOT_FOUND 			(NMERR_BASE + 0x0006)
// #define NMERR_CONFERENCE_NOT_INSTANTIATED 	(NMERR_BASE + 0x0007)
// #define NMERR_FOLDER_EXISTS					(NMERR_BASE + 0x0008)

/* Errors that are returned from the server */
#define NMERR_SERVER_BASE			 	0xD100L
#define NMERR_ACCESS_DENIED			 	(NMERR_SERVER_BASE + 0x0006)
#define NMERR_NOT_SUPPORTED          	(NMERR_SERVER_BASE + 0x000A)
#define NMERR_PASSWORD_EXPIRED       	(NMERR_SERVER_BASE + 0x000B)
#define NMERR_PASSWORD_INVALID       	(NMERR_SERVER_BASE + 0x000C)
#define NMERR_USER_NOT_FOUND         	(NMERR_SERVER_BASE + 0x000D)
#define NMERR_USER_DISABLED          	(NMERR_SERVER_BASE + 0x0010)
#define NMERR_DIRECTORY_FAILURE      	(NMERR_SERVER_BASE + 0x0011)
#define NMERR_HOST_NOT_FOUND		 	(NMERR_SERVER_BASE + 0x0019)
#define NMERR_ADMIN_LOCKED           	(NMERR_SERVER_BASE + 0x001C)
#define NMERR_DUPLICATE_PARTICIPANT  	(NMERR_SERVER_BASE + 0x001F)
#define NMERR_SERVER_BUSY            	(NMERR_SERVER_BASE + 0x0023)
#define NMERR_OBJECT_NOT_FOUND       	(NMERR_SERVER_BASE + 0x0024)
#define NMERR_DIRECTORY_UPDATE       	(NMERR_SERVER_BASE + 0x0025)
#define NMERR_DUPLICATE_FOLDER       	(NMERR_SERVER_BASE + 0x0026)
#define NMERR_DUPLICATE_CONTACT      	(NMERR_SERVER_BASE + 0x0027)
#define NMERR_USER_NOT_ALLOWED       	(NMERR_SERVER_BASE + 0x0028)
#define NMERR_TOO_MANY_CONTACTS      	(NMERR_SERVER_BASE + 0x0029)
#define NMERR_CONFERENCE_NOT_FOUND_2   	(NMERR_SERVER_BASE + 0x002B)
#define NMERR_TOO_MANY_FOLDERS       	(NMERR_SERVER_BASE + 0x002C)
#define NMERR_SERVER_PROTOCOL        	(NMERR_SERVER_BASE + 0x0030)
#define NMERR_CONVERSATION_INVITE		(NMERR_SERVER_BASE + 0x0035)
#define NMERR_USER_BLOCKED	         	(NMERR_SERVER_BASE + 0x0039)
#define NMERR_MASTER_ARCHIVE_MISSING 	(NMERR_SERVER_BASE + 0x003A)
#define NMERR_PASSWORD_EXPIRED_2     	(NMERR_SERVER_BASE + 0x0042)
#define NMERR_CREDENTIALS_MISSING   	(NMERR_SERVER_BASE + 0x0046)
#define NMERR_AUTHENTICATION_FAILED		(NMERR_SERVER_BASE + 0x0049)
#define NMERR_EVAL_CONNECTION_LIMIT		(NMERR_SERVER_BASE + 0x004A)

#endif
