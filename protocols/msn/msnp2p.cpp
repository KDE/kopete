/*
    msnp2p.cpp - msn p2p protocol

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

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

#include <stdlib.h>

// qt
#include <qregexp.h>
#include <qfile.h>

// kde
#include <kdebug.h>
#include <kmdcodec.h>
#include <ktempfile.h>
#include <krun.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdeversion.h>
#include <kstandarddirs.h>


MSNP2P::MSNP2P( QObject *parent , const char * name )
: QObject( parent , name )
{
	m_file=0l;
	m_Sfile=0L;
	m_msgIdentifier=0;
	m_sessionId=0;
	m_totalDataSize=0;
	m_offset=0;
}

MSNP2P::~MSNP2P()
{
	delete m_file;
	delete m_Sfile;
}


void MSNP2P::slotReadMessage( const QByteArray &msg )
{
	QString messageHeader=QCString(msg.data() , (msg.find('\0')==-1) ? msg.size() : msg.find('\0') );

	QRegExp rx("Content-Type: ([A-Za-z0-9$!*/\\-]*)");
	rx.search( messageHeader );
	QString type=rx.cap(1);

	if( type== "application/x-msnmsgrp2p"  )
	{
		//Get the starting position of the 48-bytes bunary header
		unsigned int startBinHeader=0;
		bool justCR=false;
		while(startBinHeader < msg.size()-2)
		{
			if( msg.data()[startBinHeader]=='\r')
				startBinHeader++;
			if( msg.data()[startBinHeader]=='\n' )
			{
				if(justCR) break;
				else justCR=true;
			}
			else justCR=false;
			startBinHeader++;
		}
		startBinHeader++;
		if(!justCR || startBinHeader+48 > msg.size()) return;  //no binary header, or not long enough   , TODO: error handling

		//Read some interesting field from the binary header
		unsigned int dataMessageSize=(int)(unsigned char)(msg.data()[startBinHeader+24]) + (int)((unsigned char)msg.data()[startBinHeader+25])*256;
		unsigned int totalSize=(int)(unsigned char)(msg.data()[startBinHeader+16]) + (int)((unsigned char)msg.data()[startBinHeader+17])*256 + (int)((unsigned char)msg.data()[startBinHeader+18])*256*256  + (int)((unsigned char)msg.data()[startBinHeader+19])*256*256*256;
		unsigned int dataOffset=(int)(unsigned char)(msg.data()[startBinHeader+8]) + (int)((unsigned char)msg.data()[startBinHeader+9])*256 + (int)((unsigned char)msg.data()[startBinHeader+10])*256*256  + (int)((unsigned char)msg.data()[startBinHeader+11])*256*256*256;

		if(dataMessageSize==0)
		{
			kdDebug(14140) << "MSNP2P::slotReadMessage: I do not care, it's a ACK     - flag= "  << (int)(unsigned char)(msg.data()[startBinHeader+28])  << endl;
			return;
		}

		if(msg.size() < startBinHeader+48+dataMessageSize)
		{
			//the message's size is shorter than the announced size
			//TODO error handling
			return;
		}

		QString dataMessage=QCString((msg.data()+startBinHeader+48) , dataMessageSize);

		if(m_msgHandle.isEmpty())
		{ //if these addresses were not previously set, get it, they should be provided in the first message at last.
			QRegExp rx("To: <msnmsgr:([^>]*)>");
			if( rx.search( dataMessage ) != -1 )
				m_myHandle=rx.cap(1);

			rx=QRegExp("From: <msnmsgr:([^>]*)>");
			if( rx.search( dataMessage ) != -1 )
				m_msgHandle=rx.cap(1);
		}

		//Send the ack if needed
		if(dataOffset+dataMessageSize>=totalSize)
			sendP2PAck( (msg.data()+startBinHeader) );



		if(m_file)  //we are already downloading something to this file
		{
			m_file->file()->writeBlock( (msg.data()+startBinHeader+48) , dataMessageSize );

			if(dataOffset+dataMessageSize >= totalSize) //the file is complete
			{
				m_file->close();

/*				//TODO: show the file in a better way
				#if KDE_IS_VERSION(3,1,90)
				KRun::runURL( m_file->name(), "image/png", true );
				#else
				KRun::runURL( m_file->name(), "image/png" );
				#endif

				delete m_file;*/
				emit fileReceived(m_file);
				m_file=0;

				//send the bye message
				QCString dataMessage= QString(
						"BYE MSNMSGR:"+m_msgHandle+"MSNSLP/1.0\r\n"
						"To: <msnmsgr:"+m_msgHandle+">\r\n"
						"From: <msnmsgr:"+m_myHandle+">\r\n"
						"Via: MSNSLP/1.0/TLP ;branch={A0D624A6-6C0C-4283-A9E0-BC97B4B46D32}\r\n"
 						"CSeq: 0\r\n"
 						"Call-ID: {9D79AE57-1BD5-444B-B14E-3FC9BB2B5D58}\r\n"
 						"Max-Forwards: 0\r\n"
 						"Content-Type: application/x-msnmsgr-sessionclosebody\r\n"
 						"Content-Length: 3\r\n\r\n" ).utf8();
				sendP2PMessage(dataMessage);

				//deleteLater();
			}
		}
		else
		{
			kdDebug(14140) << "MSNP2P::slotReadMessage: dataMessage: "  << dataMessage << endl;

			if(msg.data()[startBinHeader+48] == '\0' )
			{  //This can be only the data preparaion message.   prepare to download
				m_file=new KTempFile( locateLocal( "tmp", "msnpicture-" ), ".png" );
				m_file->setAutoDelete(true);
			}
   			else if (dataMessage.contains("INVITE"))
			{
				//Parse the message to get some info for replying with the 200 OK message
				QRegExp rx(";branch=\\{([0-9A-F\\-]*)\\}\r\n");
				rx.search( dataMessage );
				QString branch=rx.cap(1);

				rx=QRegExp("Call-ID: \\{([0-9A-F\\-]*)\\}\r\n");
				rx.search( dataMessage );
				QString callid=rx.cap(1);

				rx=QRegExp("SessionID: ([0-9]*)\r\n");
				rx.search( dataMessage );
				unsigned long int sessID=rx.cap(1).toUInt();


				QString content="SessionID: " + QString::number( sessID ) ;

				QCString dataMessage= QString(
						"MSNSLP/1.0 200 OK\r\n"
						"To: <msnmsgr:"+m_msgHandle+">\r\n"
						"From: <msnmsgr:"+m_myHandle+">\r\n"
						"Via: MSNSLP/1.0/TLP ;branch={"+ branch +"}\r\n"
						"CSeq: 1\r\n"
						"Call-ID: {"+callid+"}\r\n"
						"Max-Forwards: 0\r\n"
						"Content-Type: application/x-msnmsgr-sessionreqbody\r\n"
						"Content-Length: "+ QString::number(content.length()+5)+"\r\n"
						"\r\n" + content + "\r\n\r\n").utf8(); //\0

				sendP2PMessage(dataMessage);

				//send the data preparation message
				m_sessionId=sessID;
				QByteArray initM(4);
				initM.fill('\0');
				sendP2PMessage(initM);

				//prepare to send the file
				m_Sfile = new QFile( locateLocal( "appdata", "msnpicture-"+ m_myHandle.lower().replace(QRegExp("[./~]"),"-")  +".png" ) );
				if(!m_Sfile->open(IO_ReadOnly))  {/*error?*/}
    			m_totalDataSize=  m_Sfile->size();

				QTimer::singleShot( 10, this, SLOT(slotSendData()) ); //Go for upload
			}
			else if (dataMessage.contains("BYE"))
			{
				//deleteLater();
			}
		}
	}
	else
	{
//		kdDebug(14140) << "MSNSwitchBoardSocket::slotReadMessage: Unknown type '" << type << endl;
	}

}



