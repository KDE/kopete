/*
    Copyright (c) 2005 by Olivier Goffart        <ogoffart@ kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "webcam.h"

#if MSN_WEBCAM

#include <stdlib.h>
#include <kdebug.h>
#include <qregexp.h>
#include <kbufferedsocket.h>
#include <klocale.h>
#include <kserversocket.h>
#include <kmessagebox.h>
#include <qlabel.h>
#include <qguardedptr.h>
#include <qtimer.h>

#include "dispatcher.h"

#include "mimicwrapper.h"
#include "msnwebcamdialog.h"


using namespace KNetwork;

namespace P2P {

Webcam::Webcam( const QString& to, Dispatcher *parent, Q_UINT32 sessionId)
	: TransferContext(parent)
{
	setType(P2P::WebcamType);
	m_direction = Incoming;
	m_sessionId  = sessionId;
	m_recipient  = to;
	m_offset = 0l;
	m_listener  = 0l;
	m_webcamSocket=0L;
	m_webcamState=wsNegotiating;
	
	m_mimic=0L;
	m_widget=0L;
}

Webcam::~Webcam()
{
	m_dispatcher=0l;
	delete m_mimic;
	delete m_webcamSocket;
	delete m_widget;
}

void Webcam::askIncommingInvitation()
{
	//protect, in case this is deleted when the messagebox is active
	QGuardedPtr<Webcam> _this = this;
	int result=KMessageBox::questionYesNo( 0L , i18n("The contact %1 want to show you his webcam, do you want to see it?").arg(m_recipient),
										   i18n("Webcam invitation - Kopete MSN Plugin") , i18n("Accept") , i18n("Decline"));
	if(!_this)
		return;
	
	QString content = QString("SessionID: %1\r\n\r\n").arg(m_sessionId);
	if(result==KMessageBox::Yes)
	{
		//Send two message, an OK, and an invite.
		//Normaly, the user should decline the invite (i hope)
		
		// Send a 200 OK message to the recipient.
		sendMessage(OK, content);
				
		
		//send an INVITE message we want the user decline
		//need to change the branch of the second message
		m_branch=Uid::createUid();
				
		content=QString("Bridges: TRUDPv1 TCPv1\r\n"
						"NetID: -1280904111\r\n"
						"Conn-Type: Symmetric-NAT\r\n"
						"UPnPNat: false\r\n"
						"ICF: false\r\n\r\n");

		sendMessage(INVITE, content);
		
	}
	else
	{
		//Decline the invitation
		sendMessage(DECLINE, content);
		m_state=Finished;
	}
}

void Webcam::sendBYEMessage()
{
	m_state=Finished;
	QString content="Context: dAMAgQ==\r\n";
	sendMessage(BYE,content);
	
	//If ever the opposite client was dead or something, we'll ack anyway, so everything get cleaned
	QTimer::singleShot(60*1000 , this, SLOT(acknowledged()));
}



void Webcam::acknowledged()
{
	kdDebug(14140) << k_funcinfo << endl;
	
	switch(m_state)
	{
		case Invitation:
		{
			m_state=Negotiation;
			break;
		}
		
		/*
		case Negotiation:
		{
			if(m_type == UserDisplayIcon)
			{
				<<< Data preparation acknowledge message.
				m_state = DataTransfer;
				m_identifier++;
				Start sending data.
				slotSendData();
			}
			break;
		}
		
		case DataTransfer:
			NOTE <<< Data acknowledged message.
			<<< Bye message should follow.
			if(m_type == File)
			{
				if(m_handshake == 0x01)
				{
					Data handshake acknowledge message.
					Start sending data.
					slotSendData();
				}
				else if(m_handshake == 0x02)
				{
					Data acknowledge message.
					Send the recipient a BYE message.
					m_state = Finished;
					sendMessage(BYE, "\r\n");
				}
			}
			
			break;
		*/
		case Finished:
			//BYE or DECLINE acknowledge message.
			m_dispatcher->detach(this);
			break;
		default:
			break;
	}
}




