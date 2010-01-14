/*
    Kopete Yahoo Protocol
    Notifies about buddy icons

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef PICTURENOTIFIERTASK_H
#define PICTURENOTIFIERTASK_H

#include "task.h"

#include <kurl.h>

class QString;
class YMSGTransfer;
/**
@author André Duffeck
*/
class PictureNotifierTask : public Task
{
Q_OBJECT
public:
	PictureNotifierTask(Task *parent);
	~PictureNotifierTask();
	
	bool take(Transfer *transfer);

protected:
	virtual bool forMe( const Transfer *transfer ) const;
	void parsePictureChecksum( YMSGTransfer *transfer );
	void parsePictureStatus( YMSGTransfer *transfer );
	void parsePicture( YMSGTransfer *transfer );
	void parsePictureUploadResponse( YMSGTransfer *transfer );
signals:
	void pictureStatusNotify( const QString &, int );
	void pictureChecksumNotify( const QString &, int );
	void pictureInfoNotify( const QString &, KUrl, int );
	void pictureRequest( const QString & );
	void pictureUploaded( const QString &, int );
};

#endif
