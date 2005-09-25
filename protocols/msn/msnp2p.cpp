/*
    msnp2p.cpp - msn p2p protocol

    Copyright (c) 2003-2005 by Olivier Goffart        <ogoffart@ kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "msnp2p.h"
#include "msnp2pdisplatcher.h"
#include "msnp2pincoming.h"
#include "msnp2poutgoing.h"

#include <stdlib.h>

// qt
#include <qregexp.h>
#include <qfile.h>
#include <qtextcodec.h>
//Added by qt3to4:
#include <QByteArray>

// kde
#include <kdebug.h>
#include <kcodecs.h>
#include <ktempfile.h>
#include <krun.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdeversion.h>
#include <kstandarddirs.h>


//kopete
#include <kopetetransfermanager.h>

/**
 * if VAR is a char*  and VAL is an integer, assing VAR[START] to VAR[START+4]  the value of VAL
 */
#define MKDWORD(VAR, START, VAL) {\
	(VAR)[(START)]=  (char)( ((VAL)&0x000000FF) ) ;  \
	(VAR)[(START)+1]=(char)( ((VAL)&0x0000FF00)  >> 8 ) ; \
	(VAR)[(START)+2]=(char)( ((VAL)&0x00FF0000)  >> 16 ) ; \
	(VAR)[(START)+3]=(char)( ((VAL)&0xFF000000)  >> 24 ) ; \
}

MSNP2P::MSNP2P( unsigned long int sessionID , MSNP2PDisplatcher *parent ) : QObject (parent)
{
	m_msgIdentifier=0;
	m_sessionId=sessionID;
	m_totalDataSize=0;
	m_offset=0;
	m_parent=parent;
	m_footer='\0';
}

MSNP2P::MSNP2P( QObject *p ) : QObject( p )
{
	m_msgIdentifier=0;
	m_sessionId=0;
	m_totalDataSize=0;
	m_offset=0;
	m_parent=static_cast<MSNP2PDisplatcher*>(this);  //dynamic_cast doesn't work yet here.
	m_footer='\0';
}

MSNP2P::~MSNP2P() {}

void MSNP2P::makeMSNSLPMessage( MessageType type, QString content )
{
	QString contentType= QString( "application/x-msnmsgr-sessionreqbody" );
	QString method;
	QString CSeq;

	switch(type)
	{
		case INVITE:
			method="INVITE MSNMSGR:"+ m_msgHandle + "  MSNSLP/1.0";
			if(!content.contains("AppID"))
				contentType="application/x-msnmsgr-transreqbody";
			CSeq="0";
			break;
		case DECLINE:
			method="MSNSLP/1.0 603 DECLINE";
			CSeq="1";
			break;
		case ERROR:
			contentType="null";
			method="MSNSLP/1.0 500 Internal Error";
			CSeq="1";
			break;
		case OK:
		{
			MSNP2PIncoming* in = dynamic_cast<MSNP2PIncoming*>(this);
			if(in && in->m_kopeteTransfer ) //this is the best way i found to identify the second OK message
				contentType="application/x-msnmsgr-transreqbody";
			method="MSNSLP/1.0 200 OK";
			CSeq="1";
			break;
		}
		case BYE:
			contentType="application/x-msnmsgr-sessionclosebody";
			method="BYE MSNMSGR:"+m_msgHandle+" MSNSLP/1.0";
			CSeq="0";
			break;
	}

	QByteArray dataMessage= QString(
			method + "\r\n"
			"To: <msnmsgr:"+m_msgHandle+">\r\n"
			"From: <msnmsgr:"+m_myHandle+">\r\n"
			"Via: MSNSLP/1.0/TLP ;branch={"+m_branch.toUpper()+"}\r\n"
			"CSeq: "+ CSeq +"\r\n"
			"Call-ID: {"+m_CallID.toUpper()+"}\r\n"
			"Max-Forwards: 0\r\n"
			"Content-Type: "+ contentType +"\r\n"
			"Content-Length: "+ QString::number(content.length()+1)+"\r\n"
			"\r\n" + content ).toUtf8(); //\0
	//the data message must be end by \0,  bye chance, QCString automaticaly appends \0 at the end of the QByteArray

	kdDebug(14141) << k_funcinfo << dataMessage << endl;

	sendP2PMessage(dataMessage);
}


