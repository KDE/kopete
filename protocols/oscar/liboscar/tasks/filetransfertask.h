/*
    Kopete Oscar Protocol
    File Transfer Task

    Copyright 2006 Chani Armitage <chanika@gmail.com>
    Copyright 2007 Matt Rogers <mattr@kde.org>

    Kopete ( c ) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    based on ssidata.h and ssidata.cpp ( c ) 2002 Tom Linsky <twl6@po.cwru.edu>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or ( at your option ) any later version.    *
    *                                                                       *
    *************************************************************************
*/


#ifndef FILETRANSFERTASK_H
#define FILETRANSFERTASK_H

#include "task.h"

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtNetwork/QAbstractSocket>

class QTcpServer;
class QTcpSocket;

class KJob;
class Transfer;
namespace Oscar
{
	class Message;
}

class FileTransferTask : public Task
{
Q_OBJECT
public:
	/** create an incoming filetransfer */
	FileTransferTask( Task* parent, const QString& contact, const QString& self, QByteArray cookie, Buffer b );
	/** create an outgoing filetransfer */
	FileTransferTask( Task* parent, const QString& contact, const QString& self, const QStringList& files );
	~FileTransferTask();

	QString internalId() const;
	QString contactName() const;
	QString fileName() const;
	Oscar::WORD fileCount() const;
	Oscar::DWORD totalSize() const;
	QString description() const;

	//! Task implementation
	void onGo();
	bool take( Transfer* transfer );
	bool take( int type, QByteArray cookie, Buffer b );
	bool takeAutoResponse( int type, QByteArray cookie, Buffer* b );

public slots:
	void doCancel();
	void doAccept( const QString &localDirecotry );
	void doAccept( const QStringList &localFileNames );
	void timeout();

signals:
	void transferCancelled();
	void transferError( int errorCode, const QString &error );
	void transferProcessed( unsigned int totalSent );
	void transferFinished();

	void nextFile( const QString& sourceFile, const QString& destinationFile );
	void nextFile( const QString& fileName, unsigned int fileSize );
	void fileProcessed( unsigned int bytesSent, unsigned int fileSize );

	void sendMessage( const Oscar::Message &msg );
	
	void cancelOft();

private slots:
	void readyAccept(); //serversocket got connection
	void socketError( QAbstractSocket::SocketError );
	void proxyRead();
	void socketConnected();

	void fileProcessedOft( unsigned int bytesSent, unsigned int fileSize );
	void fileFinishedOft( const QString& fileName, unsigned int fileSize );
	void errorOft( int errorCode, const QString &error );
	void doneOft(); //oft told us it's done

private:
	enum Action { Send, Receive };
	void init( Action act );
	void sendReq();
	bool listen();
	bool validFile( const QString& file );
	bool validDir( const QString& dir );
	Oscar::Message makeFTMsg();
	void initOft();
	void parseReq( Buffer b );
	void doConnect(); //attempt connection to other user (direct or redirect)
	void proxyInit(); //send init command to proxy server
	void doneConnect();
	void connectFailed(); //tries another method of connecting
	void doOft();
	QString parseDescription( const QByteArray &description ) const;

	Oscar::OFTRendezvous m_oftRendezvous;

	Action m_action;
	QString m_contactName; //other person's username
	QString m_selfName; //my username
	QString m_desc; //file description
	QTcpServer *m_tcpServer; //listens for direct connections
	QTcpSocket *m_connection; //where we actually send file data
	QTimer m_timer; //if we're idle too long, then give up
	Oscar::WORD m_port; //to connect to
	QByteArray m_ip; //to connect to
	QByteArray m_altIp; //to connect to if m_ip fails
	bool m_proxy; //are we using a proxy?
	bool m_proxyRequester; //did we choose to request the proxy?
	enum State { Default, Listening, Connecting, ProxySetup, OFT, Done };
	State m_state;
	unsigned int m_fileFinishedBytes;
};

#endif
//kate: space-indent off; tab-width 4; indent-mode csands;