void Webcam::processMessage(const Message& message)
{
	if(message.header.dataOffset+message.header.dataSize >= message.header.totalDataSize)
		acknowledge( message ); //aknowledge if needed
	
	if(message.applicationIdentifier != 4l)
	{
		QString body = QCString(message.body.data(), message.header.dataSize);
		kdDebug(14141) << k_funcinfo << "received, " << body << endl;

		if(body.startsWith("MSNSLP/1.0 603 DECLINE"))
		{
			//that's the declinaison of the second invitaiton message, don't care for now
		}
		else if(body.startsWith("BYE"))
		{
			m_state = Finished;

			// Dispose of this transfer context.
			m_dispatcher->detach(this);
		}
		return;
	}
	
	
	//Let's take the fun, we entering into the delicious webcam  negotiation binary protocol

	//well, there is maybe better to take utf16,  but it's ascii, so no problem.
	QByteArray dataMessage=message.body;
	for(uint pos=m_content.isNull() ? 10 : 0; pos<dataMessage.size(); pos+=2)
	{
		if(dataMessage[pos] !=0 )
			m_content+=dataMessage[pos];
	}

	if(message.header.dataOffset+message.header.dataSize < message.header.totalDataSize)
		return;

	kdDebug(14141) << k_funcinfo << "Message contents: " << m_content << "\n" << endl;
	if(m_content.length() < 5)
		makeSIPMessage(m_content);
	else if(m_content.contains("<producer>"))
	{
		QRegExp rx("<rid>([0-9]*)</rid>.*<session>([0-9]*)</session>");
		rx.search(m_content);	
		QString rid=rx.cap(1);
		QString sess=rx.cap(2);
		QString viewerxml=xml(sess.toUInt() , rid.toUInt());
		kdDebug(14140) << k_funcinfo << "vewerxml= " << viewerxml << endl; 
		makeSIPMessage(  viewerxml );
//		FarsightWrapper *farsight=new FarsightWrapper( viewerxml , m_content , this );
		
		m_auth=QString("recipientid=%1&sessionid=%2\r\n\r\n").arg(rid,sess);
		
		kdDebug(14140) << k_funcinfo << "m_auth= " << m_auth << endl;
	
		m_listener = new KServerSocket("7786",this);
		//m_listener->setResolutionEnabled(true);
				// Create the callback that will try to accept incoming connections.
		QObject::connect(m_listener, SIGNAL(readyAccept()), this, SLOT(slotAccept()));
		QObject::connect(m_listener, SIGNAL(gotError(int)), this, SLOT(slotListenError(int)));
				// Listen for incoming connections.
		bool isListening = m_listener->listen(1);
		kdDebug(14140) << k_funcinfo << (isListening ? QString("listening %1").arg(m_listener->localAddress().nodeName()) : QString("not listening")) << endl;
		
		rx=QRegExp("<tcpport>([^<]*)</tcpport>");
		rx.search(m_content);
		QString port1=rx.cap(1);
		
		rx=QRegExp("<tcplocalport>([^<]*)</tcplocalport>");
		rx.search(m_content);
		QString port2=rx.cap(1);
		if(port2==port1)
			port2=QString::null;
		
		rx=QRegExp("<tcpexternalport>([^<]*)</tcpexternalport>");
		rx.search(m_content);
		QString port3=rx.cap(1);
		if(port3==port1 || port3==port2)
			port3=QString::null;

		int an=0;
		while(true)
		{
			an++;
			if(!m_content.contains( QString("<tcpipaddress%1>").arg(an)  ))
				break;
			rx=QRegExp(QString("<tcpipaddress%1>([^<]*)</tcpipaddress%2>").arg(an).arg(an));
			rx.search(m_content);
			QString ip=rx.cap(1);
			if(ip.isNull())
				continue;
			
			if(!port1.isNull())
			{
				kdDebug(14140) << k_funcinfo << "trying to connect on " << ip <<":" << port1 << endl;
				m_allSockets.append(new KBufferedSocket( ip, port2, this ));
			}
			if(!port2.isNull())
			{
				kdDebug(14140) << k_funcinfo << "trying to connect on " << ip <<":" << port2 << endl;
				m_allSockets.append(new KBufferedSocket( ip, port2, this ));
			}
			if(!port2.isNull())
			{
				kdDebug(14140) << k_funcinfo << "trying to connect on " << ip <<":" << port2 << endl;
				m_allSockets.append(new KBufferedSocket( ip, port2, this ));
			}
		}
		QValueList<KBufferedSocket*>::iterator it;
		for ( it = m_allSockets.begin(); it != m_allSockets.end(); ++it )
		{
			KBufferedSocket *sock=(*it);
			QObject::connect( sock, SIGNAL( connected( const KResolverEntry&) ), this, SLOT( slotSocketConnected() ) );
			sock->enableRead( false );
			sock->connect();
		}
	}
	else if(m_content.contains("receivedViewerData"))
	{
		//I'm happy you received the xml i sent, really.
	}
	m_content=QString::null;
}

