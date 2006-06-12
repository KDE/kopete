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

FileTransferTask::FileTransferTask( Task* parent, const QString& contact, QByteArray cookie, Buffer b  )
:Task( parent ), m_action( Receive ), m_contact( contact ), m_cookie( cookie ), m_ss(0), m_connection(0), m_timer( this )
{
	kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "uh, we don't actually support receiving yet" << endl;
}

FileTransferTask::FileTransferTask( Task* parent, const QString& contact, const QString &fileName )
:Task( parent ), m_action( Send ), m_localFile( fileName, this ), m_contact( contact ), m_ss(0), m_connection(0), m_timer( this )
{
}

FileTransferTask::~FileTransferTask()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
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
	{ //TODO: Kopete::TransferManager::askIncomingTransfer
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "not implemented!" << endl;
		doCancel();
		return;
	}
	if ( m_contact.isEmpty() || (! validFile() ) )
	{
		setSuccess( 0 );
		return;
	}

	sendFile();
}

bool FileTransferTask::validFile()
{
	//TODO: tell hte user if any of this fails
	if ( ! m_localFile.exists() )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "file doesn't exist" << endl;
		return 0;
	}
	if ( m_localFile.size() == 0 )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "file is empty" << endl;
		return 0;
	}
	if ( ! m_localFile.open( QIODevice::ReadOnly ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "could not open file" << endl;
		return 0;
	}
	m_localFile.close();
	return true;
}

bool FileTransferTask::take( Transfer* transfer )
{
	Q_UNUSED(transfer);
	return false;
}

bool FileTransferTask::take( int type, QByteArray cookie )
{
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

void FileTransferTask::slotReadyAccept()
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
	//now we can finally send the first OFT packet.
	oftPrompt();
}

void FileTransferTask::slotSocketError( int e )
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "socket error: " << e << ": " << m_ss->errorString() << endl;
}

void FileTransferTask::oftPrompt()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "not implemented yet! " << endl;
	doCancel();
}

void FileTransferTask::doCancel()
{
	Oscar::Message msg;
	makeFTMsg( msg );
	msg.setReqType( 1 );

	emit sendMessage( msg );
	setSuccess( true );
}

void FileTransferTask::doAccept()
{
	m_timer.start();
	Oscar::Message msg;
	makeFTMsg( msg );
	msg.setReqType( 2 );
	emit sendMessage( msg );

	//FIXME: uh, should probably connect or something.
}

void FileTransferTask::timeout()
{
	//nothing's happened for ages - assume we're dead.
	//so tell the user, send off a cancel, and die
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	doCancel();
}

void FileTransferTask::sendFile()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//listen for connections
	m_ss = new KServerSocket( "5190", this ); //FIXME: don't hardcode port
	connect( m_ss, SIGNAL(readyAccept()), this, SLOT(slotReadyAccept()) );
	connect( m_ss, SIGNAL(gotError(int)), this, SLOT(slotSocketError(int)) );
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
	msg.setFile( m_localFile.size(), m_localFile.fileName() );
	//FIXME: strip path from name

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


