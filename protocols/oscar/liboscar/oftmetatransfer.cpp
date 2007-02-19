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

#include <kbufferedsocket.h>
#include <qstring.h>
#include <kdebug.h>
#include "connection.h"
#include "ofttransfer.h"
#include "oftprotocol.h"
#include "oscarutils.h"
#include <typeinfo>
#include <qfileinfo.h>
#include <klocale.h>


OftMetaTransfer::OftMetaTransfer( OFT oft, const QString& fileName, KBufferedSocket *connection )
: m_oft(oft), m_file( fileName, this ), m_connection( connection ), 
	m_timer( this ), m_state( SetupReceive )
{
	//filetransfertask is responsible for hooking us up to the ui
	//we're responsible for hooking up the connection and timer
	connect( m_connection, SIGNAL( readyRead() ), this, SLOT( socketRead() ) );
	connect( m_connection, SIGNAL( gotError( int ) ), this, SLOT( socketError( int ) ) );
}

void OftMetaTransfer::start()
{
	m_state = SetupSend;
	prompt();
}

OftMetaTransfer::~OftMetaTransfer()
{
	if( m_connection )
	{
		delete m_connection;
		m_connection = 0;
	}
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "really done" << endl;
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
		else if ( ! QFileInfo( QFileInfo( m_file ).path() ).isWritable() )
		{ //not allowed to create it
				emit error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, m_file.fileName() );
				return 0;
		}
	}
	return true;
}
*/

void OftMetaTransfer::socketError( int e )
{ //FIXME: do something
	QString desc = m_connection->errorString();
	kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "socket error: " << e << " : " << desc << endl;
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
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	QByteArray raw = m_connection->readAll(); //is this safe?
	OftProtocol p;
	uint b=0;
	//remember we're responsible for freeing this!
	OftTransfer *t = static_cast<OftTransfer*>( p.parse( raw, b ) );
	OFT data = t->data();
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "checksum: " << data.checksum << endl;
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "sentChecksum: " << data.sentChecksum << endl;
	switch( data.type )
	{
	 case 0x101:
		 if ( m_state != SetupReceive )
			 break;
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "prompt" << endl
			<< "\tmysize " <<  m_file.size() << endl
			<< "\tsendersize " << m_oft.fileSize << endl;
		//do we care about anything *in* the prompt?
		//just the checksum.
		m_oft.checksum = data.checksum;
		m_oft.modTime = data.modTime;
		if ( m_file.size() > 0 && m_file.size() <= m_oft.fileSize )
		{
			m_oft.sentChecksum = checksum();
			if ( m_file.size() < m_oft.fileSize )
			{ //could be a partial file
				resume();
				break;
			}
			else if ( m_oft.checksum == m_oft.sentChecksum )
			{ //apparently we've already got it
				//TODO: set bytesSent?
				done(); //don't redo checksum
				break;
			}

			//if we didn't break then we need the whole file
			m_oft.sentChecksum = 0xffff0000;
		}
		m_file.open( QIODevice::WriteOnly );
		//TODO what if open failed?
		ack();
		break;
	 case 0x202:
		 if ( m_state != SetupSend )
			 break;
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "ack" << endl;
		//time to send real data
		//TODO: validate file again, just to be sure
		m_file.open( QIODevice::ReadOnly );
		m_state = Sending;
		//use timer to trigger writes
		connect( &m_timer, SIGNAL( timeout() ), this, SLOT( write() ) );
		m_timer.start(0);
		break;
	 case 0x204:
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "done" << endl;
		m_connection->close();
		emit fileComplete();
		m_timer.stop();
		if ( data.sentChecksum != checksum() )
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "checksums do not match!" << endl;
		deleteLater();
		break;
	 case 0x205:
		 if ( m_state != SetupSend )
			 break;
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "receiver resume" << endl 
			<< "\tfilesize\t" << data.fileSize << endl
			<< "\tmodTime\t" << data.modTime << endl
			<< "\tbytesSent\t" << data.bytesSent << endl
			<< "\tflags\t" << data.flags << endl;
		if ( checksum( data.bytesSent ) == data.sentChecksum )
		//ok, we can resume this
			m_oft.bytesSent = data.bytesSent;
		rAgree();
		break;
	 case 0x106:{
		 if ( m_state != SetupReceive )
			 break;
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "sender resume" << endl
			<< "\tfilesize\t" << data.fileSize << endl
			<< "\tmodTime\t" << data.modTime << endl
			<< "\tbytesSent\t" << data.bytesSent << endl
			<< "\tflags\t" << data.flags << endl;
		QIODevice::OpenMode flags;
		if ( data.bytesSent ) //yay, we can resume
			flags = QIODevice::WriteOnly | QIODevice::Append;
		else
		{ //they insist on sending the whole file :(
			flags = QIODevice::WriteOnly;
			m_oft.sentChecksum = 0xffff0000;
			m_oft.bytesSent = 0;
		}
		m_file.open( flags ); 
		//TODO what if open failed?
		rAck();
		break;}
	 case 0x207:
		 if ( m_state != SetupSend )
			 break;
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "resume ack" << endl;
		//TODO: validate file again, just to be sure
		m_file.open( QIODevice::ReadOnly );
		m_file.seek( m_oft.bytesSent );
		m_state = Sending;
		//use timer to trigger writes
		connect( &m_timer, SIGNAL( timeout() ), this, SLOT( write() ) );
		m_timer.start(0);
		break;
	 default:
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "unknown type " << data.type << endl;
	}

	delete t;
}