void Webcam::makeSIPMessage(const QString &message)
{
	QByteArray dataMessage; //(12+message.length()*2);
	QDataStream writer(dataMessage, IO_WriteOnly);
	writer.setByteOrder(QDataStream::LittleEndian);
	writer << (Q_UINT8)0x80;
	writer << (Q_UINT8)0x17; //XX
	writer << (Q_UINT8)0x2a; //YY
	writer << (Q_UINT8)0x01; //ZZ
	writer << (Q_UINT8)0x08;
	writer << (Q_UINT8)0x00;
	writer << message+'\0';
	//writer << (Q_UINT16)0x0000;

	/*QString echoS="";
	unsigned int f=0;
	while(f<dataMessage.size())
	{
		echoS+="\n";
		for(unsigned int q=0; q<16 ; q++)
		{
			if(q+f<dataMessage.size())
			{
				unsigned int N=(unsigned int) (dataMessage[q+f]);
				if(N<16)
					echoS+="0";
				echoS+=QString::number( N  ,16)+" ";
			}
			else
				echoS+="   ";
		}
		echoS+="   ";
				
		for(unsigned int q=0; (q<16 && (q+f)<dataMessage.size()) ; q++)
		{
			unsigned char X=dataMessage[q+f];
			char C=((char)(( X<128 && X>31 ) ? X : '.'));
			echoS+=QString::fromLatin1(&C,1);
		}
		f+=16;
	}
	kdDebug(14141) << k_funcinfo << dataMessage.size() << echoS << endl;*/
				
	
	sendBigP2PMessage(dataMessage);
}

void Webcam::sendBigP2PMessage( const QByteArray & dataMessage)
{
	unsigned int size=m_totalDataSize=dataMessage.size();
	m_offset=0;
	++m_identifier;

	for(unsigned int f=0;f<size;f+=1200)
	{
		m_offset=f;
		QByteArray dm2;
		dm2.duplicate(dataMessage.data()+m_offset, QMIN(1200,m_totalDataSize-m_offset));
		sendData( dm2 );
		m_offset+=dm2.size();
	}
	m_offset=0;
	m_totalDataSize=0;
}



QString Webcam::xml(uint session , uint rid)
{
	if(session==0) 
		session=rand()%1000+5000;
	if(rid==0) 
		rid=rand()%100+50;
	QString who= false ? QString("producer") : QString("viewer");
	
	QString ip= m_dispatcher->localIp();
	
	return "<" + who + "><version>2.0</version><rid>"+QString::number(rid)+"</rid><udprid>"+QString::number(rid+1)+"</udprid><session>"+QString::number(session)+"</session><ctypes>0</ctypes><cpu>2931</cpu>" +
			"<tcp><tcpport>7786</tcpport>\t\t\t\t\t\t\t\t  <tcplocalport>7786</tcplocalport>\t\t\t\t\t\t\t\t  <tcpexternalport>7786</tcpexternalport><tcpipaddress1>"+ip+"</tcpipaddress1><tcpipaddress2>192.168.0.1</tcpipaddress2>,</tcp>"+
			"<udp><udplocalport>7786</udplocalport><udpexternalport>31863</udpexternalport><udpexternalip>"+ ip +"</udpexternalip><a1_port>31859</a1_port><b1_port>31860</b1_port><b2_port>31861</b2_port><b3_port>31862</b3_port><symmetricallocation>1</symmetricallocation><symmetricallocationincrement>1</symmetricallocationincrement><udpversion>1</udpversion><udpinternalipaddress1>127.0.0.1</udpinternalipaddress1></udp>"+
			"<codec></codec><channelmode>1</channelmode></"+who+">\r\n\r\n";
}

