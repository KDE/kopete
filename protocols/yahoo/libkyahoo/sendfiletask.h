/*
    Kopete Yahoo Protocol
    Send a file

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

#ifndef SENDFILETASK_H
#define SENDFILETASK_H

#include "task.h"
#include <kurl.h>
#include <qfile.h>

class QString;
namespace KNetwork{
	class KStreamSocket;
}

/**
@author André Duffeck
*/
class SendFileTask : public Task
{
	Q_OBJECT
public:
	SendFileTask(Task *parent);
	~SendFileTask();
	
	virtual void onGo();
	
	void setTarget( const QString &to );
	void setMessage( const QString &msg );
	void setFileUrl( KURL url );
	void setTransferId( unsigned int transferId );

signals:
	void bytesProcessed( unsigned int, unsigned int );
	void complete( unsigned int );
	void error( unsigned int, int, const QString & );

private slots:
	void initiateUpload();
	void connectSucceeded();
	void connectFailed( int );
	void transmitData();
	void canceled( unsigned int );

private:
	QString m_msg;
	QString m_target;
	KURL m_url;
	QFile m_file;
	unsigned int m_transferId;
	unsigned int m_transmitted;
	KNetwork::KStreamSocket *m_socket;
};

#endif
