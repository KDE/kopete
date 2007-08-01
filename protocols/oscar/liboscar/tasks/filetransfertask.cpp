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

#include "oscarsettings.h"
#include "filetransfertask.h"

#include <k3socketaddress.h>
#include <k3serversocket.h>
#include <k3bufferedsocket.h>
#include <krandom.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <kdebug.h>
#include "buffer.h"
#include "connection.h"
#include "ofttransfer.h"
#include "oftprotocol.h"
#include "oscarutils.h"
#include <typeinfo>
#include "kopetetransfermanager.h"
#include <qfileinfo.h>
#include <klocale.h>
#include "oftmetatransfer.h"

//receive
FileTransferTask::FileTransferTask( Task* parent, const QString& contact,
                                    const QString& self, QByteArray cookie,
                                    Buffer b  )
: Task( parent ), m_contactName( contact ), m_selfName( self ),
  m_timer( this )
{
	init( Receive );
	initOft();
	m_oftRendezvous.cookie = cookie;
	parseReq( b );
}

//send
FileTransferTask::FileTransferTask( Task* parent, const QString& contact,
                                    const QString& self, const QStringList& files,
                                    Kopete::Transfer *transfer )
:Task( parent ), m_contactName( contact ),
 m_selfName( self ), m_timer( this )
{
	init( Send );
	initOft();

	m_oftRendezvous.files = files;
	m_oftRendezvous.fileCount = files.size();
	for ( int i = 0; i < m_oftRendezvous.fileCount; ++i )
	{
		QFileInfo fileInfo( m_oftRendezvous.files.at(i) );
		m_oftRendezvous.totalSize += fileInfo.size();
	}

	if ( m_oftRendezvous.fileCount == 1 )
	{ //get filename without path
		m_oftRendezvous.fileName = QFileInfo( files.at(0) ).fileName();
	}

	//we get to make up an icbm cookie!
	Buffer b;
	Oscar::DWORD cookie = KRandom::random();
	b.addDWord( cookie );
	cookie = KRandom::random();
	b.addDWord( cookie );
	m_oftRendezvous.cookie = b.buffer();

	//hook up ui cancel
	connect( transfer , SIGNAL(transferCanceled()), this, SLOT( doCancel() ) );
	//hook up our ui signals
	connect( this , SIGNAL( gotCancel() ), transfer, SLOT( slotCancelled() ) );
	connect( this , SIGNAL( error( int, const QString & ) ), transfer, SLOT( slotError( int, const QString & ) ) );
	connect( this , SIGNAL( processed( unsigned int ) ), transfer, SLOT( slotProcessed( unsigned int ) ) );
	connect( this , SIGNAL( fileComplete() ), transfer, SLOT( slotComplete() ) );
}

void FileTransferTask::init( Action act )
{
	m_action = act;
	m_ss = 0;
	m_connection = 0;
	m_port = 0;
	m_proxy = false;
	m_proxyRequester = false;
	m_state = Default;
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
	if ( m_action == Receive )
	{
		//we have to send a signal because liboscar isn't supposed to know about OscarContact.

		Kopete::TransferManager *tm = 0;
		emit getTransferManager( &tm );
		connect( tm, SIGNAL( refused( const Kopete::FileTransferInfo& ) ), this, SLOT( doCancel( const Kopete::FileTransferInfo& ) ) );
		connect( tm, SIGNAL( accepted(Kopete::Transfer*, const QString &) ), this, SLOT( doAccept( Kopete::Transfer*, const QString & ) ) );

		emit askIncoming( m_contactName, m_oftRendezvous.fileName, m_oftRendezvous.totalSize, m_desc, m_oftRendezvous.cookie );
		return;
	}
	//else, send
	if ( m_contactName.isEmpty() )
	{
		setSuccess( 0 );
		return;
	}

	for ( int i = 0; i < m_oftRendezvous.fileCount; ++i )
	{
		if ( !validFile( m_oftRendezvous.files.at(i) ) )
		{
			setSuccess( 0 );
			return;
		}
	}

	if ( client()->settings()->fileProxy() )
	{ //proxy stage 1
		m_proxy = 1;
		m_proxyRequester = 1;
		doConnect();
	}
	else
		sendReq();
}

