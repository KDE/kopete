// oftmetatransfer.cpp

// Copyright (C)  2006

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#include "oftmetatransfer.h"

#include <QtCore/QFileInfo>
#include <QtNetwork/QTcpSocket>

#include <kdebug.h>

#include "ofttransfer.h"
#include "oftprotocol.h"

#define BUFFER_SIZE 32768

OftMetaTransfer::OftMetaTransfer( const QByteArray& cookie, const QStringList &files, const QString &dir, QTcpSocket *socket )
: m_file( this ), m_socket( socket ), m_state( SetupReceive )
{
	//filetransfertask is responsible for hooking us up to the ui
	//we're responsible for hooking up the socket and timer
	connect( m_socket, SIGNAL(readyRead()), this, SLOT(socketRead()) );
	connect( m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
	         this, SLOT(socketError(QAbstractSocket::SocketError)) );

	initOft();
	m_oft.cookie = cookie;
	m_files = files;
	m_dir = dir;
}

OftMetaTransfer::OftMetaTransfer( const QByteArray& cookie, const QStringList& files, QTcpSocket *socket )
: m_file( this ), m_socket( socket ), m_state( SetupSend )
{
	//filetransfertask is responsible for hooking us up to the ui
	//we're responsible for hooking up the socket and timer
	connect( m_socket, SIGNAL(readyRead()), this, SLOT(socketRead()) );
	connect( m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
	         this, SLOT(socketError(QAbstractSocket::SocketError)) );

	initOft();
	m_oft.cookie = cookie;
	for ( int i = 0; i < files.size(); ++i )
	{
		QFileInfo fileInfo( files.at(i) );
		m_oft.totalSize += fileInfo.size();
	}
	m_oft.fileCount = files.size();
	m_files = files;
}

void OftMetaTransfer::start()
{
	if ( m_files.count() == 0 )
	{
		doCancel();
		return;
	}

	//filesLeft is decremented in prompt
	m_oft.filesLeft = m_oft.fileCount + 1;
	prompt();
}

OftMetaTransfer::~OftMetaTransfer()
{
	if( m_socket )
	{
		m_socket->close();
		delete m_socket;
		m_socket = 0;
	}
	kDebug(OSCAR_RAW_DEBUG) << "really done";
}
/*
bool OftMetaTransfer::validFile()
{
	if ( m_action == Send )
	{
		if ( ! m_file.exists() )
		{
			emit error( KIO::ERR_DOES_NOT_EXIST, m_file.fileName() );
			return 0;
		}
		if ( m_file.size() == 0 )
		{
			emit error( KIO::ERR_COULD_NOT_READ, i18n("file is empty: ") + m_file.fileName() );
			return 0;
		}
		if ( ! QFileInfo( m_file ).isReadable() )
		{
			emit error( KIO::ERR_CANNOT_OPEN_FOR_READING, m_file.fileName() );
			return 0;
		}
	}
	else //receive
	{ //note: opening for writing clobbers the file
		if ( m_file.exists() )
		{
			if ( ! QFileInfo( m_file ).isWritable() )
			{ //it's there and readonly
				emit error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, m_file.fileName() );
				return 0;
			}
		}
		else if ( ! QFileInfo( QFileInfo( m_file ).toLocalFile() ).isWritable() )
		{ //not allowed to create it
				emit error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, m_file.fileName() );
				return 0;
		}
	}
	return true;
}
*/

void OftMetaTransfer::socketError( QAbstractSocket::SocketError e )
{
	QString desc = m_socket->errorString();
	kWarning(OSCAR_RAW_DEBUG) << "socket error: " << e << " : " << desc;
	emit transferError( e, desc );
}

void OftMetaTransfer::socketRead()
{
	if ( m_state == Receiving ) //raw file data
		saveData();
	else //oft packet
		readOft();
}

