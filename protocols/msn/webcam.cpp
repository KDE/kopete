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
#include <qevent.h>
#include <qdatetime.h>
#include <kconfig.h>

#include "dispatcher.h"

#include "mimicwrapper.h"
#include "msnwebcamdialog.h"


#include "avdevice/videodevicepool.h"

using namespace KNetwork;

namespace P2P {

Webcam::Webcam(Who who, const QString& to, Dispatcher *parent, Q_UINT32 sessionId)
	: TransferContext(to,parent,sessionId)  , m_who(who) , m_timerId(0)
{
	setType(P2P::WebcamType);
	m_direction = Incoming;
	m_listener  = 0l;
	m_webcamSocket=0L;
//	m_webcamState=wsNegotiating;
	
	m_mimic=0L;
	m_widget=0L;

	KConfig *config = KGlobal::config();
	config->setGroup( "MSN" );

	// Read the configuration to get the number of frame per second to send
	int webCamFps=config->readNumEntry("WebcamFPS", 25);
	m_timerFps = 1000 / webCamFps;
}

Webcam::~Webcam()
{
	kdDebug(14140) << k_funcinfo<< "################################################" << endl;
	m_dispatcher=0l;
	delete m_mimic;
	delete m_webcamSocket;
	delete m_widget;
	
	if(m_timerId != 0) //if we were sending
	{
		Kopete::AV::VideoDevicePool *videoDevice = Kopete::AV::VideoDevicePool::self(); 
		videoDevice->stopCapturing(); 
		videoDevice->close();
	}

}

void Webcam::askIncommingInvitation()
{
	m_direction = Incoming;
	//protect, in case this is deleted when the messagebox is active
	QGuardedPtr<Webcam> _this = this;
	QString message= (m_who==wProducer)  ?
			i18n("<qt>The contact %1 wants to see <b>your</b> webcam, do you want them to see it?</qt>")  :
			i18n("The contact %1 wants to show you his/her webcam, do you want to see it?")  ;
	int result=KMessageBox::questionYesNo( 0L , message.arg(m_recipient),
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
		m_state = Negotiation;  //set type to application/x-msnmsgr-transreqbody
				
		content=QString("Bridges: TRUDPv1 TCPv1\r\n"
						"NetID: -1280904111\r\n"
						"Conn-Type: Firewall\r\n"
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
//			m_state=Negotiation;
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

		if(body.startsWith("MSNSLP/1.0 200 OK"))
		{
			m_direction = Outgoing;
		}
		if(body.startsWith("INVITE"))
		{
			if(m_direction == Outgoing)
			{
				QRegExp regex(";branch=\\{([0-9A-F\\-]*)\\}\r\n");
				regex.search(body);
				m_branch=regex.cap(1);
				//decline
				sendMessage(DECLINE);
				makeSIPMessage("syn",0x17,0x2a,0x01);
			}
		}
		else if(body.startsWith("MSNSLP/1.0 603 DECLINE"))
		{
			//if it is the declinaison of the second invite message, we have to don't care
			//TODO anyway, if it's the declinaison of our invitation, we have to something
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
	
#if 0
	QString echoS="";
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
	kdDebug(14141) << k_funcinfo << dataMessage.size() << echoS << endl;
#endif
	
	
	
	
	
	for(uint pos=m_content.isNull() ? 10 : 0; pos<dataMessage.size(); pos+=2)
	{
		if(dataMessage[pos] !=0 )
			m_content+=dataMessage[pos];
	}

	if(message.header.dataOffset+message.header.dataSize < message.header.totalDataSize)
		return;

	kdDebug(14141) << k_funcinfo << "Message contents: " << m_content << "\n" << endl;
	if(m_content.startsWith("syn"))
	{
		if(m_direction == Incoming)
			makeSIPMessage("syn",0x17,0x2a,0x01);
		else
			makeSIPMessage("ack",0xea,0x00,0x00);
	}
	else if(m_content.startsWith("ack"))
	{
		if(m_direction == Incoming)
			makeSIPMessage("ack",0xea,0x00,0x00);
		
		if(m_who==wProducer)
		{
			uint sess=rand()%1000+5000;
			uint rid=rand()%100+50;
			m_myAuth=QString("recipientid=%1&sessionid=%2\r\n\r\n").arg(rid).arg(sess);
			kdDebug(14140) << k_funcinfo << "m_myAuth= " << m_myAuth << endl;
			QString  producerxml=xml(sess , rid);
			kdDebug(14140) << k_funcinfo << "producerxml= " << producerxml << endl; 
			makeSIPMessage(producerxml);
		}
	}
	else if(m_content.contains("<producer>")   ||  m_content.contains("<viewer>"))
	{
		QRegExp rx("<rid>([0-9]*)</rid>.*<session>([0-9]*)</session>");
		rx.search(m_content);	
		QString rid=rx.cap(1);
		QString sess=rx.cap(2);
		if(m_content.contains("<producer>"))
		{
			
			QString viewerxml=xml(sess.toUInt() , rid.toUInt());
			kdDebug(14140) << k_funcinfo << "vewerxml= " << viewerxml << endl; 
			makeSIPMessage(  viewerxml ,0x00,0x09,0x00 );
			m_peerAuth=m_myAuth=QString("recipientid=%1&sessionid=%2\r\n\r\n").arg(rid,sess);
			kdDebug(14140) << k_funcinfo << "m_auth= " << m_myAuth << endl;
		}
		else
		{
			m_peerAuth=QString("recipientid=%1&sessionid=%2\r\n\r\n").arg(rid,sess);
			
			makeSIPMessage("receivedViewerData", 0xec , 0xda , 0x03);
		}

		if(!m_listener)
		{
			//it should have been creed in xml
			sendBYEMessage();
			return;
		}
		//m_listener->setResolutionEnabled(true);
				// Create the callback that will try to accept incoming connections.
		QObject::connect(m_listener, SIGNAL(readyAccept()), this, SLOT(slotAccept()));
		QObject::connect(m_listener, SIGNAL(gotError(int)), this, SLOT(slotListenError(int)));
				// Listen for incoming connections.
		bool isListening = m_listener->listen();
		kdDebug(14140) << k_funcinfo << (isListening ? QString("listening %1").arg(m_listener->localAddress().toString()) : QString("not listening")) << endl;		
		
		rx=QRegExp("<tcpport>([^<]*)</tcpport>");
		rx.search(m_content);
		QString port1=rx.cap(1);
		if(port1=="0")
			port1=QString::null;
		
		rx=QRegExp("<tcplocalport>([^<]*)</tcplocalport>");
		rx.search(m_content);
		QString port2=rx.cap(1);
		if(port2==port1 || port2=="0")
			port2=QString::null;
		
		rx=QRegExp("<tcpexternalport>([^<]*)</tcpexternalport>");
		rx.search(m_content);
		QString port3=rx.cap(1);
		if(port3==port1 || port3==port2 || port3=="0")
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
				KBufferedSocket *sock=new KBufferedSocket( ip, port1, this );
				m_allSockets.append(sock);
				QObject::connect( sock, SIGNAL( connected( const KResolverEntry&) ), this, SLOT( slotSocketConnected() ) );
				QObject::connect( sock, SIGNAL( gotError(int)), this, SLOT(slotSocketError(int)));
				sock->connect(ip, port1);
				kdDebug(14140) << k_funcinfo << "okok " << sock << " - " << sock->peerAddress().toString() << " ; " << sock->localAddress().toString()  << endl;
			}
			if(!port2.isNull())
			{
				kdDebug(14140) << k_funcinfo << "trying to connect on " << ip <<":" << port2 << endl;
				KBufferedSocket *sock=new KBufferedSocket( ip, port2, this );
				m_allSockets.append(sock);
				QObject::connect( sock, SIGNAL( connected( const KResolverEntry&) ), this, SLOT( slotSocketConnected() ) );
				QObject::connect( sock, SIGNAL( gotError(int)), this, SLOT(slotSocketError(int)));
				sock->connect(ip, port2);
			}
			if(!port3.isNull())
			{
				kdDebug(14140) << k_funcinfo << "trying to connect on " << ip <<":" << port3 << endl;
				KBufferedSocket *sock=new KBufferedSocket( ip, port3, this );
				m_allSockets.append(sock);
				QObject::connect( sock, SIGNAL( connected( const KResolverEntry&) ), this, SLOT( slotSocketConnected() ) );
				QObject::connect( sock, SIGNAL( gotError(int)), this, SLOT(slotSocketError(int)));
				sock->connect(ip, port3);
			}
		}
		QValueList<KBufferedSocket*>::iterator it;
		for ( it = m_allSockets.begin(); it != m_allSockets.end(); ++it )
		{
			KBufferedSocket *sock=(*it);
			
			//sock->enableRead( false );
			kdDebug(14140) << k_funcinfo << "connect to "  << sock << " - "<< sock->peerAddress().toString() << " ; " << sock->localAddress().toString()  << endl;
		}
	}
	else if(m_content.contains("receivedViewerData"))
	{
		//I'm happy you received the xml i sent, really.
	}
	else
		error();
	m_content=QString::null;
}

void Webcam::makeSIPMessage(const QString &message, Q_UINT8 XX, Q_UINT8 YY , Q_UINT8 ZZ)
{
	QByteArray dataMessage; //(12+message.length()*2);
	QDataStream writer(dataMessage, IO_WriteOnly);
	writer.setByteOrder(QDataStream::LittleEndian);
	writer << (Q_UINT8)0x80;
	writer << (Q_UINT8)XX;
	writer << (Q_UINT8)YY;
	writer << (Q_UINT8)ZZ;
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
	QString who= ( m_who == wProducer ) ? QString("producer") : QString("viewer");
	
	QString ip;
	
	uint ip_number=1;
	QStringList::iterator it;
	QStringList ips=m_dispatcher->localIp();
	for ( it = ips.begin(); it != ips.end(); ++it )
	{
		ip+=QString("<tcpipaddress%1>%2</tcpipaddress%3>").arg(ip_number).arg(*it).arg(ip_number);
		++ip_number;
	}

        QString port = QString::number(getAvailablePort());
	
	m_listener = new KServerSocket(port, this) ;
	
	return "<" + who + "><version>2.0</version><rid>"+QString::number(rid)+"</rid><udprid>"+QString::number(rid+1)+"</udprid><session>"+QString::number(session)+"</session><ctypes>0</ctypes><cpu>2931</cpu>" +
			"<tcp><tcpport>"+port+"</tcpport>\t\t\t\t\t\t\t\t  <tcplocalport>"+port+"</tcplocalport>\t\t\t\t\t\t\t\t  <tcpexternalport>"+port+"</tcpexternalport>"+ip+"</tcp>"+
			"<udp><udplocalport>7786</udplocalport><udpexternalport>31863</udpexternalport><udpexternalip>"+ ip +"</udpexternalip><a1_port>31859</a1_port><b1_port>31860</b1_port><b2_port>31861</b2_port><b3_port>31862</b3_port><symmetricallocation>1</symmetricallocation><symmetricallocationincrement>1</symmetricallocationincrement><udpversion>1</udpversion><udpinternalipaddress1>127.0.0.1</udpinternalipaddress1></udp>"+
			"<codec></codec><channelmode>1</channelmode></"+who+">\r\n\r\n";
}

int Webcam::getAvailablePort()
{
    KConfig *config = KGlobal::config();
    config->setGroup( "MSN" );
    QString basePort=config->readEntry("WebcamPort");
    if(basePort.isEmpty() || basePort == "0" )
		basePort="6891";
	
    uint firstport = basePort.toInt();
    uint maxOffset=config->readUnsignedNumEntry("WebcamMaxPortOffset", 10);
    uint lastport = firstport + maxOffset;

	// try to find an available port
	//
    KServerSocket *ss = new KServerSocket();
    ss->setFamily(KResolver::InetFamily);
    bool found = false;
	unsigned int port = firstport;
    for( ; port <= lastport; ++port) {
		ss->setAddress( QString::number( port ) );
		bool success = ss->listen();
		if( found = ( success && ss->error() == KSocketBase::NoError ) )
			break;
		ss->close();
    }
	delete ss;
	

    kdDebug(14140) << k_funcinfo<< "found available port : " << port << endl;

	return port;
}


/* ---------- Now functions about the dirrect connection  --------- */

void Webcam::slotSocketConnected()
{
	kdDebug(14140) << k_funcinfo <<"##########################" << endl;
	
	m_webcamSocket=const_cast<KBufferedSocket*>(static_cast<const KBufferedSocket*>(sender()));
	if(!m_webcamSocket)
		return;
	
	kdDebug(14140) << k_funcinfo << "Connection established on  " <<  m_webcamSocket->peerAddress().toString() << " ; " << m_webcamSocket->localAddress().toString()  << endl;
	
	m_webcamSocket->setBlocking(false);
	m_webcamSocket->enableRead(true);
	m_webcamSocket->enableWrite(false);

	// Create the callback that will try to read bytes from the accepted socket.
	QObject::connect(m_webcamSocket, SIGNAL(readyRead()),   this, SLOT(slotSocketRead()));
	// Create the callback that will try to handle the socket close event.
	QObject::connect(m_webcamSocket, SIGNAL(closed()),      this, SLOT(slotSocketClosed()));
	// Create the callback that will try to handle the socket error event.
//	QObject::connect(m_webcamSocket, SIGNAL(gotError(int)), this, SLOT(slotSocketError(int)));

	m_webcamStates[m_webcamSocket]=wsConnected;
	QCString to_send=m_peerAuth.utf8();
	m_webcamSocket->writeBlock(to_send.data(), to_send.length());
	kdDebug(14140) << k_funcinfo << "sending "<< m_peerAuth << endl;

}


void Webcam::slotAccept()
{
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

	kdDebug(14140) << k_funcinfo << "################################  Direct connection established." << endl;

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
	
	m_allSockets.append(m_webcamSocket);
	m_webcamStates[m_webcamSocket]=wsNegotiating;
}

void Webcam::slotSocketRead()
{
	m_webcamSocket=const_cast<KBufferedSocket*>(static_cast<const KBufferedSocket*>(sender()));

	uint available = m_webcamSocket->bytesAvailable();
	kdDebug(14140) << k_funcinfo  <<  m_webcamSocket <<  "############# " << available << " bytes available." << endl;
		
	QByteArray avail_buff(available);
	m_webcamSocket->peekBlock(avail_buff.data(), avail_buff.size());
	
	kdDebug(14140) << k_funcinfo  <<  m_webcamSocket << avail_buff << endl;
	
	
	
	const QString connected_str("connected\r\n\r\n");
	switch(m_webcamStates[m_webcamSocket])
	{
		case wsNegotiating:
		{
			if(available < m_myAuth.length())
			{
				kdDebug(14140) << k_funcinfo << "waiting more data   ( " << available << "  of  " <<m_myAuth.length()<< " )"<<  endl;
				break;
			}
			QByteArray buffer(available);
			m_webcamSocket->readBlock(buffer.data(), buffer.size());
		
			kdDebug(14140) << k_funcinfo << buffer.data() <<  endl;

			if(QString(buffer) == m_myAuth )
			{
				closeAllOtherSockets();
				kdDebug(14140) << k_funcinfo << "Sending " << connected_str << endl;
				QCString conne=connected_str.utf8();
				m_webcamSocket->writeBlock(conne.data(), conne.length());
				m_webcamStates[m_webcamSocket]=wsConnecting;
				
				//SHOULD NOT BE THERE
				m_mimic=new MimicWrapper();
				if(m_who==wProducer)
				{
					Kopete::AV::VideoDevicePool *videoDevice = Kopete::AV::VideoDevicePool::self();
					videoDevice->open();
					videoDevice->setSize(320, 240);
					videoDevice->startCapturing();
					
					m_timerId=startTimer(m_timerFps);
					kdDebug(14140) << k_funcinfo <<  "new timer" << m_timerId << endl;
				}
				m_widget=new MSNWebcamDialog(m_recipient);
				connect(m_widget, SIGNAL( closingWebcamDialog() ) , this , SLOT(sendBYEMessage()));

			}
			else
			{
				kdWarning(14140) << k_funcinfo << "Auth failed" << endl;
				m_webcamSocket->disconnect();
				m_webcamSocket->deleteLater();
				m_allSockets.remove(m_webcamSocket);
				m_webcamSocket=0l;
				//sendBYEMessage();
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
			
// 			kdDebug(14140) << k_funcinfo << "state " << m_webcamState << " received :" << QCString(buffer) <<  endl;
				
	
			if(QString(buffer) == connected_str)
			{
				if(m_webcamStates[m_webcamSocket]==wsConnected)
				{
					closeAllOtherSockets();
					kdDebug(14140) << k_funcinfo << "Sending " << connected_str << endl;
					QCString conne=connected_str.utf8();
					m_webcamSocket->writeBlock(conne.data(), conne.length());
												
					//SHOULD BE DONE IN ALL CASE
				m_mimic=new MimicWrapper();
				if(m_who==wProducer)
				{
					Kopete::AV::VideoDevicePool *videoDevice = Kopete::AV::VideoDevicePool::self();
					videoDevice->open();
					videoDevice->setSize(320, 240);
					videoDevice->startCapturing();
					
					m_timerId=startTimer(m_timerFps);
					kdDebug(14140) << k_funcinfo <<  "new timer" << m_timerId << endl;
				}
				m_widget=new MSNWebcamDialog(m_recipient);
				connect(m_widget, SIGNAL( closingWebcamDialog() ) , this , SLOT(sendBYEMessage()));
				
				}
				m_webcamStates[m_webcamSocket]=wsTransfer;

			}
			else
			{
				kdWarning(14140) << k_funcinfo << "Connecting failed" << endl;
				m_webcamSocket->disconnect();
				m_webcamSocket->deleteLater();
				m_allSockets.remove(m_webcamSocket);
				m_webcamSocket=0l;
			}
			break;
		}
		case wsTransfer:
		{
			if(m_who==wProducer)
			{
				kdWarning(14140) << k_funcinfo << "data received when we are producer"<<  endl;
				break;
			}
			if(available < 24)
			{
				kdDebug(14140) << k_funcinfo << "waiting more data   ( " << available << "  of  " <<24<< " )"<<  endl;
				break;
			}
			QByteArray buffer(24);
			m_webcamSocket->peekBlock(buffer.data(), buffer.size());
			
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
	
	KBufferedSocket *m_webcamSocket=const_cast<KBufferedSocket*>(static_cast<const KBufferedSocket*>(sender()));
	
	kdDebug(14140) << k_funcinfo <<  m_webcamSocket <<  endl;
	
	if(m_listener)
	{ //if we are still waiting for other socket to connect, just remove this socket from the socket list
		m_webcamSocket->disconnect();
		m_webcamSocket->deleteLater();
		m_allSockets.remove(m_webcamSocket);
		m_webcamSocket=0l;
	}
	else // else, close the session
		sendBYEMessage();
	
}

void Webcam::slotSocketError(int errorCode)
{
	KBufferedSocket *socket=const_cast<KBufferedSocket*>(static_cast<const KBufferedSocket*>(sender()));
	kdDebug(14140) << k_funcinfo <<  socket << " -  " << errorCode << " : " << socket->errorString() << endl;
	//sendBYEMessage();
}

void Webcam::closeAllOtherSockets()
{
	//m_lisener->close();
	delete m_listener;
	m_listener=0l;
	
	QValueList<KBufferedSocket*>::iterator it;
	for ( it = m_allSockets.begin(); it != m_allSockets.end(); ++it )
	{
		KBufferedSocket *sock=(*it);
		if(sock != m_webcamSocket)
			delete sock;
	}
	m_allSockets.clear();
}


void Webcam::timerEvent( QTimerEvent *e )
{
	if(e->timerId() != m_timerId)
		return TransferContext::timerEvent(e);
	
//	kdDebug(14140) << k_funcinfo << endl;
		
	Kopete::AV::VideoDevicePool *videoDevice = Kopete::AV::VideoDevicePool::self();
	videoDevice->getFrame();
	QImage img;
	videoDevice->getImage(&img);
	
	if(m_widget)
		m_widget->newImage(img);
	
	if(img.width()!=320 || img.height()!=240)
	{
		kdWarning(14140) << k_funcinfo << "Bad image size " <<img.width() << "x" <<  img.height() << endl;
		return;
	}

	uchar *bits=img.bits();
	QByteArray image_data(img.width()*img.height()*3);
	uint b2=0;
	uint imgsize=img.width()*img.height()*4;
	for(uint f=0; f< imgsize; f+=4)
	{
		image_data[b2+0]=bits[f+2];
		image_data[b2+1]=bits[f+1];
		image_data[b2+2]=bits[f+0];
		b2+=3;
	}
	
	QByteArray frame=m_mimic->encode(image_data);
	
	
	kdDebug(14140) << k_funcinfo << "Sendinf frame of size " << frame.size() << endl;
	//build the header.
	QByteArray header;
	
	QDataStream writer(header, IO_WriteOnly);
	writer.setByteOrder(QDataStream::LittleEndian);
	writer << (Q_UINT16)24;  // header size
	writer << (Q_UINT16)img.width();
	writer << (Q_UINT16)img.height();
	writer << (Q_UINT16)0x0000; //wtf .?
	writer << (Q_UINT32)frame.size();
	writer << (Q_UINT8)('M') << (Q_UINT8)('L') << (Q_UINT8)('2') << (Q_UINT8)('0');
	writer << (Q_UINT32)0x00000000; //wtf .?
	writer << QTime::currentTime();  //FIXME:  possible midnight bug ?

	m_webcamSocket->writeBlock(header.data(), header.size());
	m_webcamSocket->writeBlock(frame.data(), frame.size());
}


}


#include "webcam.moc"

#endif