void FileTransferTask::parseReq( Buffer b )
{
	QByteArray proxy_ip;
	QByteArray client_ip;
	QByteArray verified_ip;
	QByteArray fileName;
	QTextCodec *c=0;
	m_altIp.clear(); //just to be sure

	while( b.bytesAvailable() )
	{
		TLV tlv = b.getTLV();
		Buffer b2( tlv.data );
		switch( tlv.type )
		{
		 case 0x2711: //file-specific stuff
			{
			if ( m_action == Send ) //then we don't care
				break;

			bool multipleFiles = (b2.getWord() == 0x02);
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "multiple file flag: " << multipleFiles << endl;
			m_oftRendezvous.fileCount = b2.getWord();
			m_oftRendezvous.totalSize = b2.getDWord();
			fileName = b2.getBlock( b2.bytesAvailable() - 1 ); //null terminated
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "file: " << fileName << endl;
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "total size: " << m_oftRendezvous.totalSize << " files: " << m_oftRendezvous.fileCount << endl;
			break;
			}
		 case 0x2712:
			c = Oscar::codecForName( tlv.data );
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "filename encoding " << tlv.data << endl;
			break;
		 case 2:
		 	proxy_ip = tlv.data;
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "proxy ip " << proxy_ip << endl;
			break;
		 case 3:
		 	client_ip = tlv.data;
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
		 case 0x0c:
			m_desc = tlv.data; //FIXME: what codec?
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "user message: " << tlv.data << endl;
			break;
		 case 0x0d:
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "default encoding " << tlv.data << endl;
			break;
		 case 0x0e:
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "language " << tlv.data << endl;
			break;
		 case 0x10:
		 	m_proxy = true;
			break;
		 default:
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "ignoring tlv type " << tlv.type << endl;
		}
	}

	if ( !c )
	{
		c = QTextCodec::codecForName( "UTF-8" );
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "no codec, assuming utf8" << endl;
	}

	if ( c )
		m_oftRendezvous.fileName = c->toUnicode( fileName );
	else
	{
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo  << "couldn't get any codec! " << endl;
		m_oftRendezvous.fileName = fileName;
	}

	if( m_proxy )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "proxy requested" << endl;
		m_ip = proxy_ip;
	}
	else if ( client_ip.isEmpty() )
		m_ip = verified_ip;
	else if ( client_ip == "\0\0\0\0" )
	{ //ip is all 0's
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "proxy??" << endl;
		//wtf... I guess it wants *me* to request a proxy?
		m_proxy = 1;
		m_proxyRequester = 1;
	}
	else
	{
		m_ip = client_ip;
		if ( verified_ip != client_ip )
			m_altIp = verified_ip;
	}

}

bool FileTransferTask::validFile( const QString& file )
{
	QFileInfo fileInfo( file );
	if ( m_action == Send )
	{
		if ( ! fileInfo.exists() )
		{
			emit error( KIO::ERR_DOES_NOT_EXIST, fileInfo.fileName() );
			return 0;
		}
		if ( fileInfo.size() == 0 )
		{
			emit error( KIO::ERR_COULD_NOT_READ, i18n("file is empty: ") + fileInfo.fileName() );
			return 0;
		}
		if ( ! fileInfo.isReadable() )
		{
			emit error( KIO::ERR_CANNOT_OPEN_FOR_READING, fileInfo.fileName() );
			return 0;
		}
	}
	else //receive
	{ //note: opening for writing clobbers the file
		if ( fileInfo.exists() )
		{
			if ( ! fileInfo.isWritable() )
			{ //it's there and readonly
				emit error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, fileInfo.fileName() );
				return 0;
			}
		}
		else if ( ! QFileInfo( fileInfo.path() ).isWritable() )
		{ //not allowed to create it
			emit error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, fileInfo.fileName() );
			return 0;
		}
	}
	return true;
}

bool FileTransferTask::validDir( const QString& dir )
{
	QFileInfo fileInfo( dir );
	if ( m_action == Receive )
	{
		if ( fileInfo.exists() && fileInfo.isDir() )
		{
			if ( !fileInfo.isWritable() )
			{ //it's there and readonly
				emit error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, dir );
				return false;
			}
		}
		else
		{ //not allowed o create it
			emit error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, dir );
			return false;
		}
	}
	return true;
}

bool FileTransferTask::take( Transfer* transfer )
{
	Q_UNUSED(transfer);
	return false;
}

