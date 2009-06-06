/*
    Kopete Yahoo Protocol
    sendpicturetask.h - Send our picture or information about it

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

#ifndef SENDPICTURETASK_H
#define SENDPICTURETASK_H

#include "task.h"

class QString;
class QFile;
namespace KIO	{ 
	class Job;
	class TransferJob; 
}
namespace KNetwork	{ 
	class KBufferedSocket;
}

/**
@author André Duffeck
*/
class SendPictureTask : public Task
{
Q_OBJECT
public:
	enum Type { UploadPicture, SendChecksum, SendInformation, SendStatus };

	SendPictureTask(Task *parent);
	virtual ~SendPictureTask();
	
	virtual void onGo();

	void setType( Type type );
	void setTarget( const QString &to );
	void setFilename( const QString & );
	void setFilesize( int );
	void setPath( const QString & );
	void setChecksum( int );
	void setStatus( int );
	void setUrl( const QString & );
private:
	void initiateUpload();
	void sendChecksum();
	void sendInformation();
	void sendStatus();
private slots:
	void connectSucceeded();
	void connectFailed( int );
	void readResult();
private:
	Type m_type;
	QString m_target;
	QString m_fileName;
	int m_fileSize;
	QString m_path;
	int m_checksum;
	int m_status;
	QString m_url;
	int m_transmitted;
	QFile *m_file;
	KNetwork::KBufferedSocket *m_socket;
};

#endif
