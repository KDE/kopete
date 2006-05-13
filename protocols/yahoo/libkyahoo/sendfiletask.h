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
	class KBufferedSocket;
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

signals:
	void bytesProcessed( unsigned int );
	void complete();
	void error( int, const QString & );

private slots:
	void initiateUpload();
	void connectSucceeded();
	void connectFailed( int );
	void transmitData();

private:
	QString m_msg;
	QString m_target;
	KURL m_url;
	QFile m_file;
	unsigned int m_transmitted;
	KNetwork::KBufferedSocket *m_socket;
};

#endif