bool FileTransferTask::take( int type, QByteArray cookie, Buffer b )
{
	kDebug(14151) << k_funcinfo << "comparing to " << m_oftRendezvous.cookie << endl;
	if ( cookie != m_oftRendezvous.cookie )
		return false;

	//ooh, ooh, something happened!
	switch( type )
	{
	 case 0: //direct transfer ain't good enough
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "redirect or proxy request" << endl;
		if ( m_state != Listening )
		{
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "other client is insane." << endl;
			break;
		}

		delete m_ss;
		m_ss = 0;
		parseReq( b );
		doConnect();
		break;
	 case 1:
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "other user cancelled filetransfer :(" << endl;
		emit gotCancel(); //FIXME: what if it's not hooked up?
		emit cancelOft();
		m_timer.stop();
		setSuccess( true );
		break;
	 case 2:
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "other user acceptetd filetransfer :)" << endl;
		break;
	 default:
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "bad request type: " << type << endl;
	}
	return true;
}

void FileTransferTask::readyAccept()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "******************" << endl;
	m_connection = dynamic_cast<KBufferedSocket*>( m_ss->accept() );
	m_ss->close(); //free up the port so others can listen
	m_ss->deleteLater();
	m_ss = 0;
	if (! m_connection )
	{ //either it wasn't buffered, or it did something weird
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "connection failed somehow." << endl;
		emit error( KIO::ERR_COULD_NOT_ACCEPT, QString() );
		doCancel();
		return;
	}
	//ok, so we have a direct connection. for filetransfers. cool.
	//now we go on to the OFT phase.
	doOft();
}

void FileTransferTask::doOft()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "******************" << endl;
	QObject::disconnect( m_connection, 0, 0, 0 ); //disconnect signals
	OftMetaTransfer *oft;

	if ( m_action == Receive )
		oft = new OftMetaTransfer( m_oftRendezvous.cookie, m_oftRendezvous.dir, m_connection );
	else
		oft = new OftMetaTransfer( m_oftRendezvous.cookie, m_oftRendezvous.files, m_connection );

	m_connection=0; //it's not ours any more
	//might be a good idea to hook up some signals&slots.
	connect( oft, SIGNAL(fileProcessed(unsigned int, unsigned int)), this, SIGNAL(processed(unsigned int)) );
	connect( oft, SIGNAL(transferCompleted()), this, SLOT(doneOft()) );
	connect( this, SIGNAL(cancelOft()), oft, SLOT(doCancel()) );
	//now we can finally send the first OFT packet.
	if ( m_action == Send )
		oft->start();
}

void FileTransferTask::doneOft()
{
	emit fileComplete();
	setSuccess( true );
}

void FileTransferTask::socketError( int e )
{ //FIXME: handle this properly for all cases
	QString desc;
	if ( m_ss )
		desc = m_ss->errorString();
	else if ( m_connection )
		desc = m_connection->errorString();
	kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "socket error: " << e << " : " << desc << endl;
	if ( m_state == Connecting )
	{ //connection failed, try another way
		if ( m_proxy )
		{ //fuck, we failed at a proxy! just give up
			emit error( KIO::ERR_COULD_NOT_CONNECT, desc );
			doCancel();
		}
		else
		{
			m_timer.stop();
			connectFailed();
		}
	}
}

void FileTransferTask::proxyRead()
{
	if ( m_state != ProxySetup )
	{
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "reading non-proxy data!" << endl;
	}

	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	QByteArray raw = m_connection->readAll(); //is this safe?
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	Buffer b( raw );
	Oscar::WORD length = b.getWord();
	if ( b.bytesAvailable() != length )
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "length is " << length << " but we have " << b.bytesAvailable() << "bytes!" << endl;
	b.skipBytes( 2 ); //protocol version
	Oscar::WORD command = b.getWord();
	b.skipBytes( 6 ); //4 unknown, 2 flags

	switch( command )
	{
		case 1: //error
		{
			Oscar::WORD err = b.getWord();
			QString errMsg;
			switch( err )
			{
				case 0x0d: //we did something wrong (eg. wrong username)
				case 0x0e: //we did something wronger (eg. no cookie)
					errMsg = i18n("Bad Request");
					break;
				case 0x10: //we did something else wrong
					errMsg = i18n("Request Timed Out");
					break;
				case 0x1a: //other side was too slow
					errMsg = i18n("Accept Period Timed Out");
					break;
				default:
					errMsg = i18n("Unknown Error: ") + QString::number( err );
			}

			emit error( KIO::ERR_COULD_NOT_LOGIN, errMsg );
			doCancel();
			break;
		}
		case 3: //ack
			m_port = b.getWord();
			m_ip = b.getBlock( 4 );
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "got port " << m_port << " ip " << m_ip << endl;
			//now we send a proxy request to the other side
			sendReq();
			break;
		case 5: //ready
			doneConnect();
	}
}

