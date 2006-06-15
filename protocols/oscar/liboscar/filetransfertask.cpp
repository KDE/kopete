// filetransfertask.cpp

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

#include "filetransfertask.h"

#include <kserversocket.h>
#include <kbufferedsocket.h>
#include <krandom.h>
#include <qstring.h>
#include <kdebug.h>
#include "buffer.h"
#include "connection.h"
#include "transfer.h"
#include "oscarutils.h"
#include <typeinfo>
#include "kopetetransfermanager.h"
#include <qfileinfo.h>

FileTransferTask::FileTransferTask( Task* parent, const QString& contact, QByteArray cookie, Buffer b  )
:Task( parent ), m_action( Receive ), m_file( this ), m_contact( contact ), m_cookie( cookie ), m_ss(0), m_connection(0), m_timer( this ), m_size( 0 )
{
	kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "uh, we don't actually support receiving yet" << endl;
	parseReq( b );
	
}

FileTransferTask::FileTransferTask( Task* parent, const QString& contact, const QString &fileName, Kopete::Transfer *transfer )
:Task( parent ), m_action( Send ), m_file( fileName, this ), m_contact( contact ), m_ss(0), m_connection(0), m_timer( this )
{
	//get filename without path
	m_name = QFileInfo( fileName ).fileName();
	//hook up ui cancel
	connect( transfer , SIGNAL(transferCanceled()), this, SLOT( doCancel() ) );
	//hook up our ui signals
	connect( this , SIGNAL( gotCancel() ), transfer, SLOT( slotCancelled() ) );
	connect( this , SIGNAL( gotAccept() ), transfer, SLOT( slotAccepted() ) );
	connect( this , SIGNAL( error( int, const QString & ) ), transfer, SLOT( slotError( int, const QString & ) ) );
}

FileTransferTask::~FileTransferTask()
{
	if( m_connection )
	{
		delete m_connection;
		m_connection = 0;
	}
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "done" << endl;
}

void FileTransferTask::onGo()
{
	connect( &m_timer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
	m_timer.start( 30 * 1000 );
	if ( m_action == Receive )
	{
		//we have to send a signal because liboscar isn't supposed to know about OscarContact.
		Kopete::TransferManager *tm = 0;
		emit getTransferManager( &tm );
		connect( tm, SIGNAL( refused( const Kopete::FileTransferInfo& ) ), this, SLOT( doCancel( const Kopete::FileTransferInfo& ) ) );
		connect( tm, SIGNAL( accepted(Kopete::Transfer*, const QString &) ), this, SLOT( doAccept( Kopete::Transfer*, const QString & ) ) );

		emit askIncoming( m_contact, m_name, m_size, QString::null, m_cookie );
		//TODO: support icq descriptions
		return;
	} 
	//else, send
	if ( m_contact.isEmpty() || (! validFile() ) )
	{
		setSuccess( 0 );
		return;
	}

	sendFile();
}

void FileTransferTask::parseReq( Buffer b )
{
	//TODO: get the ip, port, etc.
	while( b.bytesAvailable() )
	{
		TLV tlv = b.getTLV();
		switch( tlv.type )
		{
		 case 0x2711: //file-specific stuff
		 {
		 	Buffer b2( tlv.data );
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "multiple file flag: " << b2.getWord() << " file count: " << b2.getWord() << endl;
			m_size = b2.getDWord();
			m_name = b2.getBlock( b2.bytesAvailable() );
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "size: " << m_size << " file: " << m_name << endl;
			break;
		 }
		 default:
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "ignoring tlv type " << tlv.type << endl;
		}
	}
}

bool FileTransferTask::validFile()
{
	//TODO: tell hte user if any of this fails
	if ( ! m_file.exists() )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "file doesn't exist" << endl;
		return 0;
	}
	if ( m_file.size() == 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "file is empty" << endl;
		return 0;
	}
	if ( ! m_file.open( QIODevice::ReadOnly ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "could not open file" << endl;
		return 0;
	}
	m_file.close();
	return true;
}

bool FileTransferTask::take( Transfer* transfer )
{
	Q_UNUSED(transfer);
	return false;
}

bool FileTransferTask::take( int type, QByteArray cookie )
{
	kDebug(14151) << k_funcinfo << "comparing to " << m_cookie << endl;
	if ( cookie != m_cookie )
		return false;

	//ooh, ooh, something happened!
	m_timer.start();
	switch( type )
	{
	 case 0: //TODO: direct transfer ain't good enough
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "we don't handle requests yet!" << endl;
		break;
	 case 1:
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "other user cancelled filetransfer :(" << endl;
		emit gotCancel();
		setSuccess( true );
		break;
	 case 2:
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "other user acceptetd filetransfer :)" << endl;
		emit gotAccept();
		break;
	 default:
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "bad request type: " << type << endl;
	}
	return true;
}

void FileTransferTask::readyAccept()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "******************" << endl;
	m_timer.start();
	m_connection = dynamic_cast<KBufferedSocket*>( m_ss->accept() );
	delete m_ss; //free up the port so others can listen
	m_ss = 0;
	if (! m_connection )
	{ //waah. FIXME: deal with this properly
		//either it wasn't buffered, or it did something weird
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "connection failed somehow." << endl;
		doCancel();
		return;
	}
	//ok, so we have a direct connection. for filetransfers. cool.
	//might be a good idea to hook up some signals&slots.
	connect( m_connection, SIGNAL( readyRead() ), this, SLOT( socketRead() ) );
	connect( m_connection, SIGNAL( closed() ), this, SLOT( socketClosed() ) );
	connect( m_connection, SIGNAL( gotError( int ) ), this, SLOT( socketError( int ) ) );
	//now we can finally send the first OFT packet.
	oftPrompt();
}