void OftMetaTransfer::write()
{
	if ( m_connection->bytesToWrite() )
		return; //give hte socket time to catch up
	//an arbitrary amount to send each time.
	int max = 256;
	char data[256];
	int read = m_file.read( data, max );
	if( read == -1 )
	{ //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "failed to read :(" << endl;
		return;
	}

	int written = m_connection->write( data, read );
	if( written == -1 )
	{ //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "failed to write :(" << endl;
		return;
	}

	m_oft.bytesSent += written;
	if ( written != read ) //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "didn't write everything we read" << endl;
	//tell the ui
	emit processed( m_oft.bytesSent );
	if ( m_oft.bytesSent >= m_oft.fileSize )
	{
		m_file.close();
		m_timer.stop();
		//now we sit and do nothing until either an OFT Done
		//arrives or the user cancels.
		//we *should* always get the OFT done right away.
	}
}

void OftMetaTransfer::saveData()
{
	QByteArray raw = m_connection->readAll(); //is this safe?
	int written = m_file.write( raw );
	if( written == -1 )
	{ //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "failed to write :(" << endl;
		return;
	}
	m_oft.bytesSent += written;
	if ( written != raw.size() ) //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "didn't write everything we read" << endl;
	//tell the ui
	emit processed( m_oft.bytesSent );
	if ( m_oft.bytesSent >= m_oft.fileSize )
	{
		m_file.close();
		m_oft.sentChecksum = checksum();
		done();
	}

}

void OftMetaTransfer::sendOft()
{
	//now make a transfer out of it
	OftTransfer t( m_oft );
	int written = m_connection->write( t.toWire() );

	if( written == -1 ) //FIXME: handle this properly
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "failed to write :(" << endl;
}

void OftMetaTransfer::prompt()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_oft.type = 0x0101; //type = prompt
	m_oft.modTime = QFileInfo( m_file ).lastModified().toTime_t();
	m_oft.checksum = checksum();
	sendOft();
	//now we wait for the other side to ack
}

void OftMetaTransfer::ack()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_oft.type = 0x0202; //type = ack
	sendOft();
	m_state = Receiving;
}

void OftMetaTransfer::rAck()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_oft.type = 0x0207; //type = resume ack
	sendOft();
	m_state = Receiving;
}

void OftMetaTransfer::done()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_oft.type = 0x0204; //type = done
	if ( m_oft.sentChecksum != m_oft.checksum )
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "checksums do not match!" << endl;
	m_oft.flags = 1;
	sendOft();
	m_connection->close();
	m_state = Done;
	emit fileComplete();
	//wait for the oft done to be really sent
	connect( &m_timer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
	m_timer.start( 10 );
}

void OftMetaTransfer::resume()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_oft.type = 0x0205; //type = resume
	m_oft.bytesSent = m_file.size();
	sendOft();
}

void OftMetaTransfer::rAgree()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_oft.type = 0x0106; //type = sender resume
	sendOft();
}

void OftMetaTransfer::doCancel()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//stop our timer in case we were sending stuff
	m_timer.stop();
	m_connection->close();
	deleteLater();
}

void OftMetaTransfer::timeout()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	if ( m_state != Done )
		return; //can't happen
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "waiting for empty buffer..." << endl;
	if ( m_connection->bytesToWrite() == 0 )
	{
		m_timer.stop();
		deleteLater(); //yay, it's ok to kill everything now
	}
}

//FIXME: this is called more often than necessary. for large files that might be annoying.
Oscar::DWORD OftMetaTransfer::checksum( int max )
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//code adapted from joscar's FileTransferChecksum
	Oscar::DWORD check = 0x0000ffff;
	m_file.open( QIODevice::ReadOnly );

	char b;
	while( max != 0 && m_file.getChar( &b ) ) {
		Oscar::DWORD oldcheck = check;

		int val = ( b & 0xff ) << 8;
		if ( --max && m_file.getChar( &b ) )
		{
			val += ( b & 0xff );
			--max;
		}

		check -= val;

		if (check > oldcheck)
			check--;
	}
	m_file.close();

	check = ((check & 0x0000ffff) + (check >> 16));
	check = ((check & 0x0000ffff) + (check >> 16));
	check = check << 16;

	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << check << endl;
	return check;
}

#include "oftmetatransfer.moc"