void OftMetaTransfer::readOft()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	QByteArray raw = m_socket->readAll(); //is this safe?
	OftProtocol p;
	uint b=0;
	//remember we're responsible for freeing this!
	OftTransfer *t = static_cast<OftTransfer*>( p.parse( raw, b ) );
	OFT data = t->data();
	kDebug(OSCAR_RAW_DEBUG) << "checksum: " << data.checksum;
	kDebug(OSCAR_RAW_DEBUG) << "sentChecksum: " << data.sentChecksum;

	switch( data.type )
	{
	case 0x101:
		handleReceiveSetup( data );
		break;
	case 0x202:
		handleSendSetup( data );
		break;
	case 0x204:
		handleSendDone( data );
		break;
	case 0x205:
		handleSendResumeRequest( data );
		break;
	case 0x106:
		handleReceiveResumeSetup( data );
		break;
	case 0x207:
		handleSendResumeSetup( data );
		break;
	default:
		kWarning(OSCAR_RAW_DEBUG) << "unknown type " << data.type;
	}

	delete t;
}

void OftMetaTransfer::initOft()
{
	//set up the default values for the oft
	m_oft.type = 0; //invalid
	m_oft.cookie = 0;
	m_oft.fileSize = 0;
	m_oft.modTime = 0;
	m_oft.checksum = 0xFFFF0000; //file checksum
	m_oft.bytesSent = 0;
	m_oft.sentChecksum = 0xFFFF0000; //checksum of transmitted bytes
	m_oft.flags = 0x20; //flags; 0x20=not done, 1=done
	m_oft.fileName.clear();
	m_oft.fileCount = 1;
	m_oft.filesLeft = 1;
	m_oft.partCount = 1;
	m_oft.partsLeft = 1;
	m_oft.totalSize = 0;
}

void OftMetaTransfer::handleReceiveSetup( const OFT &oft )
{
	if ( m_state != SetupReceive )
		return;

	kDebug(OSCAR_RAW_DEBUG) << "prompt" << endl
		<< "\tmysize " <<  m_file.size() << endl
		<< "\tsendersize " << oft.fileSize << endl;
	//do we care about anything *in* the prompt?
	//just the checksum.

	m_oft.checksum = oft.checksum;
	m_oft.modTime = oft.modTime;
	m_oft.fileCount = oft.fileCount;
	m_oft.filesLeft = oft.filesLeft;
	m_oft.partCount = oft.partCount;
	m_oft.partsLeft = oft.partsLeft;
	m_oft.totalSize = oft.totalSize;
	m_oft.fileName = oft.fileName;
	m_oft.bytesSent = oft.bytesSent;
	m_oft.fileSize = oft.fileSize;

	int currentFileIndex = oft.fileCount - oft.filesLeft;
	if ( currentFileIndex < m_files.count() )
		m_file.setFileName( m_files.at( currentFileIndex ) );
	else
		m_file.setFileName( m_dir + oft.fileName );

	emit fileStarted( m_oft.fileName, m_file.fileName() );
	emit fileStarted( m_file.fileName(), m_oft.fileSize );
	if ( m_file.size() > 0 && m_file.size() <= oft.fileSize )
	{
		m_oft.sentChecksum = fileChecksum( m_file );
		if ( m_file.size() < oft.fileSize )
		{ //could be a partial file
			resume();
			return;
		}
		else if ( m_oft.checksum == m_oft.sentChecksum )
		{ //apparently we've already got it
			//TODO: set bytesSent?
			done(); //don't redo checksum
			return;
		}

		//if we didn't break then we need the whole file
		m_oft.sentChecksum = 0xffff0000;
	}

	m_file.open( QIODevice::WriteOnly );
	//TODO what if open failed?
	ack();
}

void OftMetaTransfer::handleReceiveResumeSetup( const OFT &oft )
{
	if ( m_state != SetupReceive )
		return;

	kDebug(OSCAR_RAW_DEBUG) << "sender resume" << endl
		<< "\tfilesize\t" << oft.fileSize << endl
		<< "\tmodTime\t" << oft.modTime << endl
		<< "\tbytesSent\t" << oft.bytesSent << endl
		<< "\tflags\t" << oft.flags << endl;

	QIODevice::OpenMode flags;
	if ( oft.bytesSent ) //yay, we can resume
	{
		flags = QIODevice::WriteOnly | QIODevice::Append;
	}
	else
	{ //they insist on sending the whole file :(
		flags = QIODevice::WriteOnly;
		m_oft.sentChecksum = 0xffff0000;
		m_oft.bytesSent = 0;
	}

	m_file.open( flags );
	//TODO what if open failed?
	rAck();
}

