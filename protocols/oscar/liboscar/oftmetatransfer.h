// oftmetatransfer.h

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

#ifndef OFTMETATRANSFER_H
#define OFTMETATRANSFER_H

#include <qfile.h>
#include <qtimer.h>
#include "oscartypes.h"

namespace KNetwork
{
	class KBufferedSocket;
}
typedef KNetwork::KBufferedSocket KBufferedSocket;
namespace Kopete
{
	class FileTransferInfo;
}

using namespace Oscar;
class OftMetaTransfer : public QObject
{
Q_OBJECT
public:
	/** create filetransfer */
	OftMetaTransfer( OFT oft, const QString& fileName, KBufferedSocket *connection );
	~OftMetaTransfer();

	void start(); //start sending file

public slots:
	void doCancel(); //one of the users is cancelling us

signals:
//	void error( int, const QString & );
	void processed( unsigned int );
	void fileComplete();

private slots:
	void socketError( int );
	void socketRead();
	void write();
	void timeout();

private:
	//bool validFile();
	void sendOft();
	void prompt();
	void ack();
	void done();
	void resume(); //receiver wants to resume partial file
	void rAgree(); //sender agrees to resume
	void rAck(); //resume ack
	void saveData(); //save incoming data to disk
	void readOft(); //handle incoming oft packet
	Oscar::DWORD checksum( int max = -1 ); //return checksum of our file, up to max bytes
					//XXX this does put an arbitrary limit on file size

	OFT m_oft;
	
	QFile m_file;
	KBufferedSocket *m_connection; //where we actually send file data
	QTimer m_timer; //if we're idle too long, then give up
	enum State { SetupReceive, SetupSend, Receiving, Sending, Done };
	State m_state;
};

#endif
