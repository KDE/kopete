//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "gwfield.h" 

#include "updatecontacttask.h"

using namespace GroupWise; 

UpdateContactTask::UpdateContactTask(Task* parent): UpdateItemTask(parent)
{
}


UpdateContactTask::~UpdateContactTask()
{
}

void UpdateContactTask::renameContact( const QString & newName, const QValueList<ContactItem> & contactInstances )
{
	// build a list of delete, add fields that removes each instance on the server and then readds it with the new name
	Field::FieldList lst;
	const QValueList<ContactItem>::ConstIterator end = contactInstances.end();
	for( QValueList<ContactItem>::ConstIterator it = contactInstances.begin(); it != end; ++it )
	{
		Field::FieldList contactFields;
		contactFields.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, (*it).id ) );
		contactFields.append( new Field::SingleField( NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, (*it).parentId ) );
		contactFields.append( new Field::SingleField( NM_A_SZ_SEQUENCE_NUMBER, 0, NMFIELD_TYPE_UTF8, (*it).sequence ) );
		if ( !(*it).dn.isNull() )
			contactFields.append( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, (*it).dn ) );
		if ( !(*it).displayName.isNull() )
			contactFields.append( new Field::SingleField( NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, (*it).displayName ) );
		lst.append( 
			new Field::MultiField( NM_A_FA_CONTACT, NMFIELD_METHOD_DELETE, 0, NMFIELD_TYPE_ARRAY, contactFields ) );
	}
	for( QValueList<ContactItem>::ConstIterator it = contactInstances.begin(); it != end; ++it )
	{
		Field::FieldList contactFields;
		contactFields.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, (*it).id ) );
		contactFields.append( new Field::SingleField( NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, (*it).parentId ) );
		contactFields.append( new Field::SingleField( NM_A_SZ_SEQUENCE_NUMBER, 0, NMFIELD_TYPE_UTF8, (*it).sequence ) );
		if ( !(*it).dn.isNull() )
			contactFields.append( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, (*it).dn ) );
		contactFields.append( new Field::SingleField( NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, newName ) );
		lst.append( 
			new Field::MultiField( NM_A_FA_CONTACT, NMFIELD_METHOD_ADD, 0, NMFIELD_TYPE_ARRAY, contactFields ) );
	}
	//lst.dump( true );
	UpdateItemTask::item( lst );
}

#include "updatecontacttask.moc"
