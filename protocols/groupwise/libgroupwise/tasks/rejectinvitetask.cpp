/*
    Kopete Groupwise Protocol
    rejectinvitetask.cpp - Decline an invitation to chat 

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

#include "rejectinvitetask.h"

RejectInviteTask::RejectInviteTask(Task* parent): RequestTask(parent)
{
}

RejectInviteTask::~RejectInviteTask()
{
}

void RejectInviteTask::reject( const GroupWise::ConferenceGuid & guid )
{
	Field::FieldList lst, tmp;
	tmp.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, guid ) );
	lst.append( new Field::MultiField( Field::NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	createTransfer( "rejectconf", lst );
}

#include "rejectinvitetask.moc"
