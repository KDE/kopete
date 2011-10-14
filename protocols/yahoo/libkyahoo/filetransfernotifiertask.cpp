/*
    Kopete Yahoo Protocol
    Notifies about incoming filetransfers

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>

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

#include <QFile>
#include <QPixmap>
#include <kdebug.h>
#include <kcodecs.h>

using namespace KYahoo;

FileTransferNotifierTask::FileTransferNotifierTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

FileTransferNotifierTask::~FileTransferNotifierTask()
{

}

bool FileTransferNotifierTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;
	
	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);

	if( t->service() == Yahoo::ServiceFileTransfer )
		parseFileTransfer( t );
	else if( t->service() == Yahoo::ServiceFileTransfer7 )
		parseFileTransfer7( t );
	else if( t->service() == Yahoo::ServicePeerToPeer )
		acceptFileTransfer( t );
		

	return true;
}

bool FileTransferNotifierTask::forMe( const Transfer *transfer ) const
{
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if( t->service() == Yahoo::ServiceP2PFileXfer ||
	    t->service() == Yahoo::ServicePeerToPeer ||
	    t->service() == Yahoo::ServiceFileTransfer ||
	    (t->service() == Yahoo::ServiceFileTransfer7 &&
	     t->firstParam(222).toInt() == 1)
	)
		return true;
	else
		return false;
}

void FileTransferNotifierTask::parseFileTransfer( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

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



	if( from.startsWith( "FILE_TRANSFER_SYSTEM" ) )
	{
		client()->notifyError( "Fileupload result received.", msg, Client::Notice );
		return;
	}	
	
	if( url.isEmpty() )
		return;
	

	unsigned int left = url.lastIndexOf( '/' ) + 1;
	unsigned int right = url.lastIndexOf( '?' );
	filename = url.mid( left, right - left );

	emit incomingFileTransfer( from, url, expires, msg, filename, size, QPixmap() );
}

void FileTransferNotifierTask::parseFileTransfer7( YMSGTransfer *t )
{ 
	kDebug(YAHOO_RAW_DEBUG) ;

	QString from;		/* key = 4  */
	QString to;		/* key = 5  */
	QString url;		/* key = 20  */
	long expires;		/* key = 38  */
	QString msg;		/* key = 14  */
	QString filename;	/* key = 27  */
	unsigned long size;	/* key = 28  */
	QByteArray preview;	/* key = 267 */
	QPixmap previewPixmap;
	
	if( t->firstParam( 222 ).toInt() == 2 )
		return;					// user cancelled the file transfer

	from = t->firstParam( 4 );
	to = t->firstParam( 5 );
	url = t->firstParam( 265 );
	msg = t->firstParam( 14 );
	expires = t->firstParam( 38 ).toLong();
	filename = t->firstParam( 27 );
	size = t->firstParam( 28 ).toULong();
	preview = QByteArray::fromBase64( t->firstParam( 267 ) );

	if( preview.size() > 0 )
	{
		previewPixmap.loadFromData( preview );
	}

	emit incomingFileTransfer( from, url, expires, msg, filename, size, previewPixmap );
}

void FileTransferNotifierTask::acceptFileTransfer( YMSGTransfer *transfer )
{
	kDebug(YAHOO_RAW_DEBUG) ;
	
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePeerToPeer);
	t->setId( client()->sessionID() );
	t->setParam( 4, client()->userId().toLocal8Bit() );
	t->setParam( 5, transfer->firstParam( 4 ) );
	t->setParam( 11, transfer->firstParam( 11 ) );

	send( t );
}

#include "filetransfernotifiertask.moc"
