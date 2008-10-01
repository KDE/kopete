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

#include <QtCore/QFile>
#include <QtNetwork/QAbstractSocket>

#include "oscartypes.h"

class QTcpSocket;

class OftMetaTransfer : public QObject
{
Q_OBJECT
public:
	/** Receive constructor
	 * @param files list of file names with path.
	 * @param dir default directory if @p files list isn't big enough.
	 */
	OftMetaTransfer( const QByteArray& cookie, const QStringList &files, const QString& dir, QTcpSocket *socket );

	/** Send constructor */
	OftMetaTransfer( const QByteArray& cookie, const QStringList& files, QTcpSocket *socket );

	~OftMetaTransfer();

	void start(); //start sending file

public slots:
	void doCancel(); //one of the users is cancelling us

signals:
	void fileStarted( const QString& sourceFile, const QString& destinationFile );
	void fileStarted( const QString& fileName, unsigned int fileSize );
	void fileProcessed( unsigned int bytesSent, unsigned int fileSize );
	void fileFinished( const QString& fileName, unsigned int fileSize );

	void transferCompleted();
	void transferError( int errorCode, const QString &error );

private slots:
	//bool validFile();
	void socketError( QAbstractSocket::SocketError );
	void socketRead();
	void write();
	void emitTransferCompleted();

private:
	void initOft();
	void handleReceiveSetup( const Oscar::OFT &oft );
	void handleReceiveResumeSetup( const Oscar::OFT &oft );

	void handleSendSetup( const Oscar::OFT &oft );
	void handleSendResumeSetup( const Oscar::OFT &oft );
	void handleSendResumeRequest( const Oscar::OFT &oft );
	void handleSendDone( const Oscar::OFT &oft );

	void sendOft();
	void prompt();
	void ack();
	void done();
	void resume(); //receiver wants to resume partial file
	void rAgree(); //sender agrees to resume
	void rAck(); //resume ack
	void saveData(); //save incoming data to disk
	void readOft(); //handle incoming oft packet

	/** return checksum of our file, up to bytes */
	Oscar::DWORD fileChecksum( QFile& file, int bytes = -1 ) const;
	Oscar::DWORD chunkChecksum( const char *buffer, int bufferSize,
	                            Oscar::DWORD checksum, bool shiftIndex ) const;

	Oscar::OFT m_oft;

	QFile m_file;

	QString m_dir; //directory where we save files
	QStringList m_files; //list of files that we want to send

	QTcpSocket *m_socket; //where we actually send file data
	enum State { SetupReceive, SetupSend, Receiving, Sending, Done };
	State m_state;
};

#endif