void OftMetaTransfer::handleSendSetup( const OFT &oft )
{
	if ( m_state != SetupSend )
		return;

	kDebug(OSCAR_RAW_DEBUG) << "ack";
	emit fileStarted( m_file.fileName(), oft.fileName );
	emit fileStarted( m_file.fileName(), oft.fileSize );

	//time to send real data
	//TODO: validate file again, just to be sure
	m_file.open( QIODevice::ReadOnly );
	m_state = Sending;

	//use bytesWritten to trigger writes
	connect( m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(write()) );
	write();
}

void OftMetaTransfer::handleSendResumeSetup( const OFT &oft )
{
	Q_UNUSED(oft);

	if ( m_state != SetupSend )
		return;

	kDebug(OSCAR_RAW_DEBUG) << "resume ack";
	//TODO: validate file again, just to be sure
	m_file.open( QIODevice::ReadOnly );
	m_file.seek( m_oft.bytesSent );
	m_state = Sending;

	//use bytesWritten to trigger writes
	connect( m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(write()) );
	write();
}

void OftMetaTransfer::handleSendResumeRequest( const OFT &oft )
{
	if ( m_state != SetupSend )
		return;

	kDebug(OSCAR_RAW_DEBUG) << "receiver resume" << endl
		<< "\tfilesize\t" << oft.fileSize << endl
		<< "\tmodTime\t" << oft.modTime << endl
		<< "\tbytesSent\t" << oft.bytesSent << endl
		<< "\tflags\t" << oft.flags << endl;

	if ( fileChecksum( m_file, oft.bytesSent ) == oft.sentChecksum )
	{
		m_oft.sentChecksum = oft.sentChecksum;
		m_oft.bytesSent = oft.bytesSent; //ok, we can resume this
	}

	rAgree();
}

void OftMetaTransfer::handleSendDone( const OFT &oft )
{
	kDebug(OSCAR_RAW_DEBUG) << "done";
	emit fileFinished( m_file.fileName(), oft.bytesSent );

	disconnect( m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(write()) );
	if ( oft.sentChecksum != m_oft.checksum )
		kDebug(OSCAR_RAW_DEBUG) << "checksums do not match!";

	if ( m_oft.filesLeft > 1 )
	{ // Ready for next file
		m_state = SetupSend;
		prompt();
	}
	else
	{ // Last file, ending connection
		connect( m_socket, SIGNAL(disconnected()), this, SLOT(emitTransferCompleted()) );
		m_socket->disconnectFromHost();
	}
}

void OftMetaTransfer::write()
{
	if ( m_socket->bytesToWrite() )
		return; //give hte socket time to catch up

	//an arbitrary amount to send each time.
	char data[BUFFER_SIZE];

	m_file.seek( m_oft.bytesSent );
	int read = m_file.read( data, BUFFER_SIZE );
	if( read == -1 )
	{ //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << "failed to read :(";
		return;
	}

	int written = m_socket->write( data, read );
	if( written == -1 )
	{ //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << "failed to write :(";
		return;
	}

	m_oft.sentChecksum = chunkChecksum( data, written,
	                                    m_oft.sentChecksum, m_oft.bytesSent & 1);
	m_oft.bytesSent += written;
	
	//tell the ui
	emit fileProcessed( m_oft.bytesSent, m_oft.fileSize );
	if ( m_oft.bytesSent >= m_oft.fileSize )
	{
		m_file.close();
		disconnect( m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(write()) );
		//now we sit and do nothing until either an OFT Done
		//arrives or the user cancels.
		//we *should* always get the OFT done right away.
	}
}

void OftMetaTransfer::saveData()
{
	QByteArray raw = m_socket->readAll(); //is this safe?
	int written = m_file.write( raw );
	if( written == -1 )
	{ //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << "failed to write :(";
		return;
	}

	m_oft.sentChecksum = chunkChecksum( raw.constData(), raw.size(),
	                                    m_oft.sentChecksum, m_oft.bytesSent & 1);
	m_oft.bytesSent += written;
	if ( written != raw.size() )
	{	//FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << "didn't write everything we read";
		doCancel();
	}

	//tell the ui
	emit fileProcessed( m_oft.bytesSent, m_oft.fileSize );
	if ( m_oft.bytesSent >= m_oft.fileSize )
	{
		m_file.close();
		done();
	}

}

