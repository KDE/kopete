/*
    Kopete Yahoo Protocol
    Notifies about incoming filetransfers

    Copyright (c) 2006 André Duffeck <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "filetransfernotifiertask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <kdebug.h>

FileTransferNotifierTask::FileTransferNotifierTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
}

FileTransferNotifierTask::~FileTransferNotifierTask()
{

}

bool FileTransferNotifierTask::take( Transfer* transfer )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;
	
	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);

	if( t->service() == Yahoo::ServiceP2PFileXfer ||
		t->service() == Yahoo::ServicePeerToPeer )
		declineP2P( t );	
	else if( t->service() ==  Yahoo::ServiceFileTransfer )
		parseFileTransfer( t );

	return true;
}

bool FileTransferNotifierTask::forMe( Transfer *transfer ) const
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if( t->service() == Yahoo::ServiceP2PFileXfer ||
		t->service() == Yahoo::ServicePeerToPeer ||
		t->service() == Yahoo::ServiceFileTransfer
	)
		return true;
	else
		return false;
}

void FileTransferNotifierTask::parseFileTransfer( YMSGTransfer *t )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString from;		/* key = 4  */
	QString to;		/* key = 5  */
	QString url;		/* key = 20  */
	long expires;		/* key = 38  */
	QString msg;		/* key = 14  */
	QString filename;	/* key = 27  */
	unsigned long size;	/* key = 28  */

	from = t->firstParam( 4 );
	to = t->firstParam( 5 );
	url = t->firstParam( 20 );
	expires = t->firstParam( 38 ).toLong();
	msg = t->firstParam( 14 );
	filename = t->firstParam( 27 );
	size = t->firstParam( 28 ).toULong();


	if( url.isEmpty() )
		return;

	unsigned int left = url.findRev( '/' ) + 1;
	unsigned int right = url.findRev( '?' );
	filename = url.mid( left, right - left );

	emit incomingFileTransfer( from, url, expires, msg, filename, size );
}

void FileTransferNotifierTask::declineP2P( YMSGTransfer *t )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	Q_UNUSED( t );
}

#include "filetransfernotifiertask.moc"
