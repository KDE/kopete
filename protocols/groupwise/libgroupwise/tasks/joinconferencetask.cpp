//
// C++ Implementation: joinconferencetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "response.h"

#include "joinconferencetask.h"

JoinConferenceTask::JoinConferenceTask(Task* parent): RequestTask(parent)
{
}

JoinConferenceTask::~JoinConferenceTask()
{
}

void JoinConferenceTask::join( const QString & guid )
{
	Field::FieldList lst, tmp;
	tmp.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, guid ) );
	lst.append( new Field::MultiField( NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	createTransfer( "sendinvite", lst );
}

bool JoinConferenceTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		qDebug( "JoinConferenceTask::take()" );
		Response * response = dynamic_cast<Response *>( transfer );
		Field::FieldList responseFields = response->fields();
		Field::SingleField * resultCodeField = responseFields.findSingleField( NM_A_SZ_RESULT_CODE );
		int resultCode = resultCodeField->value().toInt();
		// if the request was successful
		if ( resultCode == GroupWise::None )
		{
			// extract the list of participants and store them
			Field::MultiField * participants = responseFields.findMultiField( NM_A_FA_CONTACT_LIST );
			if ( participants )
			{
				Field::SingleField * contact = 0;
				Field::FieldList contactList = participants->fields();
				for ( Field::FieldListIterator it = contactList.find( NM_A_SZ_DN );
					 it != contactList.end();
					 it = contactList.find( ++it, NM_A_SZ_DN ) )
				{
					contact = static_cast<Field::SingleField *>( *it );
					if ( contact )
						m_participants.append( contact->value().toString() );
				}
			}
			else 
				setError( GroupWise::Protocol );
			
			setSuccess();	
		}
		else
			setError( resultCode );
		return true;
	}
	else
		return false;
}

QStringList JoinConferenceTask::participants()
{
	return m_participants;
}

#include "joinconferencetask.moc"
