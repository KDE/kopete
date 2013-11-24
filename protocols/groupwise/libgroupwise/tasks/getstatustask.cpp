/*
    Kopete Groupwise Protocol
    getstatustask.cpp - fetch a contact's details from the server

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

#include "getstatustask.h"

#include "response.h"

GetStatusTask::GetStatusTask(Task* parent): RequestTask(parent)
{
}

GetStatusTask::~GetStatusTask()
{
}

void GetStatusTask::userDN( const QString & dn )
{
	m_userDN = dn;
	// set up Transfer
	Field::FieldList lst;
	// changed from USERID to DN as per Gaim/GWIM
	lst.append( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, m_userDN ) );
	createTransfer( "getstatus", lst );
}

bool GetStatusTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	
	Field::FieldList responseFields = response->fields();
	responseFields.dump( true );
	// parse received details and signal like billio
	Field::SingleField * sf = 0;
	quint16 status;
	sf = responseFields.findSingleField( Field::NM_A_SZ_STATUS );
	if ( sf )
	{
		// As of Sept 2004 the server always responds with 2 (Available) here, even if the sender is not
		// This must be because the sender is not on our contact list but has sent us a message.
		// TODO: Check that the change to sending DNs above has fixed this problem.
		status = sf->value().toInt();
		// unfortunately getstatus doesn't give us an away message so we pass QString() here
		emit gotStatus( m_userDN, status, QString() );
		setSuccess();
	}	
	else
		setError();	
	return true;
}

#include "getstatustask.moc"