void OftMetaTransfer::sendOft()
{
	//now make a transfer out of it
	OftTransfer t( m_oft );
	int written = m_socket->write( t.toWire() );

	if( written == -1 ) //FIXME: handle this properly
		kDebug(OSCAR_RAW_DEBUG) << "failed to write :(";
}

void OftMetaTransfer::prompt()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	m_oft.type = 0x0101; //type = prompt

	m_oft.filesLeft--;
	const int index = m_oft.fileCount - m_oft.filesLeft;

	m_file.setFileName( m_files.at( index ) );
	QFileInfo fileInfo( m_file );

	m_oft.modTime = fileInfo.lastModified().toTime_t();
	m_oft.fileSize = fileInfo.size();
	m_oft.fileName = fileInfo.fileName();
	m_oft.checksum = fileChecksum( m_file );
	m_oft.sentChecksum = 0xFFFF0000;
	m_oft.bytesSent = 0;
	sendOft();
	//now we wait for the other side to ack
}

void OftMetaTransfer::ack()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	m_oft.type = 0x0202; //type = ack
	sendOft();
	m_state = Receiving;
}

void OftMetaTransfer::rAck()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	m_oft.type = 0x0207; //type = resume ack
	sendOft();
	m_state = Receiving;
}

void OftMetaTransfer::done()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	m_oft.type = 0x0204; //type = done
	if ( m_oft.sentChecksum != m_oft.checksum )
		kDebug(OSCAR_RAW_DEBUG) << "checksums do not match!";

	emit fileFinished( m_file.fileName(), m_oft.bytesSent );
	if ( m_oft.filesLeft == 1 )
		m_oft.flags = 1;

	sendOft();

	if ( m_oft.filesLeft > 1 )
	{ //Ready for next file
		m_state = SetupReceive;
	}
	else
	{ //Last file, ending connection
		m_state = Done;

		connect( m_socket, SIGNAL(disconnected()), this, SLOT(emitTransferCompleted()) );
		m_socket->disconnectFromHost();
	}
}

void OftMetaTransfer::resume()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	m_oft.type = 0x0205; //type = resume
	m_oft.bytesSent = m_file.size();
	sendOft();
}

void OftMetaTransfer::rAgree()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	m_oft.type = 0x0106; //type = sender resume
	sendOft();
}

void OftMetaTransfer::doCancel()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	//stop our timer in case we were sending stuff
	disconnect( m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(write()) );
	m_socket->close();
	deleteLater();
}

void OftMetaTransfer::emitTransferCompleted()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	
	emit transferCompleted();
	deleteLater(); //yay, it's ok to kill everything now
}

Oscar::DWORD OftMetaTransfer::fileChecksum( QFile& file, int bytes ) const
{
	Oscar::DWORD checksum = 0xFFFF0000;
	char data[BUFFER_SIZE];
	int read;
	int totalRead = 0;

	file.open( QIODevice::ReadOnly );
	while ( (read = file.read( data, BUFFER_SIZE )) > 0 )
	{
		if ( bytes != -1 && (totalRead + read) >= bytes )
		{
			read = bytes - totalRead;
			checksum = chunkChecksum( data, read, checksum, totalRead & 1);
			break;
		}
		else
		{
			checksum = chunkChecksum( data, read, checksum, totalRead & 1);
		}
		totalRead += read;
	}
	file.close();
	
	if ( read == -1 )
		return 0xFFFF0000;
	
	return checksum;
}

Oscar::DWORD OftMetaTransfer::chunkChecksum( const char *buffer, int bufferSize,
                                             Oscar::DWORD checksum, bool shiftIndex ) const
{
	//code adapted from Miranda's oft_calc_checksum
	const int evenIndex = (shiftIndex) ? 1 : 0;
	
	checksum = (checksum >> 16) & 0xffff;
	for ( int i = 0; i < bufferSize; i++ )
	{
		quint16 val = (uchar)buffer[i];
		
		if ( (i & 1) == evenIndex )
			val = val << 8;
		
		if (checksum < val)
			checksum -= val + 1;
		else // simulate carry
			checksum -= val;
	}
	checksum = ((checksum & 0x0000ffff) + (checksum >> 16));
	checksum = ((checksum & 0x0000ffff) + (checksum >> 16));
	return checksum << 16;
}

#include "oftmetatransfer.moc"

