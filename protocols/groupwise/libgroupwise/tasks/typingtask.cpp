/*
    Kopete Groupwise Protocol
    typingtask.cpp - sends typing notifications to the server

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

//#include "eventtransfer.h"

#include "typingtask.h"

TypingTask::TypingTask(Task* parent): RequestTask(parent)
{
}


TypingTask::~TypingTask()
{
}

void TypingTask::typing( const GroupWise::ConferenceGuid & conferenceGuid, const bool typing )
{
	Field::FieldList typingNotification, outgoingList;
	typingNotification.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, conferenceGuid ) );
	typingNotification.append( new Field::SingleField( Field::NM_A_SZ_TYPE, 0, NMFIELD_TYPE_UTF8, 
				QString::number( typing ? GroupWise::UserTyping : GroupWise::UserNotTyping ) ) );
	outgoingList.append( new Field::MultiField( Field::NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, typingNotification ) );
	createTransfer( "sendtyping", outgoingList );
}

#include "typingtask.moc"
