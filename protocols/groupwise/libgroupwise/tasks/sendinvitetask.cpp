/*
    Kopete Groupwise Protocol
    sendinvitetask.cpp - invites someone to join a conference

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
#include "sendinvitetask.h"
#include <QStringList>

SendInviteTask::SendInviteTask(Task* parent): RequestTask(parent)
{
}

SendInviteTask::~SendInviteTask()
{
}

void SendInviteTask::invite( const GroupWise::ConferenceGuid & guid, const QStringList & invitees, const GroupWise::OutgoingMessage & msg)
{
	Field::FieldList lst, tmp;
	tmp.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, guid ) );
	lst.append( new Field::MultiField( Field::NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	QStringList::const_iterator end = invitees.end();
	for ( QStringList::const_iterator it = invitees.begin(); it != end; ++it )
		lst.append( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_DN, *it ) );
	if ( !msg.message.isEmpty() )
		lst.append( new Field::SingleField( Field::NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, msg.message ) );
	createTransfer( "sendinvite", lst );
}
