// filetransfertask.h

// Copyright (C)  2006

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without fdeven the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

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
	void gotAccept();
	void gotCancel();
	void error( int, const QString & );
	void askIncoming( QString c, QString f, DWORD s, QString d, QString i );
	void getTransferManager( Kopete::TransferManager ** );
	void processed( unsigned int );
	void fileComplete();

private slots:
	void readyAccept(); //serversocket got connection
	void socketError( int );
	void socketRead();
	void socketConnected();
	void write();

private:
	void sendReq();
	bool listen();
	bool validFile();
	Oscar::Message makeFTMsg();
	OFT makeOft();
	void sendOft( OFT );
	void oftPrompt();
	void oftAck();
	void oftDone();
	void parseReq( Buffer b );
	void saveData(); //save incoming data to disk
	void doConnect(); //attempt connection to other user (direct or redirect)
	void proxyInit(); //send init command to proxy server
	void doneConnect();
	void oftRead(); //handle incoming oft packet
	void proxyRead(); //handle incoming proxy packet
	void connectFailed(); //tries another method of connecting


	enum Action { Send, Receive };
	Action m_action;
	QFile m_file;
	QString m_contactName; //other person's username
	QString m_selfName; //my username
	QByteArray m_cookie; //icbm cookie for this transfer
	KServerSocket *m_ss; //listens for direct connections
	KBufferedSocket *m_connection; //where we actually send file data
	QTimer m_timer; //if we're idle too long, then give up
	DWORD m_size; //incoming filesize
	QString m_name; //sender's filename without path
	DWORD m_bytes; //file bytes sent/received
	WORD m_port; //to connect to
	QByteArray m_ip; //to connect to
	bool m_proxy; //are we using a proxy?
	bool m_proxyRequester; //did we choose to request the proxy?
	enum State { Default, Connecting, ProxySetup, Receiving };
	State m_state;
};

#endif
