/*
    gwfield.h - Fields used for Request/Response data in GroupWise

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed    
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef GWFIELD_H
#define GWFIELD_H

/* Field types */
/* Comments: ^1 not used ^2 ignored ^3 apparently only used in _field_to_string for debug */
/* Otherwise: widely used */
#define	NMFIELD_TYPE_INVALID				0
/* ^1 */
#define	NMFIELD_TYPE_NUMBER				1 
/* ^1 */
#define	NMFIELD_TYPE_BINARY				2
/* ^2? */
#define	NMFIELD_TYPE_BYTE				3
/* ^3  */
#define	NMFIELD_TYPE_UBYTE				4
/* ^3  */
#define	NMFIELD_TYPE_WORD				5
/* ^3  */
#define	NMFIELD_TYPE_UWORD				6
/* ^3  */
#define	NMFIELD_TYPE_DWORD				7
/* ^3  */
#define	NMFIELD_TYPE_UDWORD				8 
/*WILLNOTE used in nm_send_login ( build ID ) and nm_send_message ( message type = 0 ) */
#define	NMFIELD_TYPE_ARRAY				9
#define	NMFIELD_TYPE_UTF8				10
#define	NMFIELD_TYPE_BOOL				11
/* ^3  */
#define	NMFIELD_TYPE_MV					12
#define	NMFIELD_TYPE_DN					13

/* Field methods */
#define NMFIELD_METHOD_VALID			0
#define NMFIELD_METHOD_IGNORE			1
#define NMFIELD_METHOD_DELETE			2
#define NMFIELD_METHOD_DELETE_ALL		3
#define NMFIELD_METHOD_EQUAL			4
#define NMFIELD_METHOD_ADD				5
#define NMFIELD_METHOD_UPDATE			6
#define NMFIELD_METHOD_GTE				10
#define NMFIELD_METHOD_LTE				12
#define NMFIELD_METHOD_NE				14
#define NMFIELD_METHOD_EXIST			15
#define NMFIELD_METHOD_NOTEXIST			16
#define NMFIELD_METHOD_SEARCH			17
#define NMFIELD_METHOD_MATCHBEGIN		19
#define NMFIELD_METHOD_MATCHEND			20
#define NMFIELD_METHOD_NOT_ARRAY		40
#define NMFIELD_METHOD_OR_ARRAY			41
#define NMFIELD_METHOD_AND_ARRAY		42

