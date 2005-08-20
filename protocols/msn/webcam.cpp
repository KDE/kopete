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

#include "dispatcher.h"

//#include "farsight_wrapper.h"
using namespace KNetwork;

namespace P2P {

Webcam::Webcam( const QString& to, Dispatcher *parent, Q_UINT32 sessionId)
	: TransferContext(parent)
{
	m_direction = Incoming;
	m_sessionId  = sessionId;
	m_recipient  = to;
	m_offset = 0l;
	m_listener  = 0l;
	m_webcamSocket=0L;
	m_state=wsNegotiating;
}

Webcam::~Webcam()
{
}

void Webcam::acknowledged()
{
//	kdDebug(14140) << k_funcinfo << endl;
	
// 	switch(m_state)
// 	{
// /*	case Invitation:
// 		{
// 			if(m_type == UserDisplayIcon)
// 			{
// 				m_state = Negotiation;
// 				Send data preparation message.
// 				sendDataPreparation();
// 			}
// 			break;
// 	}*/
// 		/*
// 		case Negotiation:
// 		{
// 			if(m_type == UserDisplayIcon)
// 			{
// 				<<< Data preparation acknowledge message.
// 				m_state = DataTransfer;
// 				m_identifier++;
// 				Start sending data.
// 				slotSendData();
// 			}
// 			break;
// 		}
// 		
// 		case DataTransfer:
// 			NOTE <<< Data acknowledged message.
// 			<<< Bye message should follow.
// 			if(m_type == File)
// 			{
// 				if(m_handshake == 0x01)
// 				{
// 					Data handshake acknowledge message.
// 					Start sending data.
// 					slotSendData();
// 				}
// 				else if(m_handshake == 0x02)
// 				{
// 					Data acknowledge message.
// 					Send the recipient a BYE message.
// 					m_state = Finished;
// 					sendMessage(BYE, "\r\n");
// 				}
// 			}
// 			
// 			break;
// 		*/
// /*		case Finished:
// 			if(m_type == File)
// 			{
// 				BYE acknowledge message.
// 				m_dispatcher->detach(this);
// 			}
// 			
// 		break;*/
// 	}
}




void Webcam::processMessage(const Message& message)
{
	if(message.header.dataOffset+message.header.dataSize >= message.header.totalDataSize)
		acknowledge( message );
	
	if(message.applicationIdentifier != 4l)
		return;

	QByteArray dataMessage=message.body;
	
	QString echoS="";
//QString debug2="";

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
		QRegExp rx("<rid>([1-9]*)</rid>.*<session>([1-9]*)</session>");
		rx.search(m_content);	
		QString viewerxml=xml(rx.cap(2).toUInt() , rx.cap(1).toUInt()).utf8();
		makeSIPMessage(  viewerxml );
//		FarsightWrapper *farsight=new FarsightWrapper( viewerxml , m_content , this );
		
		m_auth="recipientid="+rx.cap(1)+"&sessionid="+rx.cap(2)+"\r\n\r\n";
		
		
		m_listener = new KServerSocket("7786",this);
		//m_listener->setResolutionEnabled(true);
				// Create the callback that will try to accept incoming connections.
		QObject::connect(m_listener, SIGNAL(readyAccept()), this, SLOT(slotAccept()));
		QObject::connect(m_listener, SIGNAL(gotError(int)), this, SLOT(slotListenError(int)));
				// Listen for incoming connections.
		bool isListening = m_listener->listen(1);
		kdDebug(14140) << k_funcinfo << (isListening ? "listening" : "not listening") << endl;
		kdDebug(14140) << k_funcinfo << "local endpoint, " << m_listener->localAddress().nodeName() << endl;
				

	}
	else if(m_content.contains("receivedViewerData"))
	{

	}
	m_content=QString::null;
}

void Webcam::makeSIPMessage(const QString &message)
{
	QByteArray  dataMessage(10+message.length()*2);
	dataMessage[0]=0x80;
	dataMessage[1]=0x17; //XX
	dataMessage[2]=0x2a; //YY
	dataMessage[3]=0x01; //ZZ
	dataMessage[4]=0x08;
	dataMessage[5]=0x00;
	dataMessage[6]=message.length()*2+2;
	dataMessage[7]=0x00;
	dataMessage[8]=0x00;
	dataMessage[9]=0x00;
	for(uint f=0; f<message.length(); f++)
	{
		dataMessage[10+2*f]=message[f].latin1();
		dataMessage[11+2*f]=0x00;
	}

	QString echoS="";
//QString debug2="";

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

/////////////////////////////////

void Webcam::slotListenError(int errorCode)
{
	kdDebug(14140) << k_funcinfo << m_listener->errorString() << endl;
}

void Webcam::slotAccept()
{
	kdDebug(14140) << "####################################" <<k_funcinfo << endl;
	// Try to accept an incoming connection from the sending client.
	m_webcamSocket = static_cast<KBufferedSocket*>(m_listener->accept());
	if(!m_webcamSocket)
	{
		// NOTE If direct connection fails, the sending
		// client wil transfer the file data through the
		// existing session.
		kdDebug(14140) << k_funcinfo << "Direct connection failed." << endl;
		// Close the listening endpoint.
		m_listener->close();
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
}

void Webcam::slotSocketRead()
{
	int available = m_webcamSocket->bytesAvailable();
	kdDebug(14140) << k_funcinfo << available << ", bytes available." << endl;
	if(available > 0)
	{
		QByteArray buffer(available);
		m_webcamSocket->readBlock(buffer.data(), buffer.size());
		
		kdDebug(14140) << k_funcinfo << buffer.data() <<  endl;
		
		
		switch(m_state)
		{
			case wsNegotiating:
				if(QString(buffer) == m_auth )
				{
					QString s("connected\r\n\r\n");
					
					kdDebug(14140) << k_funcinfo << "Sending" << s << endl;
					
					QByteArray conne=s.utf8();
					
					m_socket->writeBlock(conne.data(), conne.size()-1);
				}
				else
				{
					kdWarning(14140) << k_funcinfo << "Auth failed" << endl;
					m_webcamSocket->disconnect();
				}
				break;
		}
		
		
	}
	
}

void Webcam::slotSocketClosed()
{
	kdDebug(14140) << k_funcinfo << endl;
}

void Webcam::slotSocketError(int errorCode)
{
	kdDebug(14140) << k_funcinfo <<  errorCode <<  endl;
}


}


#include "webcam.moc"

#endif

