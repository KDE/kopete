/*
    Kopete Groupwise Protocol
    sendmessagetask.cpp - sends a message to a conference

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

#include "sendmessagetask.h"

SendMessageTask::SendMessageTask(Task* parent): RequestTask(parent)
{
}


SendMessageTask::~SendMessageTask()
{
}

void SendMessageTask::message( const QStringList & recipientDNList, const OutgoingMessage & msg )
{
	// Assumes the conference is instantiated, unlike Gaim's nm_send_message
	Field::FieldList lst, tmp, msgBodies;
	// list containing GUID
	tmp.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, msg.guid ) );
	lst.append( new Field::MultiField( Field::NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	msgBodies.append( new Field::SingleField( Field::NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, msg.rtfMessage ) );
	// message body type indicator / separator?
 	msgBodies.append( new Field::SingleField( Field::NM_A_UD_MESSAGE_TYPE, 0, NMFIELD_TYPE_UDWORD, 0 ) );
	// message body plaintext
	msgBodies.append( new Field::SingleField( Field::NM_A_SZ_MESSAGE_TEXT, 0, NMFIELD_TYPE_UTF8, msg.message ) );
	// list containing message bodies
	lst.append( new Field::MultiField( Field::NM_A_FA_MESSAGE, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, msgBodies ) );
	// series of participants (may be empty )
	QStringList::ConstIterator end = recipientDNList.end();
	for ( QStringList::ConstIterator it = recipientDNList.begin(); it != end; ++it )
		lst.append( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_DN, *it ) );
	createTransfer( "sendmessage", lst );
}
