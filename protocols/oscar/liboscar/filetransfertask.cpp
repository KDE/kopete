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
#include "ofttransfer.h"
#include "oftprotocol.h"
#include "oscarutils.h"
#include <typeinfo>
#include "kopetetransfermanager.h"
#include <qfileinfo.h>

FileTransferTask::FileTransferTask( Task* parent, const QString& contact, QByteArray cookie, Buffer b  )
:Task( parent ), m_action( Receive ), m_file( this ), m_contact( contact ), m_cookie( cookie ), m_ss(0), m_connection(0), m_timer( this ), m_size( 0 ), m_bytes( 0 ), m_port( 0 )
{
	kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "uh, we don't actually support receiving yet" << endl;
	parseReq( b );
	
}

FileTransferTask::FileTransferTask( Task* parent, const QString& contact, const QString &fileName, Kopete::Transfer *transfer )
:Task( parent ), m_action( Send ), m_file( fileName, this ), m_contact( contact ), m_ss(0), m_connection(0), m_timer( this ), m_bytes( 0 ), m_port( 0 )
{
	//get filename without path
	m_name = QFileInfo( fileName ).fileName();
	//hook up ui cancel
	connect( transfer , SIGNAL(transferCanceled()), this, SLOT( doCancel() ) );
	//hook up our ui signals
	connect( this , SIGNAL( gotCancel() ), transfer, SLOT( slotCancelled() ) );
	connect( this , SIGNAL( gotAccept() ), transfer, SLOT( slotAccepted() ) );
	connect( this , SIGNAL( error( int, const QString & ) ), transfer, SLOT( slotError( int, const QString & ) ) );
	connect( this , SIGNAL( processed( unsigned int ) ), transfer, SLOT( slotProcessed( unsigned int ) ) );
	connect( this , SIGNAL( fileComplete() ), transfer, SLOT( slotComplete() ) );
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
	m_timer.start( 60 * 1000 );
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
	DWORD proxy_ip = 0;
	DWORD client_ip = 0;
	QByteArray verified_ip;
	bool proxy = false;
	while( b.bytesAvailable() )
	{
		TLV tlv = b.getTLV();
		Buffer b2( tlv.data );
		switch( tlv.type )
		{
		 case 0x2711: //file-specific stuff
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "multiple file flag: " << b2.getWord() << " file count: " << b2.getWord() << endl;
			m_size = b2.getDWord();
			m_name = b2.getBlock( b2.bytesAvailable() );
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "size: " << m_size << " file: " << m_name << endl;
			break;
		 case 2:
		 	proxy_ip = b2.getDWord();
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "proxy ip " << proxy_ip << endl;
			break;
		 case 3:
		 	client_ip = b2.getDWord();
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "client ip " << client_ip << endl;
			break;
		 case 4:
		 	verified_ip = tlv.data;
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "verified ip " << verified_ip << endl;
			break;
		 case 5:
		 	m_port = b2.getWord();
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "port " << m_port << endl;
			break;
		 case 0x10:
		 	proxy = true;
			break;
		 default:
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "ignoring tlv type " << tlv.type << endl;
		}
	}

	if( proxy )
	{ //uhoh, not supported yet. TODO
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "proxy unsupported" << endl;
	}

	//for now, only try the verified ip. FIXME
	m_ip = verified_ip;

}

