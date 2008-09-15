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

//#include <QGlobals>
#include <QObject>
#include <QVariant>

#include "libgroupwise_export.h"

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
#define NM_PROTOCOL_VERSION				5
#define NMFIELD_MAX_STR_LENGTH			32768


/* Attribute Names (field tags) */
namespace Field {
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_IP_ADDRESS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_PORT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_FOLDER;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_CONTACT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_CONVERSATION;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_MESSAGE;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_CONTACT_LIST;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_RESULTS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_INFO_DISPLAY_ARRAY;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_USER_DETAILS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_OBJECT_ID;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_PARENT_ID;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_SEQUENCE_NUMBER;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_TYPE;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_STATUS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_STATUS_TEXT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_DN;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_DISPLAY_NAME;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_USERID;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_CREDENTIALS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_MESSAGE_BODY;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_MESSAGE_TEXT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_MESSAGE_TYPE;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_PARTICIPANTS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_INVITES;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_EVENT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_COUNT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_DATE;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_EVENT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_B_NO_CONTACTS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_B_NO_CUSTOMS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_B_NO_PRIVACY;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_B_ONLY_MODIFIED;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UW_STATUS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_OBJECT_ID;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_TRANSACTION_ID;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_RESULT_CODE;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_BUILD;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_AUTH_ATTRIBUTE;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_KEEPALIVE;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_USER_AGENT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_BLOCKING;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_BLOCKING_DENY_LIST;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_BLOCKING_ALLOW_LIST;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_BLOCKING_ALLOW_ITEM;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_BLOCKING_DENY_ITEM;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_LOCKED_ATTR_LIST;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_DEPARTMENT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_TITLE;
	// GW7
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_CUSTOM_STATUSES;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_STATUS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_QUERY_COUNT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_CHAT;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_DISPLAY_NAME;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_CHAT_OWNER_DN;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_PARTICIPANTS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_DESCRIPTION;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_DISCLAIMER;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_QUERY;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_ARCHIVE;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_MAX_USERS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_TOPIC;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_CHAT_ACL;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_FA_CHAT_ACL_ENTRY;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_SZ_ACCESS_FLAGS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_CHAT_CREATOR_DN;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_CREATION_TIME;
	extern LIBGROUPWISE_EXPORT QByteArray NM_A_UD_CHAT_RIGHTS;
	extern LIBGROUPWISE_EXPORT QByteArray NM_FIELD_TRUE;
	extern LIBGROUPWISE_EXPORT QByteArray NM_FIELD_FALSE;

	extern LIBGROUPWISE_EXPORT QByteArray KOPETE_NM_USER_DETAILS_CN;
	extern LIBGROUPWISE_EXPORT QByteArray KOPETE_NM_USER_DETAILS_GIVEN_NAME;
	extern LIBGROUPWISE_EXPORT QByteArray KOPETE_NM_USER_DETAILS_SURNAME;
	extern LIBGROUPWISE_EXPORT QByteArray KOPETE_NM_USER_DETAILS_ARCHIVE_FLAG;
	extern LIBGROUPWISE_EXPORT QByteArray KOPETE_NM_USER_DETAILS_FULL_NAME;


/**
 * Fields are typed units of information interchanged between the groupwise server and its clients.
 * In this implementation Fields are assumed to have a straight data flow from a Task to a socket and vice versa,
 * so the @ref Task::take() is responsible for deleting incoming Fields and the netcode is responsible for
 * deleting outgoing Fields.
 */

	/**
	 * Abstract base class of all field types
	 */
	class FieldBase
	{
	public:
//		FieldBase() {}
		FieldBase( const QByteArray & tag, quint8 method, quint8 flags, quint8 type );
		virtual ~FieldBase() {}
		QByteArray tag() const;
		quint8 method() const;
		quint8 flags() const;
		quint8 type() const;
		void setFlags( const quint8 flags );
	protected:
		QByteArray m_tag;
		quint8 m_method;
		quint8 m_flags;
		quint8 m_type;  // doch needed
	};
	
	typedef QList<Field::FieldBase*>::Iterator FieldListIterator;
	typedef QList<Field::FieldBase*>::ConstIterator FieldListConstIterator;
	class SingleField;
	class MultiField;
	
	class FieldList : public QList<Field::FieldBase *>
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
			FieldListIterator find( const QByteArray & tag );
			/** 
			 * Locate the first occurrence of a given field in the list, starting at the supplied iterator
			 * @param tag The tag name of the field to search for.
			 * @param it An iterator within the list, to start searching from.
			 * @return An iterator pointing to the first occurrence found, or end() if none was found.
			 */
			FieldListIterator find( FieldListIterator &it, const QByteArray & tag );
			/**
			 * Get the index of the first occurrence of tag, or -1 if not found
			 */
			int findIndex( const QByteArray & tag );
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
			SingleField * findSingleField( const QByteArray & tag );
			MultiField * findMultiField( const QByteArray & tag );
		protected:
			SingleField * findSingleField( FieldListIterator &it, const QByteArray & tag );
			MultiField * findMultiField( FieldListIterator &it, const QByteArray & tag );

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
		SingleField( const QByteArray & tag, quint8 method, quint8 flags, quint8 type, QVariant value );
		/** 
		 * Convenience constructor for NMFIELD_METHOD_VALID fields
		 */
		SingleField( const QByteArray & tag, quint8 flags, quint8 type, QVariant value );
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
		MultiField( const QByteArray & tag, quint8 method, quint8 flags, quint8 type );
		MultiField( const QByteArray & tag, quint8 method, quint8 flags, quint8 type, FieldList fields );
		~MultiField();
		FieldList fields() const;
		void setFields( FieldList );
	private:
		FieldList m_fields; // nb implicitly shared, copy-on-write - is there a case where this is bad?
	};
	
} // namespace Field
#endif
