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

// kde
#include <kdebug.h>
#include <kmdcodec.h>
#include <ktempfile.h>
#include <krun.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdeversion.h>
#include <kstandarddirs.h>


//kopete
#include <kopetetransfermanager.h>




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

	QCString dataMessage= QString(
			method + "\r\n"
			"To: <msnmsgr:"+m_msgHandle+">\r\n"
			"From: <msnmsgr:"+m_myHandle+">\r\n"
			"Via: MSNSLP/1.0/TLP ;branch={"+m_branch.upper()+"}\r\n"
			"CSeq: "+ CSeq +"\r\n"
			"Call-ID: {"+m_CallID.upper()+"}\r\n"
			"Max-Forwards: 0\r\n"
			"Content-Type: "+ contentType +"\r\n"
			"Content-Length: "+ QString::number(content.length()+1)+"\r\n"
			"\r\n" + content ).utf8(); //\0
	//the data message must be end by \0,  bye chance, QCString automaticaly appends \0 at the end of the QByteArray

	kdDebug(14141) << k_funcinfo << dataMessage << endl;

	sendP2PMessage(dataMessage);
}


void MSNP2P::sendP2PMessage(const QByteArray &dataMessage)
{
	const QCString messageHeader=QString(
			"MIME-Version: 1.0\r\n"
			"Content-Type: application/x-msnmsgrp2p\r\n"
			"P2P-Dest: "+ m_msgHandle  +"\r\n\r\n").utf8();
	const uint messageHeaderLength = messageHeader.length();


	QByteArray binHeader(48);
	binHeader.fill('\0'); //fill with 0 for starting

	if(m_msgIdentifier==0)
		m_msgIdentifier=rand()%0x0FFFFFF0+4;
	else if(m_offset==0)
		m_msgIdentifier++;


	//SessionID     (it's byll if the transfer has not started.  (i.e the footer == 0)
	unsigned long int sessionID= m_footer ? m_sessionId : 0;
	binHeader[0]=(char)(sessionID%256);
	binHeader[1]=(char)((unsigned long int)(sessionID/256)%256);
	binHeader[2]=(char)((unsigned long int)(sessionID/(256*256))%256);
	binHeader[3]=(char)((unsigned long int)(sessionID/(256*256*256))%256);

	//MessageID
	binHeader[4]=(char)(m_msgIdentifier%256);
	binHeader[5]=(char)((unsigned long int)(m_msgIdentifier/256)%256);
	binHeader[6]=(char)((unsigned long int)(m_msgIdentifier/(256*256))%256);
	binHeader[7]=(char)((unsigned long int)(m_msgIdentifier/(256*256*256))%256);

	//offset
	binHeader[8]=(char)(m_offset%256);
	binHeader[9]=(char)((unsigned long int)(m_offset/256)%256);
	binHeader[10]=(char)((unsigned long int)(m_offset/(256*256))%256);
	binHeader[11]=(char)((unsigned long int)(m_offset/(256*256*256))%256);

	unsigned int size=dataMessage.size();

	if(m_totalDataSize) //it's a splitted message
	{
		binHeader[16]=(char)(m_totalDataSize%256);
		binHeader[17]=(char)((unsigned long int)(m_totalDataSize/256)%256);
		binHeader[18]=(char)((unsigned long int)(m_totalDataSize/(256*256))%256);
		binHeader[19]=(char)((unsigned long int)(m_totalDataSize/(256*256*256))%256);

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
		binHeader[16]=(char)size%256;
		binHeader[17]=(int)size/256;
	}

	//message size
	binHeader[24]=(char)size%256;
	binHeader[25]=(int)size/256;

	//Ack sessionID
	binHeader[32]=(char)(rand()%256);
	binHeader[33]=(char)(rand()%256);
	binHeader[34]=(char)(rand()%256);
	binHeader[35]=(char)(rand()%256);

	/*binHeader[32]=0xDE;
	binHeader[33]=0xC7;
	binHeader[34]=0x07;
	binHeader[35]=0x14;*/


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

	const QCString messageHeader=QString(
			"MIME-Version: 1.0\r\n"
			"Content-Type: application/x-msnmsgrp2p\r\n"
			"P2P-Dest: "+ m_msgHandle  +"\r\n\r\n").utf8();
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
	binHeader[4]=(char)(m_msgIdentifier%256);
	binHeader[5]=(char)((unsigned long int)(m_msgIdentifier/256)%256);
	binHeader[6]=(char)((unsigned long int)(m_msgIdentifier/(256*256))%256);
	binHeader[7]=(char)((unsigned long int)(m_msgIdentifier/(256*256*256))%256);

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

		QString dataMessage=QCString((msgStr.message.data()+48) , msgStr.dataMessageSize);

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



#include "msnp2p.moc"

