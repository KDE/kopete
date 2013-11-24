/*
    Kopete Groupwise Protocol
    createconferencetask.cpp - Request task that creates conferences on the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#include "createconferencetask.h"

#include "client.h"
#include "response.h"


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
	tmp.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, m_guid ) );
	lst.append( new Field::MultiField( Field::NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	// series of participants (may be empty )
	QStringList::const_iterator end = participants.end();
	for ( QStringList::const_iterator it = participants.begin(); it != end; ++it )
		lst.append( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_DN, *it ) );
	lst.append( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_DN, client()->userDN() ) );
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
	if ( response->resultCode() == GroupWise::None )
	{
		Field::MultiField * listField = responseFields.findMultiField( Field::NM_A_FA_CONVERSATION );
		Field::FieldList guidList = listField->fields();
		Field::SingleField * guidField = guidList.findSingleField( Field::NM_A_SZ_OBJECT_ID );
		m_guid = guidField->value().toString();
		setSuccess();
	}
	else
		setError( response->resultCode() );
	return true;
	
}

GroupWise::ConferenceGuid CreateConferenceTask::conferenceGUID() const
{
	return m_guid;
}

int CreateConferenceTask::clientConfId() const
{
	return m_confId;
}	

#include "createconferencetask.moc"
