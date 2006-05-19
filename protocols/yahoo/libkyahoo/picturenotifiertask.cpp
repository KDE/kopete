/*
    Kopete Yahoo Protocol
    Notifies about buddy icons

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "picturenotifiertask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qstringlist.h>
#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>

PictureNotifierTask::PictureNotifierTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
}

PictureNotifierTask::~PictureNotifierTask()
{

}

bool PictureNotifierTask::take( Transfer* transfer )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;
	
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;

	switch( t->service() )
	{	
		case Yahoo::ServicePictureStatus:
			parsePictureStatus( t );
		break;
		case Yahoo::ServicePictureChecksum:
			parsePictureChecksum( t );
		break;
		case Yahoo::ServicePicture:
			parsePicture( t );
		break;
		case Yahoo::ServicePictureUpload:
			parsePictureUploadResponse( t );
		break;
		default:
		break;
	}	

	return true;
}

bool PictureNotifierTask::forMe( Transfer* transfer ) const
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if ( t->service() == Yahoo::ServicePictureChecksum ||
		t->service() == Yahoo::ServicePicture ||
		t->service() == Yahoo::ServicePictureUpdate ||
		t->service() == Yahoo::ServicePictureUpload ||
		t->service() == Yahoo::ServicePictureStatus )
		return true;
	else
		return false;
}

void PictureNotifierTask::parsePictureStatus( YMSGTransfer *t )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString	nick;		/* key = 4 */
	int state;		/* key = 213  */

	nick = t->firstParam( 4 );
	state = t->firstParam( 213 ).toInt();
	
	emit pictureStatusNotify( nick, state );
}

void PictureNotifierTask::parsePictureChecksum( YMSGTransfer *t )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString	nick;		/* key = 4 */
	int checksum;		/* key = 192  */

	nick = t->firstParam( 4 );
	checksum = t->firstParam( 192 ).toInt();
	
	if( nick != client()->userId() )
		emit pictureChecksumNotify( nick, checksum );
}

void PictureNotifierTask::parsePicture( YMSGTransfer *t )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString	nick;		/* key = 4 */
	int type;		/* key = 13: 1 = request, 2 = notification */
	QString url;		/* key = 20 */
	int checksum;		/* key = 192  */

	nick = t->firstParam( 4 );
	url = t->firstParam( 20 );
	checksum = t->firstParam( 192 ).toInt();
	type = t->firstParam( 13 ).toInt();
	
	if( type == 1 )
		emit pictureRequest( nick );
	else if( type == 2 )
		emit pictureInfoNotify( nick, KURL( url ), checksum );
}

void PictureNotifierTask::parsePictureUploadResponse( YMSGTransfer *t )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString url;
	QString error;

	url = t->firstParam( 20 );
	error = t->firstParam( 16 );
	
	if( !error.isEmpty() )
		client()->notifyError(i18n("The picture was not successfully uploaded"), error, Client::Error );

	if( !url.isEmpty() )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Emitting url: " << url << endl;
		emit pictureUploaded( url );
	}
}

#include "picturenotifiertask.moc"