/* Attribute Names (field tags) */
#define NM_A_IP_ADDRESS					"nnmIPAddress"
#define	NM_A_PORT						"nnmPort"
#define	NM_A_FA_FOLDER					"NM_A_FA_FOLDER"
#define	NM_A_FA_CONTACT					"NM_A_FA_CONTACT"
#define	NM_A_FA_CONVERSATION			"NM_A_FA_CONVERSATION"
#define	NM_A_FA_MESSAGE					"NM_A_FA_MESSAGE"
#define	NM_A_FA_CONTACT_LIST			"NM_A_FA_CONTACT_LIST"
#define	NM_A_FA_RESULTS					"NM_A_FA_RESULTS"
#define	NM_A_FA_INFO_DISPLAY_ARRAY		"NM_A_FA_INFO_DISPLAY_ARRAY"
#define	NM_A_FA_USER_DETAILS			"NM_A_FA_USER_DETAILS"
#define	NM_A_SZ_OBJECT_ID				"NM_A_SZ_OBJECT_ID"
#define	NM_A_SZ_PARENT_ID				"NM_A_SZ_PARENT_ID"
#define	NM_A_SZ_SEQUENCE_NUMBER			"NM_A_SZ_SEQUENCE_NUMBER"
#define	NM_A_SZ_TYPE					"NM_A_SZ_TYPE"
#define	NM_A_SZ_STATUS					"NM_A_SZ_STATUS"
#define	NM_A_SZ_STATUS_TEXT				"NM_A_SZ_STATUS_TEXT"
#define	NM_A_SZ_DN						"NM_A_SZ_DN"
#define	NM_A_SZ_DISPLAY_NAME			"NM_A_SZ_DISPLAY_NAME"
#define	NM_A_SZ_USERID					"NM_A_SZ_USERID"
#define NM_A_SZ_CREDENTIALS				"NM_A_SZ_CREDENTIALS"
#define	NM_A_SZ_MESSAGE_BODY			"NM_A_SZ_MESSAGE_BODY"
#define	NM_A_SZ_MESSAGE_TEXT			"NM_A_SZ_MESSAGE_TEXT"
#define	NM_A_UD_MESSAGE_TYPE			"NM_A_UD_MESSAGE_TYPE"
#define	NM_A_FA_PARTICIPANTS			"NM_A_FA_PARTICIPANTS"
#define	NM_A_FA_INVITES					"NM_A_FA_INVITES"
#define	NM_A_FA_EVENT					"NM_A_FA_EVENT"
#define	NM_A_UD_COUNT					"NM_A_UD_COUNT"
#define	NM_A_UD_DATE					"NM_A_UD_DATE"
#define	NM_A_UD_EVENT					"NM_A_UD_EVENT"
#define	NM_A_B_NO_CONTACTS				"NM_A_B_NO_CONTACTS"
#define	NM_A_B_NO_CUSTOMS				"NM_A_B_NO_CUSTOMS"
#define	NM_A_B_NO_PRIVACY				"NM_A_B_NO_PRIVACY"
#define	NM_A_B_ONLY_MODIFIED			"NM_A_B_ONLY_MODIFIED"
#define	NM_A_UW_STATUS					"NM_A_UW_STATUS"
#define	NM_A_UD_OBJECT_ID				"NM_A_UD_OBJECT_ID"
#define	NM_A_SZ_TRANSACTION_ID			"NM_A_SZ_TRANSACTION_ID"
#define	NM_A_SZ_RESULT_CODE				"NM_A_SZ_RESULT_CODE"
#define	NM_A_UD_BUILD					"NM_A_UD_BUILD"
#define	NM_A_SZ_AUTH_ATTRIBUTE			"NM_A_SZ_AUTH_ATTRIBUTE"
#define	NM_A_UD_KEEPALIVE				"NM_A_UD_KEEPALIVE"
#define NM_A_SZ_USER_AGENT				"NM_A_SZ_USER_AGENT"
#define NM_A_BLOCKING					"nnmBlocking"
#define NM_A_BLOCKING_DENY_LIST			"nnmBlockingDenyList"
#define NM_A_BLOCKING_ALLOW_LIST		"nnmBlockingAllowList"
#define	NM_A_SZ_BLOCKING_ALLOW_ITEM		"NM_A_SZ_BLOCKING_ALLOW_ITEM"
#define	NM_A_SZ_BLOCKING_DENY_ITEM		"NM_A_SZ_BLOCKING_DENY_ITEM"
#define NM_A_LOCKED_ATTR_LIST			"nnmLockedAttrList"
#define NM_A_SZ_DEPARTMENT				"OU"
#define NM_A_SZ_TITLE					"Title"
// GW7
#define NM_A_FA_CUSTOM_STATUSES			"NM_A_FA_CUSTOM_STATUSES"
#define NM_A_FA_STATUS					"NM_A_FA_STATUS"
#define NM_A_UD_QUERY_COUNT				"NM_A_UD_QUERY_COUNT"
#define NM_A_FA_CHAT					"NM_A_FA_CHAT"
#define NM_A_DISPLAY_NAME				"nnmDisplayName"
#define NM_A_CHAT_OWNER_DN				"nnmChatOwnerDN"
#define NM_A_UD_PARTICIPANTS			"NM_A_UD_PARTICIPANTS"
#define NM_A_DESCRIPTION				"nnmDescription"
#define NM_A_DISCLAIMER					"nnmDisclaimer"
#define NM_A_QUERY						"nnmQuery"
#define NM_A_ARCHIVE					"nnmArchive"
#define NM_A_MAX_USERS					"nnmMaxUsers"
#define NM_A_SZ_TOPIC					"NM_A_SZ_TOPIC"
#define NM_A_FA_CHAT_ACL				"NM_A_FA_CHAT_ACL"
#define NM_A_FA_CHAT_ACL_ENTRY			"NM_A_FA_CHAT_ACL_ENTRY"
#define NM_A_SZ_ACCESS_FLAGS			"NM_A_SZ_ACCESS_FLAGS"
#define NM_A_CHAT_CREATOR_DN			"nnmCreatorDN"
#define NM_A_CREATION_TIME				"nnmCreationTime"
#define NM_A_UD_CHAT_RIGHTS				"NM_A_UD_CHAT_RIGHTS"

#define NM_PROTOCOL_VERSION		 		5
#define	NM_FIELD_TRUE					"1"
#define	NM_FIELD_FALSE					"0"

#define NMFIELD_MAX_STR_LENGTH			32768

