//
// C++ Implementation: createcontacttask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "createcontacttask.h"

CreateContactTask::CreateContactTask(Task* parent): ModifyContactListTask(parent)
{
}


CreateContactTask::~CreateContactTask()
{
}

void CreateContactTask::contactFromUserId( const QString & userId, const QString & displayName, const int parentFolder )
{
	contact( new Field::SingleField( NM_A_SZ_USERID, 0, NMFIELD_TYPE_UTF8, userId ), displayName, parentFolder );
}

void CreateContactTask::contactFromDN( const QString & dn, const QString & displayName, const int parentFolder )
{
	contact( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, dn ), displayName, parentFolder );
}

void CreateContactTask::contact( Field::SingleField * id, const QString & displayName, const int parentFolder )
{
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( parentFolder ) ) );
	// this is either a user Id or a DN
	lst.append( id );
	
	lst.append( new Field::SingleField( NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, displayName ) );
	createTransfer( "createcontact", lst );
}

#include "createcontacttask.moc"