void MSNP2P::requestDisplayPicture( const QString &myHandle, const QString &msgHandle, QString msnObject)
{
	//reset some field
	m_file=0l;
	m_Sfile=0L;
	m_msgIdentifier=0;
	m_sessionId=0;
	m_totalDataSize=0;
	m_offset=0;

	m_myHandle=myHandle;
	m_msgHandle=msgHandle;

	msnObject=QString::fromUtf8(KCodecs::base64Encode( msnObject.utf8() ));
	msnObject.replace("=" , QString::null ) ;

	unsigned long int sessID=rand()%0xFFFFFF00+4;

	QString content="EUF-GUID: {A4268EEC-FEC5-49E5-95C3-F126696BDBF6}\r\n"
 			"SessionID: "+ QString::number(sessID)+"\r\n"
 			"AppID: 1\r\n"
 			"Context: "  + msnObject;

	QCString dataMessage= QString(
			"INVITE MSNMSGR:"+ msgHandle + "  MSNSLP/1.0\r\n"
 			"To: <msnmsgr:"+msgHandle+">\r\n"
 			"From: <msnmsgr:"+myHandle+">\r\n"
			"Via: MSNSLP/1.0/TLP ;branch={33517CE4-02FC-4428-B6F4-39927229B722}\r\n"
			"CSeq: 0\r\n"
			"Call-ID: {9D79AE57-1BD5-444B-B14E-3FC9BB2B5D58}\r\n"
 			"Max-Forwards: 0\r\n"
 			"Content-Type: application/x-msnmsgr-sessionreqbody\r\n"
			"Content-Length: "+ QString::number(content.length()+5)+"\r\n"
			"\r\n" + content + "\r\n\r\n").utf8(); //\0

	sendP2PMessage(dataMessage);
}