void FileTransferTask::initOft()
{
	//set up the default values for the oft
	m_oftRendezvous.cookie = 0;
	m_oftRendezvous.fileCount = 0;
	m_oftRendezvous.totalSize = 0;
}

void FileTransferTask::doCancel()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//tell the other side
	Oscar::Message msg = makeFTMsg();
	msg.setRequestType( 1 );
	emit sendMessage( msg );
	//stop our timer in case we were doing stuff
	m_timer.stop();
	emit cancelOft();
	setSuccess( true );
}


void FileTransferTask::doCancel( const Kopete::FileTransferInfo &info )
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//check that it's really for us
	if ( info.internalId() == QString( m_oftRendezvous.cookie ) )
		doCancel();
	else
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "ID mismatch" << endl;
}

void FileTransferTask::doAccept( Kopete::Transfer *t, const QString & localName )
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//check that it's really for us
	//XXX because qt is retarded, I can't simply compare a qstring and bytearray any more.
	//if there are 0's in hte cookie then the qstring will only contain part of it - but it should at least be consistent about that. it just slightly increases the tiny chance of a conflict
	if ( t->info().internalId() != QString( m_oftRendezvous.cookie ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "ID mismatch" << endl;
		return;
	}

	//TODO: we should unhook the old transfermanager signals now

	//hook up the ones for the transfer
	connect( t , SIGNAL(transferCanceled()), this, SLOT( doCancel() ) );
	connect( this , SIGNAL( gotCancel() ), t, SLOT( slotCancelled() ) );
	connect( this , SIGNAL( error( int, const QString & ) ), t, SLOT( slotError( int, const QString & ) ) );
	connect( this , SIGNAL( processed( unsigned int ) ), t, SLOT( slotProcessed( unsigned int ) ) );
	connect( this , SIGNAL( fileComplete() ), t, SLOT( slotComplete() ) );
	//and save the chosen filename

	/*FIXME: Should we prompt for a file name if we receive one file or only for a directory like the official
	         icq and aim clients (if we have multiple files we don't know their names at this stage) */
#ifdef __GNUC__
#warning Hack, this will be removed as soon as I add a function into Kopete::TransferManager that will prompt for a directory (Kedge)
#endif
	QFileInfo fileInfo( localName );
	m_oftRendezvous.dir = fileInfo.absolutePath() + '/';

	if( validDir( m_oftRendezvous.dir ) )
		doConnect();
	else
		doCancel();
}

void FileTransferTask::doConnect()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;

	QString host;
	if ( m_proxyRequester )
		host = "ars.oscar.aol.com";
	else
	{
		if ( m_ip.length() != 4 || ! m_port )
		{
			emit error( KIO::ERR_COULD_NOT_CONNECT, i18n("missing IP or port") );
			doCancel();
			return;
		}

		//ksockets demand a qstring
		host = KNetwork::KIpAddress( m_ip.constData(), 4 ).toString();
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "ip: " << host << endl;
	}

	//proxies *always* use port 5190; the "port" value is some retarded check
	m_connection = new KBufferedSocket( host, QString::number( m_proxy ? 5190 : m_port ) );
	connect( m_connection, SIGNAL( readyRead() ), this, SLOT( proxyRead() ) );
	connect( m_connection, SIGNAL( gotError( int ) ), this, SLOT( socketError( int ) ) );
	connect( m_connection, SIGNAL( connected(const KNetwork::KResolverEntry&)), this, SLOT(socketConnected()));

	m_state = Connecting;
	//socket doesn't seem to have its own timeout, so here's mine
	m_timer.disconnect();
	connect( &m_timer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
	m_timer.start( client()->settings()->timeout()  * 1000 );
	//try it
	m_connection->connect();
}

void FileTransferTask::socketConnected()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_timer.stop();
	if( m_proxy )
	//we need to send an init recv first
		proxyInit();
	else
		doneConnect();
}

void FileTransferTask::doneConnect()
{
	m_state = Default;
	if ( ! m_proxyRequester )
	{ //yay! send an accept message
		Oscar::Message msg = makeFTMsg();
		msg.setRequestType( 2 );
		emit sendMessage( msg );
	}
	//next the receiver should get a prompt from the sender.
	doOft();
}

