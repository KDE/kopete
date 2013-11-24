/*
    gwerror.cpp - Kopete Groupwise Protocol
  
    Copyright (c) 2007     Novell, Inc http://www.novell.com/linux

    Kopete (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwerror.h"

#include <klocale.h>

QString GroupWise::errorCodeToString( int errorCode )
{
	QString errorString;
	switch ( errorCode )
	{
#if 0		
		case NMERR_ACCESS_DENIED:
			errorString = i18n( "Access denied" );
			break;
		case NMERR_NOT_SUPPORTED:
			errorString = i18n( "Not supported" );
			break;
		case NMERR_PASSWORD_EXPIRED:
			errorString = i18n( "Password expired" );
			break;
		case NMERR_PASSWORD_INVALID:
			errorString = i18n( "Invalid password" );
			break;
		case NMERR_USER_NOT_FOUND:
			errorString = i18n( "User not found" );
			break;
		case NMERR_ATTRIBUTE_NOT_FOUND:
			errorString = i18n( "Attribute not found" );
			break;
		case NMERR_USER_DISABLED:
			errorString = i18n( "User not enabled" );
			break;
		case NMERR_DIRECTORY_FAILURE:
			errorString = i18n( "Directory failure" );
			break;
		case NMERR_HOST_NOT_FOUND:
			errorString = i18n( "Host not found" );
			break;
		case NMERR_ADMIN_LOCKED:
			errorString = i18n( "Locked by admin" );
			break;
		case NMERR_DUPLICATE_PARTICIPANT:
			errorString = i18n( "Duplicate participant" );
			break;
		case NMERR_SERVER_BUSY:
			errorString = i18n( "Server busy" );
			break;
		case NMERR_OBJECT_NOT_FOUND:
			errorString = i18n( "Object not found" );
			break;
		case NMERR_DIRECTORY_UPDATE:
			errorString = i18n( "Directory update" );
			break;
		case NMERR_DUPLICATE_FOLDER:
			errorString = i18n( "Duplicate folder" );
			break;
		case NMERR_DUPLICATE_CONTACT:
			errorString = i18n( "Contact list entry already exists" );
			break;
		case NMERR_USER_NOT_ALLOWED:
			errorString = i18n( "User not allowed" );
			break;
		case NMERR_TOO_MANY_CONTACTS:
			errorString = i18n( "Too many contacts" );
			break;
		case NMERR_CONFERENCE_NOT_FOUND_2:
			errorString = i18n( "Conference not found" );
			break;
		case NMERR_TOO_MANY_FOLDERS:
			errorString = i18n( "Too many folders" );
			break;
		case NMERR_SERVER_PROTOCOL:
			errorString = i18n( "Server protocol error" );
			break;
		case NMERR_CONVERSATION_INVITE:
			errorString = i18n( "Conversation invitation error" );
			break;
		case NMERR_USER_BLOCKED:
			errorString = i18n( "User is blocked" );
			break;
		case NMERR_MASTER_ARCHIVE_MISSING:
			errorString = i18n( "Master archive is missing" );
			break;
		case NMERR_PASSWORD_EXPIRED_2:
			errorString = i18n( "Expired password in use" );
			break;
		case NMERR_CREDENTIALS_MISSING:
			errorString = i18n( "Credentials missing" );
			break;
		case NMERR_AUTHENTICATION_FAILED:
			errorString = i18n( "Authentication failed" );
			break;
		case NMERR_EVAL_CONNECTION_LIMIT:
			errorString = i18n( "Eval connection limit" );
			break;
		case MSGPRES_ERR_UNSUPPORTED_CLIENT_VERSION:
			errorString = i18n( "Unsupported client version" );
			break;
		case MSGPRES_ERR_DUPLICATE_CHAT:
			errorString = i18n( "A duplicate chat was found" );
			break;
		case MSGPRES_ERR_CHAT_NOT_FOUND:
			errorString = i18n( "Chat not found" );
			break;
		case MSGPRES_ERR_INVALID_NAME:
			errorString = i18n( "Invalid chat name" );
			break;
		case MSGPRES_ERR_CHAT_ACTIVE:
			errorString = i18n( "The chat is active" );
			break;
		case MSGPRES_ERR_CHAT_BUSY:
			errorString = i18n( "Chat is busy; try again" );
			break;
		case MSGPRES_ERR_REQUEST_TOO_SOON:
			errorString = i18n( "Tried request too soon after another; try again" );
			break;
		case MSGPRES_ERR_CHAT_NOT_ACTIVE:
			errorString = i18n( "Server's chat subsystem is not active" );
			break;
		case MSGPRES_ERR_INVALID_CHAT_UPDATE:
			errorString = i18n( "The chat update request is invalid" );
			break;
		case MSGPRES_ERR_DIRECTORY_MISMATCH:
			errorString = i18n( "Write failed due to directory mismatch" );
			break;
		case MSGPRES_ERR_RECIPIENT_TOO_OLD:
			errorString = i18n( "Recipient's client version is too old" );
			break;
		case MSGPRES_ERR_CHAT_NO_LONGER_VALID:
			errorString = i18n( "Chat has been removed from server" );
			break;
		default:
			errorString = i18n("Unrecognized error code: %1", errorCode );
#else
		case NMERR_ACCESS_DENIED:
			errorString = "Access denied";
			break;
		case NMERR_NOT_SUPPORTED:
			errorString = "Not supported";
			break;
		case NMERR_PASSWORD_EXPIRED:
			errorString = "Password expired";
			break;
		case NMERR_PASSWORD_INVALID:
			errorString = "Invalid password";
			break;
		case NMERR_USER_NOT_FOUND:
			errorString = "User not found";
			break;
		case NMERR_ATTRIBUTE_NOT_FOUND:
			errorString = "Attribute not found";
			break;
		case NMERR_USER_DISABLED:
			errorString = "User is disabled";
			break;
		case NMERR_DIRECTORY_FAILURE:
			errorString = "Directory failure";
			break;
		case NMERR_HOST_NOT_FOUND:
			errorString = "Host not found";
			break;
		case NMERR_ADMIN_LOCKED:
			errorString = "Locked by admin";
			break;
		case NMERR_DUPLICATE_PARTICIPANT:
			errorString = "Duplicate participant";
			break;
		case NMERR_SERVER_BUSY:
			errorString = "Server busy";
			break;
		case NMERR_OBJECT_NOT_FOUND:
			errorString = "Object not found";
			break;
		case NMERR_DIRECTORY_UPDATE:
			errorString = "Directory update";
			break;
		case NMERR_DUPLICATE_FOLDER:
			errorString = "Duplicate folder";
			break;
		case NMERR_DUPLICATE_CONTACT:
			errorString = "Contact list entry already exists";
			break;
		case NMERR_USER_NOT_ALLOWED:
			errorString = "User not allowed";
			break;
		case NMERR_TOO_MANY_CONTACTS:
			errorString = "Too many contacts";
			break;
		case NMERR_CONFERENCE_NOT_FOUND_2:
			errorString = "Conference not found";
			break;
		case NMERR_TOO_MANY_FOLDERS:
			errorString = "Too many folders";
			break;
		case NMERR_SERVER_PROTOCOL:
			errorString = "Server protocol error";
			break;
		case NMERR_CONVERSATION_INVITE:
			errorString = "Conversation invitation error";
			break;
		case NMERR_USER_BLOCKED:
			errorString = "User is blocked";
			break;
		case NMERR_MASTER_ARCHIVE_MISSING:
			errorString = "Master archive is missing";
			break;
		case NMERR_PASSWORD_EXPIRED_2:
			errorString = "Expired password in use";
			break;
		case NMERR_CREDENTIALS_MISSING:
			errorString = "Credentials missing";
			break;
		case NMERR_AUTHENTICATION_FAILED:
			errorString = "Authentication failed";
			break;
		case NMERR_EVAL_CONNECTION_LIMIT:
			errorString = "Eval connection limit";
			break;
		case MSGPRES_ERR_UNSUPPORTED_CLIENT_VERSION:
			errorString = "Unsupported client version";
			break;
		case MSGPRES_ERR_DUPLICATE_CHAT:
			errorString = "A duplicate chat was found";
			break;
		case MSGPRES_ERR_CHAT_NOT_FOUND:
			errorString = "Chat not found";
			break;
		case MSGPRES_ERR_INVALID_NAME:
			errorString = "Invalid chat name";
			break;
		case MSGPRES_ERR_CHAT_ACTIVE:
			errorString = "The chat is active";
			break;
		case MSGPRES_ERR_CHAT_BUSY:
			errorString = "Chat is busy; try again";
			break;
		case MSGPRES_ERR_REQUEST_TOO_SOON:
			errorString = "Tried request too soon after another; try again";
			break;
		case MSGPRES_ERR_CHAT_NOT_ACTIVE:
			errorString = "Server's chat subsystem is not active";
			break;
		case MSGPRES_ERR_INVALID_CHAT_UPDATE:
			errorString = "The chat update request is invalid";
			break;
		case MSGPRES_ERR_DIRECTORY_MISMATCH:
			errorString = "Write failed due to directory mismatch";
			break;
		case MSGPRES_ERR_RECIPIENT_TOO_OLD:
			errorString = "Recipient's client version is too old";
			break;
		case MSGPRES_ERR_CHAT_NO_LONGER_VALID:
			errorString = "Chat has been removed from server";
			break;
		default:
			errorString = QString("Unrecognized error code: %s").arg( errorCode );
#endif
	}
	return errorString;
}

GroupWise::FolderItem::FolderItem()
: id(0), sequence(0), parentId(0)
{

}
