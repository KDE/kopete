/*
    Kopete Yahoo Protocol
    Send a file

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
	
	bool take(Transfer *transfer);

	void setTarget( const QString &to );
	void setMessage( const QString &msg );
	void setFileUrl( KUrl url );
	void setTransferId( unsigned int transferId );

signals:
	void bytesProcessed( unsigned int, unsigned int );
	void complete( unsigned int );
	void error( unsigned int, int, const QString & );

	void declined();

protected:
	bool forMe( const Transfer *transfer ) const;
	void sendFileTransferInfo();
	void parseFileTransfer( const Transfer *transfer );
	void parseTransferAccept(const Transfer *transfer);

	QString newYahooTransferId();

	/** returns true if things need to be stopped due to file transfer error or completed */
	bool fillSendBuffer();

	/** returns true if file transfer completed */ 
	bool checkTransferEnd();

private slots:
	void connectSucceeded();
	void connectFailed( int );
	void transmitData();
	void transmitHeader();
	void canceled( unsigned int );

private:
	QString m_msg;
	QString m_target;
	KUrl m_url;
	QFile m_file;
	unsigned int m_transferId;
	unsigned int m_transmitted;
	KNetwork::KStreamSocket *m_socket;

	QString m_relayHost;
	QString m_token;
	QString m_yahooTransferId;

	/** buffer containing data to be sent */
	QByteArray m_buffer;
	/** position (until m_bufferInPos) of data ready to be sent on the wire */
	int m_bufferOutPos;
	/** position where next fills from the file should happen */
	int m_bufferInPos;
};

#endif