void MSNP2P::sendP2PMessage(const QByteArray &dataMessage)
{
	if(m_sessionId == 0)
		kdDebug(14140) << k_funcinfo << QCString(dataMessage.data() , dataMessage.size()) << endl;

	QCString messageHeader=QString(
				"MIME-Version: 1.0\r\n"
 				"Content-Type: application/x-msnmsgrp2p\r\n"
 				"P2P-Dest: "+ m_msgHandle  +"\r\n\r\n").utf8();


	QByteArray binHeader(48);
	binHeader.fill('\0'); //fill with 0 for starting

	if(m_msgIdentifier==0)
		m_msgIdentifier=rand()%0x0FFFFFF0+4;
	else if(m_offset==0)
		m_msgIdentifier++;

	//SessionID
	binHeader[0]=(char)(m_sessionId%256);
	binHeader[1]=(char)((unsigned long int)(m_sessionId/256)%256);
	binHeader[2]=(char)((unsigned long int)(m_sessionId/(256*256))%256);
	binHeader[3]=(char)((unsigned long int)(m_sessionId/(256*256*256))%256);

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
	/*binHeader[32]=(char)(rand()%256);
	binHeader[33]=(char)(rand()%256);
	binHeader[34]=(char)(rand()%256);
	binHeader[35]=(char)(rand()%256);*/

	binHeader[32]=0xDE;
	binHeader[33]=0xC7;
	binHeader[34]=0x07;
	binHeader[35]=0x14;


	//merge all in a unique message
	QByteArray data( messageHeader.length() + binHeader.size() + dataMessage.size() + 4 );
	for(unsigned int f=0; f< messageHeader.length() ; f++)
		data[f]=messageHeader[f];
	for(unsigned int f=0; f< binHeader.size() ; f++)
		data[messageHeader.length()+f]=binHeader[f];
	for(unsigned int f=0; f< dataMessage.size() ; f++)
		data[messageHeader.length()+binHeader.size()+f]=dataMessage[f];
	for(unsigned int f=0; f< 4 ; f++) //footer
		data[messageHeader.length()+binHeader.size()+dataMessage.size()+f]='\0';

	if(m_sessionId!=0)
	{ //then, the footer ends with \1
		data[messageHeader.length()+binHeader.size() + dataMessage.size()  +3 ]='\1';
	}

	//send the message
	emit sendCommand("MSG", "D" , true , data , true );
}

void MSNP2P::sendP2PAck( const char* originalHeader )
{

	QCString messageHeader=QString(
				"MIME-Version: 1.0\r\n"
				"Content-Type: application/x-msnmsgrp2p\r\n"
				"P2P-Dest: "+ m_msgHandle  +"\r\n\r\n").utf8();


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


	QByteArray data( messageHeader.length() + binHeader.size() + 4 );
	for(unsigned int f=0; f< messageHeader.length() ; f++)
		data[f]=messageHeader[f];
	for(unsigned int f=0; f< binHeader.size() ; f++) //if binHeader is a QCString, it ends with \0 , which is ok
		data[messageHeader.length()+f]=binHeader[f];
	for(unsigned int f=0; f< 4 ; f++)
		data[messageHeader.length()+binHeader.size() +f ]='\0';

	emit sendCommand("MSG", "D" , true , data , true );
}


void MSNP2P::slotSendData()
{
	if(!m_Sfile)
		return;

	char data[1200];
	int bytesRead = m_Sfile->readBlock( data,1200 );

	QByteArray dataBA(bytesRead);
	for (  int f = 0; f < bytesRead; f++ )
		dataBA[f] = data[f];

	kdDebug(14140) << "MSNP2P::slotSendData: offset="  << m_offset << "  size=" << bytesRead << "   totalSize=" << m_totalDataSize << "     sent=" << m_offset+bytesRead <<   endl;

	sendP2PMessage(dataBA);

	if( m_totalDataSize == 0  ) //this has been reseted bacause the file is completely send
	{
		kdDebug(14140) << "MSNP2P::slotSendData: FINISHED! wait for the BYE message" <<   endl;
		delete m_Sfile;
		m_Sfile=0L;
		m_sessionId=0;
	}
	else
		QTimer::singleShot( 10, this, SLOT(slotSendData()) );
}




#include "msnp2p.moc"

