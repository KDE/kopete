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
// included for the event code for typing - FIXME: rationalise numeric constants
#include "eventtransfer.h"

#include "typingtask.h"

TypingTask::TypingTask(Task* parent): RequestTask(parent)
{
}


TypingTask::~TypingTask()
{
}

void TypingTask::typing( const QString & conferenceGuid, const bool typing )
{
	Field::FieldList typingNotification, outgoingList;
	typingNotification.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, conferenceGuid ) );
	typingNotification.append( new Field::SingleField( NM_A_SZ_TYPE, 0, NMFIELD_TYPE_UTF8, 
				QString::number( typing ? GroupWise::UserTyping : GroupWise::UserNotTyping ) ) );
	outgoingList.append( new Field::MultiField( NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, typingNotification ) );
	createTransfer( "sendtyping", outgoingList );
}

#include "typingtask.moc"