void MSNP2P::sendP2PMessage(const QByteArray &dataMessage)
{
	const QByteArray messageHeader=QString(
			"MIME-Version: 1.0\r\n"
			"Content-Type: application/x-msnmsgrp2p\r\n"
			"P2P-Dest: "+ m_msgHandle  +"\r\n\r\n").toUtf8();
	const uint messageHeaderLength = messageHeader.length();


	QByteArray binHeader(48);
	binHeader.fill('\0'); //fill with 0 for starting

	if(m_msgIdentifier==0)
		m_msgIdentifier=rand()%0x0FFFFFF0+4;
	else if(m_offset==0)
		m_msgIdentifier++;


	//SessionID     (it's nyll if the transfer has not started.  (i.e the footer == 0)
	if(m_footer)
		MKDWORD(binHeader,0,m_sessionId);

	//MessageID
	MKDWORD(binHeader,4,m_msgIdentifier);

	//offset
	MKDWORD(binHeader,8,m_offset);

	unsigned int size=dataMessage.size();
	if(m_totalDataSize) //it's a splitted message
	{
		MKDWORD(binHeader,16,m_totalDataSize);

		//update offset
		m_offset+=size;
		if(m_offset>=m_totalDataSize)
		{  //message completely sent, reset values
			m_offset=0;
			m_totalDataSize=0;
		}
	}
	else //not a splitted message, the total size is the current size
	{
		MKDWORD(binHeader,16,size);
	}

	//message size
	MKDWORD(binHeader,24,size);

	if(m_footer=='\1' && size>4)
	{	//when sending the data of an image, the flag is set to 0x20
		//  size>4 is because the data preparation message has not this flag.
		binHeader[28]=0x20;
	}

	//Ack sessionID
#if ! MSN_WEBCAM
	MKDWORD(binHeader,32,rand()%0x8FFFFFF0+4);
#else
	//For webcam, the OK and INVITE message should have the smae id, but not others.
	binHeader[32]=0xDE;
	binHeader[33]=0xC7;
	binHeader[34]=0x07;
	binHeader[35]=0x14;
#endif


	//merge all in a unique message
	QByteArray data( messageHeaderLength + binHeader.size() + dataMessage.size() + 4 );
	for(unsigned int f=0; f< messageHeaderLength ; f++)
		data[f]=messageHeader[f];
	for(unsigned int f=0; f< binHeader.size() ; f++)
		data[messageHeaderLength+f]=binHeader[f];
	for(unsigned int f=0; f< dataMessage.size() ; f++)
		data[messageHeaderLength+binHeader.size()+f]=dataMessage[f];
	for(unsigned int f=0; f< 4 ; f++) //footer
		data[messageHeaderLength+binHeader.size()+dataMessage.size()+f]='\0';

	data[messageHeaderLength+binHeader.size() + dataMessage.size()  +3 ]=m_footer;

	//send the message
	m_parent->sendCommand("MSG", "D" , true , data , true );
}

void MSNP2P::sendP2PAck( const char* originalHeader )
{

	const QByteArray messageHeader=QString(
			"MIME-Version: 1.0\r\n"
			"Content-Type: application/x-msnmsgrp2p\r\n"
			"P2P-Dest: "+ m_msgHandle  +"\r\n\r\n").toUtf8();
	const uint messageHeaderLength = messageHeader.length();


	QByteArray binHeader(48);
	binHeader.fill('\0'); //fill with 0 for starting

	//sessionID
	binHeader[0]=originalHeader[0];
	binHeader[1]=originalHeader[1];
	binHeader[2]=originalHeader[2];
	binHeader[3]=originalHeader[3];

	//MessageID
	bool a=false;
	if(m_msgIdentifier==0)
	{
		m_msgIdentifier=rand()%0xFFFFFE00+10;
		a=true;
	}
	else
		m_msgIdentifier++;
	MKDWORD(binHeader,4,m_msgIdentifier);

	if(a)
		m_msgIdentifier-=4;

	//total size
	binHeader[16]=originalHeader[16];
	binHeader[17]=originalHeader[17];
	binHeader[18]=originalHeader[18];
	binHeader[19]=originalHeader[19];
	binHeader[20]=originalHeader[20];
	binHeader[21]=originalHeader[21];
	binHeader[22]=originalHeader[22];
	binHeader[23]=originalHeader[23];

	//flag
	binHeader[28]=(char)0x02;

	//ack sessionID
	binHeader[32]=originalHeader[4];
	binHeader[33]=originalHeader[5];
	binHeader[34]=originalHeader[6];
	binHeader[35]=originalHeader[7];

	//ack unique id
	binHeader[36]=originalHeader[32];
	binHeader[37]=originalHeader[33];
	binHeader[38]=originalHeader[34];
	binHeader[39]=originalHeader[35];

	//ack data size
	binHeader[40]=originalHeader[16];
	binHeader[41]=originalHeader[17];
	binHeader[42]=originalHeader[18];
	binHeader[43]=originalHeader[19];
	binHeader[44]=originalHeader[20];
	binHeader[45]=originalHeader[21];
	binHeader[46]=originalHeader[22];
	binHeader[47]=originalHeader[23];


	QByteArray data( messageHeaderLength + binHeader.size() + 4 );
	for(unsigned int f=0; f< messageHeaderLength ; f++)
		data[f]=messageHeader[f];
	for(unsigned int f=0; f< binHeader.size() ; f++) //if binHeader is a QCString, it ends with \0 , which is ok
		data[messageHeaderLength+f]=binHeader[f];
	for(unsigned int f=0; f< 4 ; f++)
		data[messageHeaderLength+binHeader.size() +f ]='\0';

	m_parent->sendCommand("MSG", "D" , true , data , true );
}

