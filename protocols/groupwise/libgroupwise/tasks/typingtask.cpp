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
#include "client.h"
// included for the event code for typing - FIXME: rationalise numeric constants
#include "eventtransfer.h"
#include "gwfield.h"
#include "request.h"
#include "requestfactory.h"

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
	typingNotification.append( new Field::SingleField( NM_A_SZ_TYPE, 0, NMFIELD_TYPE_UTF8, QString::number( typing ? NMEVT_USER_TYPING : NMEVT_USER_NOT_TYPING ) ) );
	outgoingList.append( new Field::MultiField( NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, typingNotification ) );
	QCString command = "sendtyping";
	Request * typingRequest = client()->requestFactory()->request( command );
	typingRequest->setFields( outgoingList );

	setTransfer( typingRequest );
}

void TypingTask::onGo()
{
	//cout << "TypingTask::onGo() - sending status fields" << endl;
	send( static_cast<Request *>( transfer() ) );
}

#include "typingtask.moc"