void FileTransferTask::socketError( int e )
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "socket error: " << e << endl;
}

void FileTransferTask::socketRead()
{ //TODO: actually read the data
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_timer.start();
}

void FileTransferTask::socketClosed()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "unexpected close?" << endl;
}

void FileTransferTask::oftPrompt()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//let's start with a rough ugly attempt at getting the data into a buffer
	Buffer b;
	b.addString( "OFT2" ); //protocol version
	if ( m_name.length() > 63 )
		b.addWord( m_name.length() - 63 + 256 );
	else
		b.addWord( 256 ); //minimum length
	b.addWord( 0x0101 ); //type = prompt
	b.addString( m_cookie );
	b.addDWord( 0 ); //no encryption, no compression
	b.addWord( 1 ); //total files
	b.addWord( 1 ); //files left
	b.addWord( 1 ); //total parts (macs might have 2)
	b.addWord( 1 ); //parts left
	b.addDWord( m_file.size() ); //total bytes
	b.addDWord( m_file.size() ); // size or 'bytes sent' XXX - documentation must be wrong. I'm guessing this is the size of the current file, usually same as total bytes
	b.addDWord( QFileInfo( m_file ).lastModified().toTime_t() );
	b.addDWord( 0xFFFF0000 ); //file checksum - FIXME
	b.addDWord( 0xFFFF0000 ); //recv'd resource fork checksum (mac thing)
	b.addDWord( 0 ); //resource fork size
	b.addDWord( 0 ); //creation time ( or 0 )
	b.addDWord( 0xFFFF0000 ); //resource fork checksum (mac thing)
	b.addDWord( 0 ); //bytes transmitted
	b.addDWord( 0xFFFF0000 ); //checksum of transmitted bytes - FIXME
	//'idstring'
	b.addString( "Cool FileXfer" );
	QByteArray zeros;
	zeros.fill( 0, 19 ); //32 - 13 = 19
	b.addString( zeros ); //pad to 32 bytes

	b.addByte( 0x20 ); //flags; 0x20=not done, 1=done
	b.addByte( 0x1c ); //'name offset'
	b.addByte( 0x11 ); //'size offset'
	zeros.fill( 0, 69 );
	b.addString( zeros ); //dummy block
	zeros.resize( 16 );
	b.addString( zeros ); //mac file info
	b.addWord( 0 ); //encoding 0=ascii, 2=UTF-16BE or UCS-2BE, 3= ISO-8859-1
	b.addWord( 0 ); //encoding subcode
	//sender filename
	b.addString( m_name.toAscii() );
	if ( m_name.size() < 63 )
	{ //minimum length 64
		zeros.fill( 0, 64 - m_name.size() );
		b.addString( zeros );
	}
	else
		b.addByte( 0 ); //always null-terminated string

	//yay! the big bloated header is done. so send it
	int written = m_connection->write( b.buffer() );

	if( written == -1 ) //FIXME: handle this properly
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "failed to write :(" << endl;
	else if( written < b.length() )
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "only " << written << " bytes written :(" << endl;

	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "successfully sent " << written << " bytes :)" << endl;
	//now we wait for the other side to ack
	m_timer.start();
}

