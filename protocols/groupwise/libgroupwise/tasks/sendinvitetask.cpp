//
// C++ Implementation: sendinvitetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "sendinvitetask.h"

SendInviteTask::SendInviteTask(Task* parent): RequestTask(parent)
{
}

SendInviteTask::~SendInviteTask()
{
}

void SendInviteTask::invite( const QString & guid, const QStringList & invitees, const GroupWise::OutgoingMessage & msg)
{
	Field::FieldList lst, tmp;
	tmp.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, guid ) );
	lst.append( new Field::MultiField( NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	QValueListConstIterator<QString> end = invitees.end();
	for ( QValueListConstIterator<QString> it = invitees.begin(); it != end; ++it )
		lst.append( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_DN, *it ) );
	lst.append( new Field::SingleField( NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, /*substitute*/ msg.rtfMessage ) );
	createTransfer( "sendinvite", lst );
}
