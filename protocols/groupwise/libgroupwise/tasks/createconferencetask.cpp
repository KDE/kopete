//
// C++ Implementation: createconferencetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "client.h"
#include "response.h"


#include "createconferencetask.h"

CreateConferenceTask::CreateConferenceTask(Task* parent): RequestTask(parent), m_confId( 0 ), m_guid( BLANK_GUID )
{

}

CreateConferenceTask::~CreateConferenceTask()
{
}

void CreateConferenceTask::conference( const int confId, const QStringList &participants )
{
	m_confId = confId;
	Field::FieldList lst, tmp;
	// list containing blank GUID
	tmp.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, m_guid ) );
	lst.append( new Field::MultiField( NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	// series of participants (may be empty )
	QValueListConstIterator<QString> end = participants.end();
	for ( QValueListConstIterator<QString> it = participants.begin(); it != end; ++it )
		lst.append( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_DN, *it ) );
	lst.append( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_DN, client()->userDN() ) );
	createTransfer( "createconf", lst );
}

bool CreateConferenceTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	
	// if the createconf was successful, read the GUID and store it
	Field::FieldList responseFields = response->fields();
	Field::SingleField * resultCodeField = responseFields.findSingleField( NM_A_SZ_RESULT_CODE );
	int resultCode = resultCodeField->value().toInt();
	if ( resultCode == GroupWise::None )
	{
		Field::MultiField * listField = responseFields.findMultiField( NM_A_FA_CONVERSATION );
		Field::FieldList guidList = listField->fields();
		Field::SingleField * guidField = guidList.findSingleField( NM_A_SZ_OBJECT_ID );
		m_guid = guidField->value().toString();
		setSuccess();
	}
	else
		setError( resultCode );
	return true;
	
}

QString CreateConferenceTask::conferenceGUID() const
{
	return m_guid;
}

int CreateConferenceTask::clientConfId() const
{
	return m_confId;
}	

#include "createconferencetask.moc"