void FileTransferTask::doCancel()
{
	Oscar::Message msg;
	makeFTMsg( msg );
	msg.setReqType( 1 );

	emit sendMessage( msg );
	setSuccess( true );
}


void FileTransferTask::doCancel( const Kopete::FileTransferInfo &info )
{
	//check that it's really for us
	if ( info.internalId() == m_cookie )
		doCancel();
}

void FileTransferTask::doAccept( Kopete::Transfer *t, const QString & )
{
	//check that it's really for us
	if ( t->info().internalId() != m_cookie )
		return;
	m_timer.start();
	//TODO:
	//we should unhook the old signals now; we don't need them
	//then hook up the ones for the transfer
	//and get the chosen filename from the transfer
	//but first the damn gui needs fixing.
	//oh, and, uh, should probably connect or something.

	//send an accept message
	Oscar::Message msg;
	makeFTMsg( msg );
	msg.setReqType( 2 );
	emit sendMessage( msg );
}

void FileTransferTask::timeout()
{
	//nothing's happened for ages - assume we're dead.
	//so tell the user, send off a cancel, and die
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_timer.stop();
	emit error( KIO::ERR_ABORTED, "Timeout" );
	doCancel();
}

void FileTransferTask::sendFile()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//listen for connections
	m_ss = new KServerSocket( "5190", this ); //FIXME: don't hardcode port
	connect( m_ss, SIGNAL(readyAccept()), this, SLOT(readyAccept()) );
	connect( m_ss, SIGNAL(gotError(int)), this, SLOT(socketError(int)) );
	bool success = m_ss->listen() && m_ss->error() == KNetwork::KSocketBase::NoError;
	if (! success )
	{ //uhoh... what do we do? FIXME: at least tell the user!
		//TODO: try incrementing the port#?
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "listening failed. abandoning" << endl;
		setSuccess(false);
		return;
	}
	//reset our timeout in case getting the socket took a while
	m_timer.start();

	Buffer b;
	//we get to make up an icbm cookie!
	DWORD cookie1 = KRandom::random();
	DWORD cookie2 = KRandom::random();
	b.addDWord( cookie1 );
	b.addDWord( cookie2 );
	//save the cookie for later
	m_cookie = b.buffer();

	//set up a message for sendmessagetask
	Oscar::Message msg;
	makeFTMsg( msg );

	//now set the rendezvous info
	msg.setReqType( 0 );
	msg.setPort( 5190 ); //FIXME: hardcoding bad!
	msg.setFile( m_file.size(), m_name );

	//we're done, send it off!
	emit sendMessage( msg );
}

void FileTransferTask::makeFTMsg( Oscar::Message &msg )
{
	msg.setMessageType( 3 );
	msg.setChannel( 2 );
	msg.setIcbmCookie( m_cookie );
	msg.setReceiver( m_contact );
}

#include "filetransfertask.moc"