/* ---------- Now functions about the dirrect connection  --------- */

void Webcam::slotSocketConnected()
{
	if(m_webcamSocket)
		return;
	m_webcamSocket=const_cast<KBufferedSocket*>(static_cast<const KBufferedSocket*>(sender()));
	if(!m_webcamSocket)
		return;
	
	delete m_listener;
	m_listener=0L;
	
	QValueList<KBufferedSocket*>::iterator it;
	for ( it = m_allSockets.begin(); it != m_allSockets.end(); ++it )
	{
		KBufferedSocket *sock=(*it);
		if(sock!=m_webcamSocket)
			delete sock;
	}
	m_allSockets.clear();
	
	kdDebug(14140) << k_funcinfo << "Connection established." << endl;
	
	m_webcamSocket->setBlocking(false);
	m_webcamSocket->enableRead(true);
	m_webcamSocket->enableWrite(false);

	// Create the callback that will try to read bytes from the accepted socket.
	QObject::connect(m_webcamSocket, SIGNAL(readyRead()),   this, SLOT(slotSocketRead()));
	// Create the callback that will try to handle the socket close event.
	QObject::connect(m_webcamSocket, SIGNAL(closed()),      this, SLOT(slotSocketClosed()));
	// Create the callback that will try to handle the socket error event.
	QObject::connect(m_webcamSocket, SIGNAL(gotError(int)), this, SLOT(slotSocketError(int)));

	m_webcamState=wsConnected;
	QCString to_send=m_auth.utf8();
	m_webcamSocket->writeBlock(to_send.data(), to_send.length());

}


void Webcam::slotAccept()
{
	if(m_webcamSocket)
		return;
	
	
	// Try to accept an incoming connection from the sending client.
	m_webcamSocket = static_cast<KBufferedSocket*>(m_listener->accept());
	if(!m_webcamSocket)
	{
		// NOTE If direct connection fails, the sending
		// client wil transfer the file data through the
		// existing session.
		kdDebug(14140) << k_funcinfo << "Direct connection failed." << endl;
		// Close the listening endpoint.
//		m_listener->close();
		return;
	}

	kdDebug(14140) << k_funcinfo << "Direct connection established." << endl;

	// Set the socket to non blocking,
	// enable the ready read signal and disable
	// ready write signal.
	// NOTE readyWrite consumes too much cpu usage.
	m_webcamSocket->setBlocking(false);
	m_webcamSocket->enableRead(true);
	m_webcamSocket->enableWrite(false);

	// Create the callback that will try to read bytes from the accepted socket.
	QObject::connect(m_webcamSocket, SIGNAL(readyRead()),   this, SLOT(slotSocketRead()));
	// Create the callback that will try to handle the socket close event.
	QObject::connect(m_webcamSocket, SIGNAL(closed()),      this, SLOT(slotSocketClosed()));
	// Create the callback that will try to handle the socket error event.
	QObject::connect(m_webcamSocket, SIGNAL(gotError(int)), this, SLOT(slotSocketError(int)));
	
	//m_lisener->close();
	delete m_listener;
	m_listener=0l;
	
	QValueList<KBufferedSocket*>::iterator it;
	for ( it = m_allSockets.begin(); it != m_allSockets.end(); ++it )
	{
		KBufferedSocket *sock=(*it);
		delete sock;
	}
	m_allSockets.clear();
}

