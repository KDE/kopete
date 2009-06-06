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

#ifndef FILETRANSFERNOTIFIERTASK_H
#define FILETRANSFERNOTIFIERTASK_H

#include "task.h"
#include "yahootypes.h"

class QString;
class YMSGTransfer;
class QPixmap;

/**
@author André Duffeck
*/
class FileTransferNotifierTask : public Task
{
Q_OBJECT
public:
	FileTransferNotifierTask(Task *parent);
	~FileTransferNotifierTask();
	
	bool take(Transfer *transfer);

protected:
	virtual bool forMe( const Transfer *transfer ) const;
signals:
	void incomingFileTransfer( const QString &who, const QString &url, long expires, const QString &msg ,
	const QString &fname, unsigned long size, const QPixmap &preview );
private:
	void parseFileTransfer( YMSGTransfer *transfer );
	void parseFileTransfer7( YMSGTransfer *transfer );
	void acceptFileTransfer( YMSGTransfer *t );
	void parseFileTransfer7Info( YMSGTransfer *YMSGtransfer );
};

#endif
