/*
    Kopete Oscar Protocol
    File Transfer Task

    Copyright 2006 Chani Armitage <chanika@gmail.com>
    Copyright 2007 Matt Rogers <mattr@kde.org>

    Kopete ( c ) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

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

#include <qfile.h>
#include <qtimer.h>
#include "task.h"
#include "oscarmessage.h"

namespace KNetwork
{
	class KServerSocket;
	class KBufferedSocket;
}
typedef KNetwork::KServerSocket KServerSocket;
typedef KNetwork::KBufferedSocket KBufferedSocket;
class Transfer;
namespace Kopete
{
	class Transfer;
	class FileTransferInfo;
	class TransferManager;
}

class FileTransferTask : public Task
{
Q_OBJECT
public:
	/** create an incoming filetransfer */
	FileTransferTask( Task* parent, const QString& contact, const QString& self, QByteArray cookie, Buffer b );
	/** create an outgoing filetransfer */
	FileTransferTask( Task* parent, const QString& contact, const QString& self, const QString &fileName, Kopete::Transfer *transfer );
	~FileTransferTask();

	//! Task implementation
	void onGo();
	bool take( Transfer* transfer );
	bool take( int type, QByteArray cookie, Buffer b );

public slots:
	void doCancel();
	void doCancel( const Kopete::FileTransferInfo &info );
	void doAccept( Kopete::Transfer*, const QString & );
	void timeout();

signals:
	void sendMessage( const Oscar::Message &msg );
	void askIncoming( QString c, QString f, DWORD s, QString d, QString i );
	void getTransferManager( Kopete::TransferManager ** );
	void gotCancel();
	void error( int, const QString & );
	void processed( unsigned int );
	void fileComplete();
	void cancelOft();

private slots:
	void readyAccept(); //serversocket got connection
	void socketError( int );
	void proxyRead();
	void socketConnected();
	void doneOft(); //oft told us it's done

private:
	enum Action { Send, Receive };
	void init( Action act );
	void sendReq();
	bool listen();
	bool validFile();
	Oscar::Message makeFTMsg();
	void initOft();
	void parseReq( Buffer b );
	void doConnect(); //attempt connection to other user (direct or redirect)
	void proxyInit(); //send init command to proxy server
	void doneConnect();
	void connectFailed(); //tries another method of connecting
	void doOft();

	OFT m_oft;
	
	Action m_action;
	QFile m_file;
	QString m_contactName; //other person's username
	QString m_selfName; //my username
	QString m_desc; //file description
	KServerSocket *m_ss; //listens for direct connections
	KBufferedSocket *m_connection; //where we actually send file data
	QTimer m_timer; //if we're idle too long, then give up
	WORD m_port; //to connect to
	QByteArray m_ip; //to connect to
	QByteArray m_altIp; //to connect to if m_ip fails
	bool m_proxy; //are we using a proxy?
	bool m_proxyRequester; //did we choose to request the proxy?
	enum State { Default, Listening, Connecting, ProxySetup, Done };
	State m_state;
};

#endif
//kate: space-indent off; tab-width 4; indent-mode csands;