void MSNP2P::error()
{
	kdDebug(14140) << k_funcinfo   << endl;
	makeMSNSLPMessage( ERROR, QString::null );
	m_parent->finished(this);
}


void MSNP2P::parseMessage(MessageStruct &msgStr)
{
	if(m_msgHandle.isEmpty())
	{ //if these addresses were not previously set, get it, they should be provided in the first message at last.

		QString dataMessage=QByteArray((msgStr.message.data()+48) , msgStr.dataMessageSize);

		QRegExp rx("To: <msnmsgr:([^>]*)>");
		if( rx.search( dataMessage ) != -1 )
			m_myHandle=rx.cap(1);

		rx=QRegExp("From: <msnmsgr:([^>]*)>");
		if( rx.search( dataMessage ) != -1 )
			m_msgHandle=rx.cap(1);
	}

	//Send the ack if needed
	if(msgStr.dataOffset+msgStr.dataMessageSize>=msgStr.totalSize)
		sendP2PAck( msgStr.message.data() );
}

//---------------------------------------------------------------

#if MSN_WEBCAM

MSNP2PWebcam::MSNP2PWebcam( unsigned long int sessionID , MSNP2PDisplatcher *parent )
	: MSNP2P(sessionID , parent)
{
}

MSNP2PWebcam::~MSNP2PWebcam()
{
}





void MSNP2PWebcam::parseMessage(MessageStruct &msgStr)
{
	MSNP2P::parseMessage(msgStr);
	if(msgStr.message[msgStr.message.size()-1]!='\4')
		return;

	QByteArray dataMessage;
	dataMessage.duplicate((char*)(msgStr.message.data()+48) , msgStr.dataMessageSize);
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
		m_content+=dataMessage[pos];
	}

	if( msgStr.dataMessageSize+msgStr.dataOffset < msgStr.totalSize )
		return;
	
	kdDebug(14141) << k_funcinfo << "Message contents: " << m_content << "\n" << endl;
	if(m_content.length() < 5)
		makeSIPMessage(m_content);
	else if(m_content.contains("<producer>"))
	{
		makeSIPMessage(m_content.replace("producer","viewer"));
	}
	m_content=QString::null;
}

void MSNP2PWebcam::makeSIPMessage(const QString &message)
{
	QByteArray  dataMessage(10+message.length()*2);
	dataMessage[0]=0x80;
	dataMessage[1]=0x17;
	dataMessage[2]=0x2a;
	dataMessage[3]=0x01;
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

	m_footer='\4';
	sendBigP2PMessage(dataMessage);
	m_footer='\0';
}

void MSNP2PWebcam::sendBigP2PMessage( const QByteArray & dataMessage)
{
	unsigned int size=m_totalDataSize=dataMessage.size();
	
	for(unsigned int f=0;f<size;f+=1200)
	{
		m_offset=f;
		QByteArray dm2;
		dm2.duplicate(dataMessage.data()+m_offset, QMIN(1200,m_totalDataSize-m_offset));
		sendP2PMessage(dm2);
	}
}


void MSNP2PWebcam::error()
{
	MSNP2P::error();
}

#endif


#include "msnp2p.moc"

