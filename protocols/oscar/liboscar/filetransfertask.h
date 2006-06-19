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
	FileTransferTask( Task* parent, const QString& contact, QByteArray cookie, Buffer b );
	/** create an outgoing filetransfer */
	FileTransferTask( Task* parent, const QString& contact, const QString &fileName, Kopete::Transfer *transfer );
	~FileTransferTask();

	//! Task implementation
	void onGo();
	bool take( Transfer* transfer );
	bool take( int type, QByteArray cookie );

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

private slots:
	void readyAccept(); //serversocket got connection
	void socketError( int );
	void socketRead();
	void socketClosed();
	void write();

private:
	void sendFile();
	void makeFTMsg( Oscar::Message &msg ); //add required data to msg
	bool validFile();
	void oftPrompt();
	void parseReq( Buffer b );

	enum Action { Send, Receive };
	Action m_action;
	QFile m_file;
	QString m_contact;
	QByteArray m_cookie; //icbm cookie for this transfer
	KServerSocket *m_ss; //listens for direct connections
	KBufferedSocket *m_connection; //where we actually send file data
	QTimer m_timer; //if we're idle too long, then give up
	DWORD m_size; //incoming filesize
	QString m_name; //sender's filename without path
	DWORD m_bytes; //file bytes sent/received
};

#endif
