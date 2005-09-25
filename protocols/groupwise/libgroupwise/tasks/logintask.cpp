/*
    Kopete Groupwise Protocol
    logintask.cpp - Send our credentials to the server and process the contact list and privacy details that it returns.

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges

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

#include "client.h"
#include "response.h"
#include "privacymanager.h"
#include "userdetailsmanager.h"

#include "logintask.h"
//Added by qt3to4:
#include <QByteArray>

LoginTask::LoginTask( Task * parent )
 : RequestTask( parent )
{
}

LoginTask::~LoginTask()
{
}

void LoginTask::initialise()
{
	QString command = QString::fromLatin1("login:%1:%2").arg( client()->host() ).arg( client()->port() );
	
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_USERID, 0, NMFIELD_TYPE_UTF8, client()->userId() ) );
	lst.append( new Field::SingleField( NM_A_SZ_CREDENTIALS, 0, NMFIELD_TYPE_UTF8, client()->password() ) );
	lst.append( new Field::SingleField( NM_A_SZ_USER_AGENT, 0, NMFIELD_TYPE_UTF8, client()->userAgent() ) );
	lst.append( new Field::SingleField( NM_A_UD_BUILD, 0, NMFIELD_TYPE_UDWORD, client()->protocolVersion() ) );
	lst.append( new Field::SingleField( NM_A_IP_ADDRESS, 0, NMFIELD_TYPE_UTF8, client()->ipAddress() ) );
	createTransfer( command, lst );
}

bool LoginTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	if ( response->resultCode() )
	{
		setError( response->resultCode() );
		return true;
	}
	response->fields().dump( true );
	
	// read in myself()'s metadata fields and emit signal
	Field::FieldList loginResponseFields = response->fields();
	
	ContactDetails cd = extractUserDetails( loginResponseFields );
	emit gotMyself( cd );

	// read the privacy settings first, because this affects all contacts' apparent status
	extractPrivacy( loginResponseFields );

	// CREATE CONTACT LIST
	// locate contact list
	Field::MultiField * contactList = loginResponseFields.findMultiField( NM_A_FA_CONTACT_LIST );
	if ( !contactList )
	{
		setError( Protocol );
		return true;
	}
	Field::FieldList contactListFields = contactList->fields();
	Field::MultiField * container;
	// read folders
	for ( Field::FieldListIterator it = contactListFields.find( NM_A_FA_FOLDER );
		  it != contactListFields.end();
		  it = contactListFields.find( ++it, NM_A_FA_FOLDER ) )
	{
		container = static_cast<Field::MultiField *>( *it );
		extractFolder( container );
	}
		  
	// read contacts
	for ( Field::FieldListIterator it = contactListFields.find( NM_A_FA_CONTACT );
		  it != contactListFields.end();
		  it = contactListFields.find( ++it, NM_A_FA_CONTACT ) )
	{
		container = static_cast<Field::MultiField *>( *it );
		extractContact( container );
	}
	
	setSuccess();
	
	return true;
}

void LoginTask::extractFolder( Field::MultiField * folderContainer )
{
	FolderItem folder;
	Field::SingleField * current;
	Field::FieldList fl = folderContainer->fields();
	// object id
	current = fl.findSingleField( NM_A_SZ_OBJECT_ID );
	folder.id = current->value().toInt();
	// sequence number
	current = fl.findSingleField( NM_A_SZ_SEQUENCE_NUMBER );
	folder.sequence = current->value().toInt();
	// name 
	current = fl.findSingleField( NM_A_SZ_DISPLAY_NAME );
	folder.name = current->value().toString();
	// parent
	current = fl.findSingleField( NM_A_SZ_PARENT_ID );
	folder.parentId = current->value().toInt();
	
	client()->debug( QString( "Got folder: %1, obj: %2, parent: %3, seq: %3." ).arg( folder.name ).arg(  folder.id ).arg( folder.parentId ).arg( folder.sequence ) );
	// tell the world about it
	emit gotFolder( folder );
}

void LoginTask::extractContact( Field::MultiField * contactContainer )
{
	if ( contactContainer->tag() != NM_A_FA_CONTACT )
		return;
	ContactItem contact;
	Field::SingleField * current;
	Field::FieldList fl = contactContainer->fields();
	// sequence number, object and parent IDs are a numeric values but are stored as strings...
	current = fl.findSingleField( NM_A_SZ_OBJECT_ID );
	contact.id = current->value().toInt();
	current = fl.findSingleField( NM_A_SZ_PARENT_ID );
	contact.parentId = current->value().toInt();
	current = fl.findSingleField( NM_A_SZ_SEQUENCE_NUMBER );
	contact.sequence = current->value().toInt();
	current = fl.findSingleField( NM_A_SZ_DISPLAY_NAME );
	contact.displayName = current->value().toString();
	current = fl.findSingleField( NM_A_SZ_DN );
	contact.dn = current->value().toString().toLower();
	emit gotContact( contact );
	Field::MultiField * details = fl.findMultiField( NM_A_FA_USER_DETAILS );
	if ( details ) // not all contact list contacts have these
	{
		Field::FieldList detailsFields = details->fields();
		ContactDetails cd = extractUserDetails( detailsFields );
		if ( cd.dn.isEmpty() )
			cd.dn = contact.dn;
		// tell the UserDetailsManager that we have this contact's details
		client()->userDetailsManager()->addDetails( cd );
		emit gotContactUserDetails( cd );
	}
}

ContactDetails LoginTask::extractUserDetails( Field::FieldList & fields )
{
	ContactDetails cd;
	cd.status = GroupWise::Invalid;
	cd.archive = false;
	// read the supplied fields, set metadata and status.
	Field::SingleField * sf;
	if ( ( sf = fields.findSingleField ( NM_A_SZ_AUTH_ATTRIBUTE ) ) )
		cd.authAttribute = sf->value().toString();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_DN ) ) )
		cd.dn =sf->value().toString().toLower(); // HACK: lowercased DN
	if ( ( sf = fields.findSingleField ( "CN" ) ) )
		cd.cn = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Given Name" ) ) )
		cd.givenName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Surname" ) ) )
		cd.surname = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Full Name" ) ) )
		cd.fullName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "nnmArchive" ) ) )
		cd.archive = ( sf->value().toInt() == 1 );
	if ( ( sf = fields.findSingleField ( NM_A_SZ_STATUS ) ) )
		cd.status = sf->value().toInt();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_MESSAGE_BODY ) ) )
		cd.awayMessage = sf->value().toString();
	Field::MultiField * mf;
	QMap< QString, QString > propMap;
	if ( ( mf = fields.findMultiField ( NM_A_FA_INFO_DISPLAY_ARRAY ) ) )
	{
		Field::FieldList fl = mf->fields();
		const Field::FieldListIterator end = fl.end();
		for ( Field::FieldListIterator it = fl.begin(); it != end; ++it )
		{
			Field::SingleField * propField = static_cast<Field::SingleField *>( *it );
			QString propName = propField->tag();
			QString propValue = propField->value().toString();
			propMap.insert( propName, propValue );
		}
	}
	if ( !propMap.empty() )
	{
		cd.properties = propMap;
	}
	return cd;
}

void LoginTask::extractPrivacy( Field::FieldList & fields )
{
	bool privacyLocked = false;
	bool defaultDeny = false;
	QStringList allowList;
	QStringList denyList;
	// read blocking
	// may be a single field or may be an array 
	Field::FieldListIterator it = fields.find( NM_A_LOCKED_ATTR_LIST );
	if ( it != fields.end() )
	{
		if ( Field::SingleField * sf = dynamic_cast<Field::SingleField *>( *it ) )
		{
			if ( sf->value().toString().find( NM_A_BLOCKING ) )
				privacyLocked = true;
		}
		else if ( Field::MultiField * mf = dynamic_cast<Field::MultiField *>( *it ) )
		{
			Field::FieldList fl = mf->fields();
			for ( Field::FieldListIterator it = fl.begin(); it != fl.end(); ++it )
			{
				if ( Field::SingleField * sf = dynamic_cast<Field::SingleField *>( *it ) )
				{
					if ( sf->tag() == NM_A_BLOCKING )
					{
						privacyLocked = true;
						break;
					}
				}
			}
		}
	}
	
	// read default privacy policy
	Field::SingleField * sf = fields.findSingleField( NM_A_BLOCKING );
	if ( sf )
		defaultDeny = ( sf->value().toInt() != 0 );
	
		
	// read deny list
	denyList = readPrivacyItems( NM_A_BLOCKING_DENY_LIST, fields );
	// read allow list
	allowList = readPrivacyItems( NM_A_BLOCKING_ALLOW_LIST, fields );
	emit gotPrivacySettings( privacyLocked, defaultDeny, allowList, denyList );
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "locked is " << privacyLocked << ", default is " << defaultDeny << "\nallow list is: " << allowList << "\ndeny list is: " << denyList << endl;
}

QStringList LoginTask::readPrivacyItems( const QByteArray & tag, Field::FieldList & fields )
{
	QStringList items;
	
	Field::FieldListIterator it = fields.find( tag );
	if ( it != fields.end() )
	{
		if ( Field::SingleField * sf = dynamic_cast<Field::SingleField *>( *it ) )
		{
			items.append( sf->value().toString().toLower() );
		}
		else if ( Field::MultiField * mf = dynamic_cast<Field::MultiField *>( *it ) )
		{
			Field::FieldList fl = mf->fields();
			for ( Field::FieldListIterator it = fl.begin(); it != fl.end(); ++it )
			{
				if ( Field::SingleField * sf = dynamic_cast<Field::SingleField *>( *it ) )
				{
					items.append( sf->value().toString().toLower() );
				}
			}
		}
	}
	return items;
}
#include "logintask.moc"