#include <qglobal.h>
#include <qobject.h>
#include <qvariant.h>
#include <qvaluelist.h>	

/**
 * Fields are typed units of information interchanged between the groupwise server and its clients.
 * In this implementation Fields are assumed to have a straight data flow from a Task to a socket and vice versa,
 * so the @ref Task::take() is responsible for deleting incoming Fields and the netcode is responsible for
 * deleting outgoing Fields.
 */

namespace Field
{
	/**
	 * Abstract base class of all field types
	 */
	class FieldBase
	{
	public:
		FieldBase() {}
		FieldBase( QCString tag, Q_UINT8 method, Q_UINT8 flags, Q_UINT8 type );
		virtual ~FieldBase() {}
		QCString tag() const;
		Q_UINT8 method() const;
		Q_UINT8 flags() const;
		Q_UINT8 type() const;
		void setFlags( const Q_UINT8 flags );
	protected:
		QCString m_tag;
		Q_UINT8 m_method;
		Q_UINT8 m_flags;
		Q_UINT8 m_type;  // doch needed
	};
	
	typedef QValueListIterator<FieldBase *> FieldListIterator;
	typedef QValueListConstIterator<FieldBase *> FieldListConstIterator;
	class SingleField;
	class MultiField;
	
	class FieldList : public QValueList<FieldBase *>
	{
		public:
			/** 
			 * Destructor - doesn't delete the fields because FieldLists are passed by value
			 */
			virtual ~FieldList();
			/** 
			 * Locate the first occurrence of a given field in the list.  Same semantics as QValueList::find().
			 * @param tag The tag name of the field to search for.
			 * @return An iterator pointing to the first occurrence found, or end() if none was found.
			 */
			FieldListIterator find( QCString tag );
			/** 
			 * Locate the first occurrence of a given field in the list, starting at the supplied iterator
			 * @param tag The tag name of the field to search for.
			 * @param it An iterator within the list, to start searching from.
			 * @return An iterator pointing to the first occurrence found, or end() if none was found.
			 */
			FieldListIterator find( FieldListIterator &it, QCString tag );
			/**
			 * Get the index of the first occurrence of tag, or -1 if not found
			 */
			int findIndex( QCString tag );
			/** 
			 * Debug function, dumps to stdout
			 */
			void dump( bool recursive = false, int offset = 0 );
			/**
			 * Delete the contents of the list
			 */
			void purge();
			/** 
			 * Utility functions for finding the first instance of a tag
			 * @return 0 if no field of the right tag and type was found.
			 */
			SingleField * findSingleField( QCString tag );
			MultiField * findMultiField( QCString tag );
		protected:
			SingleField * findSingleField( FieldListIterator &it, QCString tag );
			MultiField * findMultiField( FieldListIterator &it, QCString tag );

	};

	/**
	 * This class is responsible for storing all Groupwise single value field types, eg
	 * NMFIELD_TYPE_INVALID, NMFIELD_TYPE_NUMBER, NMFIELD_TYPE_BINARY, NMFIELD_TYPE_BYTE
	 * NMFIELD_TYPE_UBYTE, NMFIELD_TYPE_DWORD, NMFIELD_TYPE_UDWORD, NMFIELD_TYPE_UTF8, NMFIELD_TYPE_BOOL
	 * NMFIELD_TYPE_DN
	 */
	class SingleField : public FieldBase
	{
	public:
		/** 
		 * Single field constructor
		 */
		SingleField( QCString tag, Q_UINT8 method, Q_UINT8 flags, Q_UINT8 type, QVariant value );
		/** 
		 * Convenience constructor for NMFIELD_METHOD_VALID fields
		 */
		SingleField( QCString tag, Q_UINT8 flags, Q_UINT8 type, QVariant value );
		~SingleField();
		void setValue( const QVariant v );
		QVariant value() const;
	private:
		QVariant m_value;
	};

	/**
	 * This class is responsible for storing multi-value GroupWise field types, eg
	 * NMFIELD_TYPE_ARRAY, NMFIELD_TYPE_MV
	 */
	class MultiField : public FieldBase
	{
	public:  
		MultiField( QCString tag, Q_UINT8 method, Q_UINT8 flags, Q_UINT8 type );
		MultiField( QCString tag, Q_UINT8 method, Q_UINT8 flags, Q_UINT8 type, FieldList fields );
		~MultiField();
		FieldList fields() const;
		void setFields( FieldList );
	private:
		FieldList m_fields; // nb implicitly shared, copy-on-write - is there a case where this is bad?
	};
	
}

#endif
