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
	tmp.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, msg.guid ) );
	lst.append( new Field::MultiField( NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	// message body as rtf : FIXME RTFIZE TEXT - HOPE
	QString substitute("{\\rtf1\\ansi\n{\\fonttbl\\f0\\fnil Monospaced;\\f1\\fnil SansSerif;}\n\n\\f1\\fs18\\i0\\b0\\ul0\\cf0 tiredy.\\par\n}\n");
	msgBodies.append( new Field::SingleField( NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, /*substitute*/ msg.rtfMessage ) );
	// message body type indicator / separator?
 	msgBodies.append( new Field::SingleField( NM_A_UD_MESSAGE_TYPE, 0, NMFIELD_TYPE_UDWORD, 0 ) );
	// message body plaintext
	msgBodies.append( new Field::SingleField( NM_A_SZ_MESSAGE_TEXT, 0, NMFIELD_TYPE_UTF8, msg.message ) );
	// list containing message bodies
	lst.append( new Field::MultiField( NM_A_FA_MESSAGE, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, msgBodies ) );
	// series of participants (may be empty )
	QValueListConstIterator<QString> end = recipientDNList.end();
	for ( QValueListConstIterator<QString> it = recipientDNList.begin(); it != end; ++it )
		lst.append( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_DN, *it ) );
	createTransfer( "sendmessage", lst );
}
