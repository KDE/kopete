/*
    Kopete Groupwise Protocol
    ChatCountsTask.cpp - Task to update chatroom participant counts

    Copyright (c) 2005      SUSE Linux Products GmbH	 http://www.suse.com

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "chatcountstask.h"

#include <qmap.h>
#include <kdebug.h>

#include "gwfield.h"
#include "response.h"

ChatCountsTask::ChatCountsTask(Task* parent): RequestTask(parent)
{
	Field::FieldList lst;
	createTransfer( "chatcounts", lst );
}


ChatCountsTask::~ChatCountsTask()
{
}

bool ChatCountsTask::take( Transfer * transfer )
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
	
	Field::FieldList responseFields = response->fields();
	Field::MultiField * resultsArray = responseFields.findMultiField( Field::NM_A_FA_RESULTS );
	if ( !resultsArray )
	{
		setError( GroupWise::Protocol );
		return true;
	}
	Field::FieldList counts = resultsArray->fields();
	const Field::FieldListIterator end = counts.end();
	for ( Field::FieldListIterator it = counts.find( Field::NM_A_FA_CHAT );
			 it != end;
			 it = counts.find( ++it, Field::NM_A_FA_CHAT ) )
	{
		Field::MultiField * mf = static_cast<Field::MultiField *>( *it );
		Field::FieldList chat = mf->fields();
		QString roomName;
		int participants = 0;
		// read the supplied fields, set metadata and status.
		Field::SingleField * sf;
		if ( ( sf = chat.findSingleField ( Field::NM_A_DISPLAY_NAME ) ) )
			roomName = sf->value().toString();
		if ( ( sf = chat.findSingleField ( Field::NM_A_UD_PARTICIPANTS ) ) )
			participants = sf->value().toInt();
		
		m_results.insert( roomName, participants );
	}
	return true;
}

QMap< QString, int > ChatCountsTask::results()
{
	return m_results;
}

#include "chatcountstask.moc"
