//
// C++ Implementation: sendmessagetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "request.h"
#include "requestfactory.h"

#include "sendmessagetask.h"

SendMessageTask::SendMessageTask(Task* parent): RequestTask(parent)
{
}


SendMessageTask::~SendMessageTask()
{
}

void SendMessageTask::message( const QString & guid, const QStringList & recipientDNList, const Message & msg )
{
	// Assumes the conference is instantiated, unlike Gaim's nm_send_message
	QCString command = "sendmessage";
	Request * sendRequest = client()->requestFactory()->request( command );
	Field::FieldList lst, tmp;
	// list containing GUID
	tmp.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, guid ) );
	lst.append( new Field::MultiField( NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	// message body as rtf : FIXME RTFIZE TEXT - HOPE 
	lst.append( new Field::SingleField( NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, QString("") ) );
	// message body type indicator / separator?
	lst.append( new Field::SingleField( NM_A_UD_MESSAGE_TYPE, 0, NMFIELD_TYPE_UDWORD, 0 ) );
	// message body plaintext
	lst.append( new Field::SingleField( NM_A_SZ_MESSAGE_TEXT, 0, NMFIELD_TYPE_UTF8, msg ) );
	// series of participants (may be empty )
	QValueListConstIterator<QString> end = recipientDNList.end();
	for ( QValueListConstIterator<QString> it = recipientDNList.begin(); it != end; ++it )
		lst.append( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_DN, *it ) );
	sendRequest->setFields( lst );
	setTransfer( sendRequest );
}

void SendMessageTask::onGo()
{	
	qDebug( "SendMessageTask::onGo() - sending sendmessage fields" );
	send( static_cast<Request *>( transfer() ) );
}