void FileTransferTask::proxyInit()
{
	m_state = ProxySetup;
	//init "send" is sent by whoever requested the proxy.
	//init "recv" is sent by the other side
	//because the second person has to include the port check

	Buffer data;
	data.addBUIN( m_selfName.toLatin1() );

	if ( !m_proxyRequester ) //if 'recv'
		data.addWord( m_port );

	data.addString( m_oftRendezvous.cookie );
	data.addTLV( 0x0001, oscar_caps[CAP_SENDFILE] ); //cap tlv

	Buffer header;
	header.addWord( 10 + data.length() ); //length
	header.addWord( 0x044a ); //packet version
	header.addWord( m_proxyRequester ? 2 : 4 ); //command: 2='send' 4='recv'
	header.addDWord( 0 ); //unknown
	header.addWord( 0 ); //flags

	header.addString( data );
	//send it to the proxy server
	int written = m_connection->write( header.buffer() );

	if( written == -1 ) //FIXME: handle this properly
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "failed to write :(" << endl;

}

void FileTransferTask::timeout()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_timer.stop();
	if ( m_state == Connecting )
	{ //kbufferedsocket took too damn long
		if ( m_proxy )
		{ //fuck, we failed at a proxy! just give up
			emit error( KIO::ERR_COULD_NOT_CONNECT, i18n("Timeout") );
			doCancel();
		}
		else
			connectFailed();
		return;
	}

	//nothing's happened for ages - assume we're dead.
	//so tell the user, send off a cancel, and die
	emit error( KIO::ERR_ABORTED, i18n("Timeout") );
	doCancel();
}

void FileTransferTask::connectFailed()
{
	delete m_connection;
	m_connection = 0;
	bool proxy = client()->settings()->fileProxy();
	if (! proxy )
	{
		if ( ! m_altIp.isEmpty() )
		{ //there's another ip to try
			m_ip = m_altIp;
			m_altIp.clear();
			doConnect();
			return;
		}
		if ( m_action == Receive )
		{ //try redirect
			sendReq();
			return;
		}
	}

	//proxy stage 2 or 3
	m_proxy = 1;
	m_proxyRequester = 1;
	doConnect();
}

bool FileTransferTask::listen()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_state = Default;
	//listen for connections
	m_ss = new KServerSocket( this );
	connect( m_ss, SIGNAL(readyAccept()), this, SLOT(readyAccept()) );
	connect( m_ss, SIGNAL(gotError(int)), this, SLOT(socketError(int)) );
	bool success = false;
	int first = client()->settings()->firstPort();
	int last = client()->settings()->lastPort();
	//I don't trust the settings to be sane
	if ( last < first )
		last = first;

	for ( int i = first; i <= last; i++ )
	{ //try ports in the range (default 5190-5199)
		m_ss->setAddress( QString::number( i ) );
		if( success = ( m_ss->listen() && m_ss->error() == KNetwork::KSocketBase::NoError ) )
		{
			m_port = i;
			break;
		}
		m_ss->close();
	}
	if (! success )
	{ //uhoh... what do we do? FIXME: maybe tell the user too many filetransfers
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "listening failed. abandoning" << endl;
		emit error( KIO::ERR_COULD_NOT_LISTEN, QString::number( last ) );
		setSuccess(false);
		return false;
	}

	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "listening for connections on port " << m_port << endl;
	m_state = Listening;
	return true;
}

void FileTransferTask::sendReq()
{
	//if we're not using a proxy we need a working serversocket
	if (!( m_proxy || listen() ))
		return;

	Buffer b;
	b.addString( m_oftRendezvous.cookie );

	//set up a message for sendmessagetask
	Oscar::Message msg = makeFTMsg();

	//now set the rendezvous info
	msg.setRequestType( 0 );
	msg.setPort( m_port );
	msg.setFileName( m_oftRendezvous.fileName );
	msg.setFileCount( m_oftRendezvous.fileCount );
	msg.setFilesSize( m_oftRendezvous.totalSize );

	if ( m_proxy )
		msg.setProxy( m_ip );

	if ( m_action == Receive )
		msg.setRequestNumber( 2 );
	else if ( m_proxy && (! client()->settings()->fileProxy() ) )
		msg.setRequestNumber( 3 );

	//we're done, send it off!
	emit sendMessage( msg );
}

Oscar::Message FileTransferTask::makeFTMsg()
{
	Oscar::Message msg;
	msg.setMessageType( Oscar::MessageType::File );
	msg.setChannel( 2 ); //rendezvous
	msg.setIcbmCookie( m_oftRendezvous.cookie );
	msg.setReceiver( m_contactName );
	return msg;
}

#include "filetransfertask.moc"
//kate: space-indent off; tab-width 4; indent-mode csands;

