/***************************************************************************
                          msnfiletransfersocket.cpp  -  description
                             -------------------
    begin                : mer jui 31 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "msnfiletransfersocket.h"

#include <stdlib.h>
#include <math.h>

//qt
#include <qtimer.h>

// kde
#include <kdebug.h>
#include <kserversocket.h>
#include <kbufferedsocket.h>
#include <kfiledialog.h>
#include <klocale.h>

#include "kopetetransfermanager.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "msnchatsession.h"
#include "msnswitchboardsocket.h"
#include "msnnotifysocket.h"
#include "msnaccount.h"

using namespace KNetwork;

MSNFileTransferSocket::MSNFileTransferSocket(const QString &handle, Kopete::Contact *c,bool incoming, QObject* parent)
	: MSNSocket(parent) , MSNInvitation(incoming, MSNFileTransferSocket::applicationID() , i18n("File Transfer - MSN Plugin"))
{
	m_handle=handle;
	m_kopeteTransfer=0l;
	m_file=0L;
	m_server=0L;
	m_contact=c;
	ready=true;

	QObject::connect( this, SIGNAL( socketClosed() ), this, SLOT( slotSocketClosed() ) );
	QObject::connect( this, SIGNAL( blockRead( const QByteArray & ) ), this, SLOT(slotReadBlock( const QByteArray & ) ) );
}

MSNFileTransferSocket::~MSNFileTransferSocket()
{
	delete m_file;
	delete m_server;
	kdDebug(14140) << "MSNFileTransferSocket::~MSNFileTransferSocket" <<endl;
}

void MSNFileTransferSocket::parseCommand(const QString & cmd, uint id, const QString & data)
{
	if( cmd == "VER" )
	{
		if(data.section( ' ', 0, 0 ) != "MSNFTP")
		{
			kdDebug(14140) << "MSNFileTransferSocket::parseCommand (VER): bad version: disconnect" <<endl;
			disconnect();
		}
		else
		{
			if( m_incoming )
				sendCommand( "USR", m_handle + " " + m_authcook, false );
			else
				sendCommand( "VER", "MSNFTP" , false );
		}
	}
	else if( cmd == "FIL" )
	{
		m_size=id; //data.toUInt(); //BUG: the size is take as id bye MSNSocket because it is a number

		m_downsize=0;
		m_file=new QFile(m_fileName);

		if( m_file->open( IO_WriteOnly ))
			sendCommand( "TFR" ,NULL,false);
		else
		{
			kdDebug(14140) << "MSNFileTransferSocket::parseCommand: ERROR: unable to open file - disconnect " <<endl;
			disconnect();
		}
	}
	else if( cmd == "BYE" )
	{
		kdDebug(14140) << "MSNFileTransferSocket::parseCommand : end of transfer " <<endl;
		disconnect();
	}
	else if( cmd == "USR" )
	{
		if(data.section( ' ', 1, 1 )!= m_authcook)
		{
			kdDebug(14140) << "MSNFileTransferSocket::parseCommand (USR): bad auth" <<endl;
			disconnect();
		}
		else
			sendCommand("FIL" , QString::number(size()) , false);
	}
	else if( cmd == "TFR" )
	{
		m_downsize=0;
		ready=true;
		QTimer::singleShot( 0, this, SLOT(slotSendFile()) );
	}
	else if( cmd == "CCL" )
	{
		disconnect();
	}
	else
		kdDebug(14140) << "MSNFileTransferSocket::parseCommand : unknown command " <<cmd <<endl;

//	kdDebug(14140) << "MSNFileTransferSocket::parseCommand : done " <<cmd <<endl;
}

void MSNFileTransferSocket::doneConnect()
{
	if(m_incoming)
		sendCommand( "VER", "MSNFTP", false );
	MSNSocket::doneConnect();
}

void MSNFileTransferSocket::bytesReceived(const QByteArray & head)
{
	if(head[0]!='\0')
	{
		kdDebug(14140) << "MSNFileTransferSocket::bytesReceived: transfer aborted" <<endl;
		QTimer::singleShot(0,this,SLOT(disconnect()));
	}
	unsigned int sz=(int)((unsigned char)head.data()[2])*256+(int)((unsigned char)head.data()[1]);
//	kdDebug(14140) << "MSNFileTransferSocket::bytesReceived: " << sz <<endl;
	readBlock(sz);
}

void MSNFileTransferSocket::slotSocketClosed()
{
	kdDebug(14140) << "MSNFileTransferSocket::slotSocketClose "<<  endl;
	if(m_file)
		m_file->close();
	delete m_file;
	m_file=0L;
	delete m_server;
	m_server=0L;
	if(m_kopeteTransfer)
	{
		if( (m_downsize!=m_size  || m_downsize==0 ) )
			m_kopeteTransfer->slotError(  KIO::ERR_UNKNOWN , i18n( "An unknown error occurred" ) );
		else 
			m_kopeteTransfer->slotComplete();
	}	
	emit done(this);
}

void MSNFileTransferSocket::slotReadBlock(const QByteArray &block)
{
	m_file->writeBlock( block.data(), block.size() );      // write to file

	m_downsize+=block.size();
	if(m_kopeteTransfer) m_kopeteTransfer->slotProcessed(m_downsize);
	kdDebug(14140) << "MSNFileTransferSocket  -  " << m_downsize << " of " << m_size <<" done"<<endl;

	if(m_downsize==m_size)
	{
		//the transfer seems to be finished.  
		sendCommand( "BYE" ,"16777989",false);
		//  if we are not already disconected in 30 seconds, do it.
		QTimer::singleShot( 30000 , this, SLOT(disconnect() ) );

	}
}

void MSNFileTransferSocket::setKopeteTransfer(Kopete::Transfer *kt)
{
	m_kopeteTransfer=kt;
	if(kt)
	{
		QObject::connect(kt , SIGNAL(transferCanceled()), this, SLOT(abort()));
		QObject::connect(kt,  SIGNAL(destroyed()) , this , SLOT(slotKopeteTransferDestroyed()));
	}
}

void MSNFileTransferSocket::listen(int port)
{
	m_server = new KServerSocket();

	QObject::connect( m_server, SIGNAL(readyAccept()), this,  SLOT(slotAcceptConnection()));
	m_server->setAddress(QString::number(port));

	kdDebug(14140) << "MSNFileTransferSocket::listen: about to listen"<<endl;
	bool listenResult = m_server->listen(1);
	kdDebug(14140) << "MSNFileTransferSocket::listen: result: "<<  listenResult <<endl;
	QTimer::singleShot( 60000, this, SLOT(slotTimer()) );
	kdDebug(14140) << "MSNFileTransferSocket::listen done" <<endl;
}

void MSNFileTransferSocket::slotAcceptConnection()
{
	kdDebug(14140) << "MSNFileTransferSocket::slotAcceptConnection" <<endl;
	if(!accept(m_server))
	{
		if( m_kopeteTransfer)
				m_kopeteTransfer->slotError(  KIO::ERR_UNKNOWN , i18n( "An unknown error occurred" ) );
		emit done(this);
	}
}

void MSNFileTransferSocket::slotTimer()
{
	if(onlineStatus() != Disconnected)
		return;
	kdDebug(14140) << "MSNFileTransferSocket::slotTimer: timeout "<<  endl;
	if( m_kopeteTransfer)
	{
		m_kopeteTransfer->slotError( KIO::ERR_CONNECTION_BROKEN , i18n( "Connection timed out" ) );
	}

	MSNChatSession* manager=dynamic_cast<MSNChatSession*>(m_contact->manager());
	if(manager && manager->service())
		manager->service()->sendCommand( "MSG" , "N", true, rejectMessage("TIMEOUT") );

	emit done(this);
}

void MSNFileTransferSocket::abort()
{
	if(m_incoming)
	{
		sendCommand( "CCL" , NULL ,false);
	}
	else
	{
		QByteArray bytes(3);
		bytes[0]='\1';
		bytes[1]='\0';
		bytes[2]='\0';
		sendBytes( bytes );
		m_downsize=m_size; //we don't want to send data anymore;
	}
	//the timer wait one second, the time to send the CCL or the binary header
	//retarding the disconnection keep away from a crash. (in KIO::Job::emitResult when `delete this`)
	QTimer::singleShot( 1000, this, SLOT(disconnect()) );
	ready=false;
}

void MSNFileTransferSocket::setFile( const QString &fn, long unsigned int fileSize )
{
	m_fileName=fn;
	if(!m_incoming)
	{
		if(m_file)
		{
			kdDebug(14140) << "MSNFileTransferSocket::setFileName: WARNING m_file already exists" << endl;
			delete m_file;
		}
		m_file = new QFile( fn );
		if(!m_file->open(IO_ReadOnly))
		{
			//FIXME: abort transfer here
			kdDebug(14140) << "MSNFileTransferSocket::setFileName: WARNING unable to open the file" << endl;
		}

		//If the fileSize is 0 it was not given, we are to get it from the file
		if(fileSize == 0L)
			m_size = m_file->size();
		else
			m_size = fileSize;
	}
}


void MSNFileTransferSocket::slotSendFile()
{
//	kdDebug(14140) <<"MSNFileTransferSocket::slotSendFile()" <<endl;
	if( m_downsize >= m_size)
	{
		//the transfer seems to be finished.  
		//  if we are not already disconected in 30 seconds, do it.
		QTimer::singleShot( 30000 , this, SLOT(disconnect() ) );
		return;
	}

	if(ready)
	{
		char data[2046];
		int bytesRead = m_file->readBlock( data, 2045 );
			
		QByteArray block(bytesRead+3);
//		char i1= (char)fmod( bytesRead, 256 ) ;
//		char i2= (char)floor( bytesRead / 256 ) ;
//		kdDebug(14140) << "MSNFileTransferSocket::slotSendFile: " << (int)i1 <<" + 256* "<< (int)i2 <<" = " << bytesRead <<endl;
		block[0]='\0';
		block[1]= (char)fmod( bytesRead, 256 );
		block[2]= (char)floor( bytesRead / 256 );

		for (  int f = 0; f < bytesRead; f++ )
		{
			block[f+3] = data[f];
		}

		sendBytes(block);

		m_downsize+=bytesRead;
		if(m_kopeteTransfer)
			 m_kopeteTransfer->slotProcessed(m_downsize);
		kdDebug(14140) << "MSNFileTransferSocket::slotSendFile: " << m_downsize << " of " << m_size <<" done"<<endl;
	}
	ready=false;

	QTimer::singleShot( 10, this, SLOT(slotSendFile()) );
}

void MSNFileTransferSocket::slotReadyWrite()
{
	ready=true;
	MSNSocket::slotReadyWrite();
}

QString MSNFileTransferSocket::invitationHead()
{
	QTimer::singleShot( 10 * 60000, this, SLOT(slotTimer()) );  //the user has 10 mins to accept or refuse or initiate the transfer

	return QString( MSNInvitation::invitationHead()+
					"Application-File: "+ m_fileName.right( m_fileName.length() - m_fileName.findRev( '/' ) - 1 ) +"\r\n"
					"Application-FileSize: "+ QString::number(size()) +"\r\n\r\n").utf8();
}

void MSNFileTransferSocket::parseInvitation(const QString& msg)
{
	QRegExp rx("Invitation-Command: ([A-Z]*)");
	rx.search(msg);
	QString command=rx.cap(1);
	if( msg.contains("Invitation-Command: INVITE") )
	{
		rx=QRegExp("Application-File: ([^\\r\\n]*)");
		rx.search(msg);
		QString filename = rx.cap(1);
		rx=QRegExp("Application-FileSize: ([0-9]*)");
		rx.search(msg);
		unsigned long int filesize= rx.cap(1).toUInt();

		MSNInvitation::parseInvitation(msg); //for the cookie

		Kopete::TransferManager::transferManager()->askIncomingTransfer( m_contact , filename, filesize, QString::null, QString::number( cookie() ) );

		QObject::connect( Kopete::TransferManager::transferManager(), SIGNAL( accepted( Kopete::Transfer *, const QString& ) ),this, SLOT( slotFileTransferAccepted( Kopete::Transfer *, const QString& ) ) );
		QObject::connect( Kopete::TransferManager::transferManager(), SIGNAL( refused( const Kopete::FileTransferInfo & ) ), this, SLOT( slotFileTransferRefused( const Kopete::FileTransferInfo & ) ) );

	}
	else if( msg.contains("Invitation-Command: ACCEPT") )
	{
		if(incoming())
		{
			rx=QRegExp("IP-Address: ([0-9\\.]*)");
			rx.search(msg);
			QString ip_address = rx.cap(1);
			rx=QRegExp("AuthCookie: ([0-9]*)");
			rx.search(msg);
			QString authcook = rx.cap(1);
			rx=QRegExp("Port: ([0-9]*)");
			rx.search(msg);
			QString port = rx.cap(1);

			setAuthCookie(authcook);
			connect(ip_address, port.toUInt());
		}
		else
		{
			unsigned long int auth = (rand()%(999999))+1;
			setAuthCookie(QString::number(auth));

			setKopeteTransfer(Kopete::TransferManager::transferManager()->addTransfer(m_contact, fileName(), size(),  m_contact->metaContact() ? m_contact->metaContact()->displayName() : m_contact->contactId() , Kopete::FileTransferInfo::Outgoing));

			MSNChatSession* manager=dynamic_cast<MSNChatSession*>(m_contact->manager());
			if(manager && manager->service())
			{
				MSNNotifySocket *notify=static_cast<MSNAccount*>(manager->account())->notifySocket();
				if(notify){
				
				QCString message=QString(
					"MIME-Version: 1.0\r\n"
					"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
					"\r\n"
					"Invitation-Command: ACCEPT\r\n"
					"Invitation-Cookie: " + QString::number(cookie()) + "\r\n"
					"IP-Address: " + notify->localIP()  + "\r\n"
					"Port: 6891\r\n"
					"AuthCookie: "+QString::number(auth)+"\r\n"
					"Launch-Application: FALSE\r\n"
					"Request-Data: IP-Address:\r\n\r\n").utf8();

				manager->service()->sendCommand( "MSG" , "N", true, message );
				}
			}

			listen(6891);
		}
	}
	else //CANCEL
	{
		MSNInvitation::parseInvitation(msg);
		if( m_kopeteTransfer)
			m_kopeteTransfer->slotError( KIO::ERR_ABORTED , i18n( "The remote user aborted" ) );
		emit done(this);

	}
}

void MSNFileTransferSocket::slotFileTransferAccepted(Kopete::Transfer *trans, const QString& fileName)
{
 	if(trans->info().internalId().toULong() != cookie())
		return;

	if(!trans->info().contact())
		return;

	setKopeteTransfer(trans);

	MSNChatSession* manager=dynamic_cast<MSNChatSession*>(m_contact->manager());

	if(manager && manager->service())
	{
		setFile(fileName);

		QCString message=QString(
			"MIME-Version: 1.0\r\n"
			"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
			"\r\n"
			"Invitation-Command: ACCEPT\r\n"
			"Invitation-Cookie: " + QString::number(cookie()) + "\r\n"
			"Launch-Application: FALSE\r\n"
			"Request-Data: IP-Address:\r\n"  ).utf8();
		manager->service()->sendCommand( "MSG" , "N", true, message );

		QTimer::singleShot( 3 * 60000, this, SLOT(slotTimer()) ); //if after 3 minutes the transfer has not begin, delete this
	}
	else
	{
		if( m_kopeteTransfer)
			m_kopeteTransfer->slotError(  KIO::ERR_UNKNOWN , i18n( "An unknown error occurred" ) );
		emit done(this);

	}
}

void MSNFileTransferSocket::slotFileTransferRefused(const Kopete::FileTransferInfo &info)
{
	if(info.internalId().toULong() != cookie())
		return;

	if(!info.contact())
		return;

	MSNChatSession* manager=dynamic_cast<MSNChatSession*>(m_contact->manager());

	if(manager && manager->service())
	{
		manager->service()->sendCommand( "MSG" , "N", true, rejectMessage() );
	}

	emit done(this);
}

void MSNFileTransferSocket::slotKopeteTransferDestroyed()
{
	m_kopeteTransfer=0L;
}


#include "msnfiletransfersocket.moc"

