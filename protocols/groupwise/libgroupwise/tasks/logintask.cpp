//
// C++ Implementation: logintask
//
// Description: 
//
//
// Author: SUSE AG (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "client.h"
#include "response.h"

#include "logintask.h"

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
	lst.append( new Field::SingleField( NM_A_UD_BUILD, 0, NMFIELD_TYPE_UDWORD, 2 ) );
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
	response->fields().dump( true );
	
	// read in myself()'s metadata fields and emit signal
	Field::FieldList loginResponseFields = response->fields();
	ContactDetails cd = extractUserDetails( loginResponseFields );
	emit gotMyself( cd );
	
	// CREATE CONTACT LIST
	// locate contact list
	Field::MultiField * contactList = loginResponseFields.findMultiField( NM_A_FA_CONTACT_LIST );
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
	// TODO: create privacy list
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
	
	qDebug( "Got folder: %s, obj: %i, parent: %i, seq: %i.", folder.name.ascii(), folder.id, folder.parentId, folder.sequence );
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
	contact.dn = current->value().toString();
	emit gotContact( contact );
	Field::MultiField * details = fl.findMultiField( NM_A_FA_USER_DETAILS );
	Field::FieldList detailsFields = details->fields();
	ContactDetails cd = extractUserDetails( detailsFields );
	cd.dn = contact.dn.lower(); // HACK: lowercased DN
	emit gotContactUserDetails( cd );
}

ContactDetails LoginTask::extractUserDetails( Field::FieldList & fields )
{
	ContactDetails cd;
	cd.status = GroupWise::Invalid;
	// read the supplied fields, set metadata and status.
	Field::SingleField * sf;
	if ( ( sf = fields.findSingleField ( NM_A_SZ_AUTH_ATTRIBUTE ) ) )
		cd.authAttribute = sf->value().toString();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_DN ) ) )
		cd.dn =sf->value().toString().lower(); // HACK: lowercased DN
	if ( ( sf = fields.findSingleField ( "CN" ) ) )
		cd.cn = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Given Name" ) ) )
		cd.givenName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Surname" ) ) )
		cd.surname = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Full Name" ) ) )
		cd.fullName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_STATUS ) ) )
		cd.status = sf->value().toInt();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_MESSAGE_BODY ) ) )
		cd.awayMessage = sf->value().toString();
	return cd;
}

#include "logintask.moc"