void Webcam::slotSocketRead()
{
	uint available = m_webcamSocket->bytesAvailable();
	kdDebug(14140) << k_funcinfo << available << ", bytes available." << endl;
	const QString connected_str("connected\r\n\r\n");
	switch(m_webcamState)
	{
		case wsNegotiating:
		{
			if(available < m_auth.length())
			{
				kdDebug(14140) << k_funcinfo << "waiting more data   ( " << available << "  of  " <<m_auth.length()<< " )"<<  endl;
				break;
			}
			QByteArray buffer(available);
			m_webcamSocket->readBlock(buffer.data(), buffer.size());
		
			kdDebug(14140) << k_funcinfo << buffer.data() <<  endl;

			if(QString(buffer) == m_auth )
			{
				kdDebug(14140) << k_funcinfo << "Sending " << connected_str << endl;
				QCString conne=connected_str.utf8();
				m_webcamSocket->writeBlock(conne.data(), conne.length());
				m_webcamState=wsConnecting;
			}
			else
			{
				kdWarning(14140) << k_funcinfo << "Auth failed" << endl;
				m_webcamSocket->disconnect();
				sendBYEMessage();
			}
			break;
		}
		case wsConnecting:
		case wsConnected:
		{
			if(available < connected_str.length())
			{
				kdDebug(14140) << k_funcinfo << "waiting more data   ( " << available << "  of  " <<connected_str.length()<< " )"<<  endl;
				break;
			}
			QByteArray buffer(connected_str.length());
			m_webcamSocket->readBlock(buffer.data(), buffer.size());
	
			if(QString(buffer) == connected_str)
			{

				m_mimic=new MimicWrapper();
				
				m_widget=new MSNWebcamDialog(m_recipient);
				connect(m_widget, SIGNAL( closingWebcamDialog() ) , this , SLOT(sendBYEMessage()));
				
				if(m_webcamState==wsConnected)
				{
					kdDebug(14140) << k_funcinfo << "Sending " << connected_str << endl;
					QCString conne=connected_str.utf8();
					m_webcamSocket->writeBlock(conne.data(), conne.length());
				}
				
				m_webcamState=wsTransfer;
			}
			else
			{
				kdWarning(14140) << k_funcinfo << "Connecting failed" << endl;
				m_webcamSocket->disconnect();
				sendBYEMessage();
			}
			break;
		}
		case wsTransfer:
		{
			if(available < 24)
			{
				kdDebug(14140) << k_funcinfo << "waiting more data   ( " << available << "  of  " <<24<< " )"<<  endl;
				break;
			}
			QByteArray buffer(available);
			m_webcamSocket->peekBlock(buffer.data(), buffer.size());
			
			kdDebug(14140) << k_funcinfo << "header   ( " << (uint)(buffer[8]) << " " <<(uint)(buffer[9]) << " " <<(uint)(buffer[10]) << " " <<(uint)(buffer[11]) << " " <<endl;
			Q_UINT32 paysize=(uchar)buffer[8] + ((uchar)buffer[9]<<8) + ((uchar)buffer[10]<<16) + ((uchar)buffer[11]<<24);
			
			if(available < (paysize+24))
			{
				kdDebug(14140) << k_funcinfo << "waiting more data   ( " << available << "  of  " <<paysize<< " )"<<  endl;
				break;
			}
			m_webcamSocket->readBlock(buffer.data(), 24); //flush
			buffer.resize(paysize);
			m_webcamSocket->readBlock(buffer.data(), buffer.size());
			
			QPixmap pix=m_mimic->decode(buffer);
			if(pix.isNull())
			{
				kdWarning(14140) << k_funcinfo << "incorrect pixmap returned, better to stop everything"<<  endl;
				m_webcamSocket->disconnect();
				sendBYEMessage();
			}
			m_widget->newImage(pix);
			break;
		}
		default:
			break;
	}

}

void Webcam::slotListenError(int errorCode)
{
	kdWarning(14140) << k_funcinfo << "Error " << errorCode << " : " << m_listener->errorString() << endl;
}

void Webcam::slotSocketClosed()
{
	if(!m_dispatcher) //we are in this destructor
		return; 
	kdDebug(14140) << k_funcinfo << endl;
	sendBYEMessage();
}

void Webcam::slotSocketError(int errorCode)
{
	kdDebug(14140) << k_funcinfo <<  errorCode <<  endl;
	sendBYEMessage();
}


}


#include "webcam.moc"

#endif