bool FileTransferTask::validFile()
{
	//TODO: tell hte user if any of this fails
	if ( m_action == Send )
	{
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
	}
	else //receive
	{
		if ( ! m_file.open( QIODevice::WriteOnly ) )
		{
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "could not open file" << endl;
			return 0;
		}
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
{ //FIXME: handle this properly
	kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "socket error: " << e << endl;
}

void FileTransferTask::socketRead()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_timer.start();
	QByteArray raw = m_connection->readAll(); //is this safe?
	OftProtocol p;
	uint b=0;
	//remember we're responsible for freeing this!
	OftTransfer *t = static_cast<OftTransfer*>( p.parse( raw, b ) );
	OFT data = t->data();
	switch( data.type )
	{
	 case 0x101:
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "prompt" << endl;
		//TODO: ack
		break;
	 case 0x202:
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "ack" << endl;
		//time to send real data
		//TODO: validate file again, just to be sure
		m_file.open( QIODevice::ReadOnly );
		//switch the timer over to the other function
		m_timer.disconnect();
		connect( &m_timer, SIGNAL( timeout() ), this, SLOT( write() ) );
		m_timer.start(0);
		break;
	 case 0x204:
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "done" << endl;
		emit fileComplete();
		m_timer.stop();
		setSuccess( true );
		break;
	 case 0x205: //not supported yet
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "receiver resume" << endl;
		doCancel();
		break;
	 case 0x106: //can't happen
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "sender resume" << endl;
		break;
	 case 0x207: //can't happen
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "resume ack" << endl;
		break;
	 default:
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "unknown type " << data.type << endl;
	}

	delete t;
}

void FileTransferTask::socketClosed()
{ //TODO: find out whether it was expected, and tell the user if it wasn't
	//possible cause might be the other end going offline
	//perhaps consider it an abrupt cancel
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "unexpected close?" << endl;
	setSuccess( true );
}

void FileTransferTask::write()
{
	//an arbitrary amount to send each time.
	int max = 256;
	QByteArray data = m_file.read( max );
	//TODO: what if it's empty? (probably means error)
	int written = m_connection->write( data );
	if( written == -1 )
	{ //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "failed to write :(" << endl;
		return;
	}

	m_bytes += written;
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "successfully sent " << written << " bytes, total " << m_bytes << endl;
	if ( written != data.size() ) //FIXME: handle this properly
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "didn't write everything we read" << endl;
	//tell the ui
	emit processed( m_bytes );
	if ( m_bytes >= m_file.size() )
	{
		//switch the timer over to the other function
		//we should always get OFT Done before this times out
		//or we could just finish now without waiting
		//but I want to do it this way for now
		m_timer.disconnect();
		connect( &m_timer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
		m_timer.start( 10 * 1000 );
	}
}

void FileTransferTask::oftPrompt()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//fill an OFT with data
	OFT data;
	data.type = 0x0101; //type = prompt
	data.cookie = m_cookie;
	data.fileSize = m_file.size();
	data.modTime = QFileInfo( m_file ).lastModified().toTime_t();
	data.checksum = 0xFFFF0000; //file checksum - FIXME
	data.bytesSent = 0;
	data.sentChecksum = 0xFFFF0000; //checksum of transmitted bytes
	data.flags = 0x20; //flags; 0x20=not done, 1=done
	data.fileName = m_name;

	//now make a transfer out of it
	OftTransfer t( data );
	int written = m_connection->write( t.toWire() );

	if( written == -1 ) //FIXME: handle this properly
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "failed to write :(" << endl;
	else
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
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//check that it's really for us
	if ( info.internalId() == m_cookie )
		doCancel();
}

void FileTransferTask::doAccept( Kopete::Transfer *t, const QString & localName )
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//check that it's really for us
	if ( t->info().internalId() != m_cookie )
		return;
	m_timer.start();

	//TODO: we should unhook the old transfermanager signals now

	//hook up the ones for the transfer
	connect( this , SIGNAL( gotCancel() ), t, SLOT( slotCancelled() ) );
	connect( this , SIGNAL( error( int, const QString & ) ), t, SLOT( slotError( int, const QString & ) ) );
	connect( this , SIGNAL( processed( unsigned int ) ), t, SLOT( slotProcessed( unsigned int ) ) );
	connect( this , SIGNAL( fileComplete() ), t, SLOT( slotComplete() ) );
	//and save the chosen filename
	m_file.setFileName( localName );
	if( ! validFile() )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "invalid file" << endl;
		doCancel();
		return;
	}
	//oh, and, uh, should probably connect or something.
	if ( m_ip.length() != 4 || ! m_port )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "nobody to connect to!" << endl;
		doCancel();
		return;
	}
	//ugly because ksockets demand a qstring
	QString ip = QString::number( m_ip.at(0) ) + '.' +
		QString::number( m_ip.at(1) ) + '.' +
		QString::number( m_ip.at(2) ) + '.' +
		QString::number( m_ip.at(3) );

	m_connection = new KBufferedSocket( ip, QString::number( m_port ) );
	connect( m_connection, SIGNAL( readyRead() ), this, SLOT( socketRead() ) );
	connect( m_connection, SIGNAL( closed() ), this, SLOT( socketClosed() ) );
	connect( m_connection, SIGNAL( gotError( int ) ), this, SLOT( socketError( int ) ) );
	connect( m_connection, SIGNAL( connected(const KNetwork::KResolverEntry&)), this, SLOT(socketConnected()));

	//try the direct connect
	m_connection->connect();
}

void FileTransferTask::socketConnected()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_timer.start();
	//yay! send an accept message
	Oscar::Message msg;
	makeFTMsg( msg );
	msg.setReqType( 2 );
	emit sendMessage( msg );
	//next we should get a prompt from the sender.
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


