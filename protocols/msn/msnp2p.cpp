/*
    msnp2p.cpp - msn p2p protocol

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>

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
#include <kopetemessagemanager.h>  // { Just for getting the contact
#include <kopeteaccount.h>         // {
#include <kopetetransfermanager.h>


MSNP2P::MSNP2P( QObject *parent , const char * name )
: QObject( parent , name )
{
	m_file=0l;
	m_Sfile=0L;
	m_Rfile=0L;
	m_msgIdentifier=0;
	m_sessionId=0;
	m_totalDataSize=0;
	m_offset=0;
	m_kopeteTransfer=0L;
}

MSNP2P::~MSNP2P()
{
	if(m_file)
		delete m_file;
	else
		delete m_Rfile;
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
		if(!justCR || startBinHeader+48 > msg.size())
		{	//no binary header, or not long enough
			if(m_kopeteTransfer)
			{
				m_kopeteTransfer->slotError( KIO::ERR_INTERNAL , i18n("Malformed packet received")  );
				m_kopeteTransfer=0L;
			}
			abortCurrentTransfer();
			return;
		}

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
			if(m_kopeteTransfer)
			{
				m_kopeteTransfer->slotError( KIO::ERR_INTERNAL , i18n("Malformed packet received")  );
				m_kopeteTransfer=0L;
			}
			abortCurrentTransfer();
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

		if(m_Rfile)  //we are already downloading something to this file
		{
			//m_file->file()->writeBlock( (msg.data()+startBinHeader+48) , dataMessageSize );
			m_Rfile->writeBlock( (msg.data()+startBinHeader+48) , dataMessageSize );

			if(m_kopeteTransfer)
				m_kopeteTransfer->slotProcessed( dataOffset+dataMessageSize );

			if(dataOffset+dataMessageSize >= totalSize) //the file is complete
			{
				if(m_file)
				{
					m_file->close();
					emit fileReceived(m_file , m_obj);
					m_file=0;
					m_Rfile=0L;
				}
				else
				{
					if(m_kopeteTransfer) m_kopeteTransfer->slotComplete();
					m_Rfile->close();
				}
/*
				delete m_file;*/

				//send the bye message
				QCString dataMessage= QString(
						"BYE MSNMSGR:"+m_msgHandle+"MSNSLP/1.0\r\n"
						"To: <msnmsgr:"+m_msgHandle+">\r\n"
						"From: <msnmsgr:"+m_myHandle+">\r\n"
						"Via: MSNSLP/1.0/TLP ;branch={A0D624A6-6C0C-4283-A9E0-BC97B4B46D32}\r\n"
 						"CSeq: 0\r\n"
 						"Call-ID: {"+m_CallID.upper()+"}\r\n"
 						"Max-Forwards: 0\r\n"
 						"Content-Type: application/x-msnmsgr-sessionclosebody\r\n"
 						"Content-Length: 3\r\n\r\n" ).utf8();
				sendP2PMessage(dataMessage);

				//deleteLater();
			}
		}
		else
		{
			kdDebug(14141) << "MSNP2P::slotReadMessage: dataMessage: "  << dataMessage << endl;

			if(msg.data()[startBinHeader+48] == '\0' )
			{  //This can be only the data preparaion message.   prepare to download
				m_file=new KTempFile( locateLocal( "tmp", "msnpicture-" ), ".png" );
				m_file->setAutoDelete(true);
				m_Rfile=m_file->file();
			}
   			else if (dataMessage.contains("INVITE"))
			{

//			kdDebug(14141) << "MSNP2P::slotReadMessage: dataMessage: "  << dataMessage << endl;


				//Parse the message to get some info for replying with the 200 OK message
				QRegExp rx(";branch=\\{([0-9A-F\\-]*)\\}\r\n");
				rx.search( dataMessage );
				QString branch=rx.cap(1);

				rx=QRegExp("Call-ID: \\{([0-9A-F\\-]*)\\}\r\n");
				rx.search( dataMessage );
				QString callid=rx.cap(1);

				rx=QRegExp("SessionID: ([0-9]*)\r\n");
				rx.search( dataMessage );
				m_sessionId=rx.cap(1).toUInt();

				rx=QRegExp("AppID: ([0-9]*)\r\n");
				rx.search( dataMessage );
				unsigned long int AppID=rx.cap(1).toUInt();

				if(m_kopeteTransfer) // we are probably negotiating a file transfer
				{
					QRegExp rx(";branch=\\{([0-9A-F\\-]*)\\}\r\n");
					rx.search( dataMessage );
					QString branch=rx.cap(1);

					rx=QRegExp("Call-ID: \\{([0-9A-F\\-]*)\\}\r\n");
					rx.search( dataMessage );
					QString callid=rx.cap(1);

					QString content="Bridge: TCPv1\r\n"
									"Listening: false\r\n"
									"Nonce: {00000000-0000-0000-0000-000000000000}";

					QCString dataMessage= QString(
								"MSNSLP/1.0 200 OK\r\n"
								"To: <msnmsgr:"+m_msgHandle+">\r\n"
								"From: <msnmsgr:"+m_myHandle+">\r\n"
								"Via: MSNSLP/1.0/TLP ;branch={"+ branch +"}\r\n"
								"CSeq: 1\r\n"
								"Call-ID: {"+callid+"}\r\n"
								"Max-Forwards: 0\r\n"
								"Content-Type: application/x-msnmsgr-transreqbody\r\n"
								"Content-Length: "+ QString::number(content.length()+5)+"\r\n"
								"\r\n" + content + "\r\n\r\n").utf8(); //\0

					//MSNP2P beleive that the transfer already startyed when sessionID !=0 )
					unsigned long int si=m_sessionId;
					m_sessionId=0;
					sendP2PMessage(dataMessage);
					m_sessionId=si; //reset the sessionID
					m_Rfile=new QFile( m_kopeteTransfer->destinationURL().path() );
					if(!m_Rfile->open(IO_WriteOnly))
					{
						if(m_kopeteTransfer)
						{
							//TODO: handle the QFILE error
							m_kopeteTransfer->slotError( KIO::ERR_CANNOT_OPEN_FOR_WRITING , i18n("Cannot open file for writing")  );
							m_kopeteTransfer=0L;
							return;
						}
						abortCurrentTransfer();
					}
					else
						kdDebug(14140) << "MSNP2P::slotReadMessage: GO GO GO GO: "  << m_kopeteTransfer->info().file() << endl;
				}
				else if(AppID==1) //Ask for a msn picture, or emoticon,  currently, we always send the display picture
				{
					QString content="SessionID: " + QString::number( m_sessionId ) ;

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

					QByteArray initM(4);
					initM.fill('\0');
					sendP2PMessage(initM);

					//prepare to send the file
					m_Sfile = new QFile( locateLocal( "appdata", "msnpicture-"+ m_myHandle.lower().replace(QRegExp("[./~]"),"-")  +".png" ) );
					if(!m_Sfile->open(IO_ReadOnly))  {/* TODO: error?*/}
					m_totalDataSize=  m_Sfile->size();

					QTimer::singleShot( 10, this, SLOT(slotSendData()) ); //Go for upload
				}
				else if(AppID==2) //filetransfer
				{
					rx=QRegExp("Context: ([0-9a-zA-Z+/=]*)");
					rx.search( dataMessage );
					QString context=rx.cap(1);

					QByteArray binaryContext;
					KCodecs::base64Decode( context.utf8() , binaryContext );

					if(binaryContext.size() < 21 )
					{ //security,  don't let hackers crash everithing.
						//TODO: handle error
						return;
					}

					//the filename is conteined in the context from the 19st char to the end.  (in utf-16)
					QTextCodec *codec = QTextCodec::codecForName("ISO-10646-UCS-2");
					if(!codec)
						return; //abort();
					QString filename = codec->toUnicode(binaryContext.data()+19 , binaryContext.size()-19) ;
					//TODO FIXME !!!!!!!!!!!!!!!!!!!


					//the size is placed in the context in the bytes 8..12  (source: the amsn code)
					unsigned long int filesize= (unsigned char)(binaryContext[8]) + (unsigned char)(binaryContext[9]) *256 + (unsigned char)(binaryContext[10]) *65536 + (unsigned char)(binaryContext[11]) *16777216 ;

			/*		unsigned char q1=binaryContext[8];
					unsigned char q2=binaryContext[9];
					unsigned char q3=binaryContext[10];
					unsigned char q4=binaryContext[11];

					unsigned long int filesize= q1 + q2<<8 + q3<<16 + q4<<24;


					QString debug= QString::number((unsigned char)binaryContext[8]) +" " +QString::number((unsigned char)binaryContext[9]) +" " +QString::number((unsigned char)binaryContext[10]) +" " +QString::number((unsigned char)binaryContext[11]) +" "+ QString::number((unsigned char)binaryContext[12])  + " et le résultat " + QString::number( filesize1 )  +"\n" +
						" q1= " +  (uint)q1 + " q2= " +  (uint)q2 + " q3= " +  (uint)q3 + " q4= " +  (uint)q4 + " et le résultat " + QString::number( filesize) ;*/

					//ugly hack to get the contact
					KopeteContact *c=0L;
					if(parent())
					{
						KopeteMessageManager *kmm=dynamic_cast<KopeteMessageManager*>(parent()->parent());
						if(kmm)
							c=kmm->account()->contacts()[m_msgHandle];
					}
					disconnect(KopeteTransferManager::transferManager(), 0L , this, 0L);
					connect(KopeteTransferManager::transferManager() , SIGNAL(accepted(KopeteTransfer*, const QString& )) ,
							this, SLOT(slotTransferAccepted(KopeteTransfer*, const QString& )));
					connect(KopeteTransferManager::transferManager() , SIGNAL(refused( const KopeteFileTransferInfo & ) ),
							this, SLOT( slotFileTransferRefused( const KopeteFileTransferInfo & ) ) );

					QString description="Warning: I can't extract correctly the filename due to an encoding problem.\n"
					                  "the extracted filename looks like: " + filename ;
					filename= "msnfile";

					KopeteTransferManager::transferManager()->askIncomingTransfer(c  , filename , filesize, description, QString::number(m_sessionId)+":"+branch+":"+callid);

				}
				else
				{
					QCString dataMessage= QString(
							"MSNSLP/1.0 500 Internal Error\r\n"
							"To: <msnmsgr:"+m_msgHandle+">\r\n"
							"From: <msnmsgr:"+m_myHandle+">\r\n"
							"Via: MSNSLP/1.0/TLP ;branch={"+ branch +"}\r\n"
							"CSeq: 1\r\n"
							"Call-ID: {"+callid+"}\r\n"
							"Max-Forwards: 0\r\n"
							"Content-Type: null\r\n"
							"Content-Length: 0\r\n\r\n").utf8(); //\0
					sendP2PMessage(dataMessage);
				}
			}
			else if (dataMessage.contains("BYE"))
			{
				//deleteLater();
			}
		}
	}
	else
	{
		kdDebug(14140) << "MSNSwitchBoardSocket::slotReadMessage: Unknown type '" << type << endl;
	}

}



void MSNP2P::requestDisplayPicture( const QString &myHandle, const QString &msgHandle, QString msnObject)
{
	//reset some field
/*	m_file=0l;
	m_Sfile=0L;
	m_msgIdentifier=0;
	m_sessionId=0;
	m_totalDataSize=0;
	m_offset=0;*/
	m_sessionId=0;

	m_myHandle=myHandle;
	m_msgHandle=msgHandle;
	m_obj=msnObject;

	msnObject=QString::fromUtf8(KCodecs::base64Encode( msnObject.utf8() ));
	msnObject.replace("=" , QString::null ) ;

	unsigned long int sessID=rand()%0xFFFFFF00+4;
	QString branch= QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-" + QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-" + QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)  + "-" + QString::number(rand()%0xAAFF+0x1111, 16) + "-" + QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)+QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)+QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16);
	m_CallID= QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-" + QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-" + QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)  + "-" + QString::number(rand()%0xAAFF+0x1111, 16) + "-" + QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)+QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)+QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16); ;

	QString content="EUF-GUID: {A4268EEC-FEC5-49E5-95C3-F126696BDBF6}\r\n"
 			"SessionID: "+ QString::number(sessID)+"\r\n"
 			"AppID: 1\r\n"
 			"Context: "  + msnObject;

	QCString dataMessage= QString(
			"INVITE MSNMSGR:"+ msgHandle + "  MSNSLP/1.0\r\n"
 			"To: <msnmsgr:"+msgHandle+">\r\n"
 			"From: <msnmsgr:"+myHandle+">\r\n"
			"Via: MSNSLP/1.0/TLP ;branch={"+branch.upper()+"}\r\n"
			"CSeq: 0\r\n"
			"Call-ID: {"+m_CallID.upper()+"}\r\n"
 			"Max-Forwards: 0\r\n"
 			"Content-Type: application/x-msnmsgr-sessionreqbody\r\n"
			"Content-Length: "+ QString::number(content.length()+5)+"\r\n"
			"\r\n" + content + "\r\n\r\n").utf8(); //\0

	sendP2PMessage(dataMessage);
}


void MSNP2P::sendP2PMessage(const QByteArray &dataMessage)
{
	if(m_sessionId == 0)
		kdDebug(14141) << k_funcinfo << QCString(dataMessage.data() , dataMessage.size()) << endl;

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

//	kdDebug(14140) << "MSNP2P::slotSendData: offset="  << m_offset << "  size=" << bytesRead << "   totalSize=" << m_totalDataSize << "     sent=" << m_offset+bytesRead <<   endl;

	sendP2PMessage(dataBA);

	if( m_totalDataSize == 0  ) //this has been reseted bacause the file is completely send
	{
//		kd(14140) << "MSNP2P::slotSendData: FINISHED! wait for the BYE message" <<   endl;
		delete m_Sfile;
		m_Sfile=0L;
		m_sessionId=0;
	}
	else
		QTimer::singleShot( 10, this, SLOT(slotSendData()) );
}



void MSNP2P::slotTransferAccepted(KopeteTransfer* transfer, const QString& /*filename*/ )
{
	QStringList internalId=QStringList::split(":" , transfer->info().internalId() );
	if(internalId[0].toUInt() == m_sessionId )
	{
		m_kopeteTransfer=transfer;

		QObject::connect(transfer , SIGNAL(transferCanceled()), this, SLOT(abortCurrentTransfer()));
		QObject::connect(transfer,  SIGNAL(destroyed()) , this , SLOT(slotKopeteTransferDestroyed()));

		QString branch=internalId[1];
		QString callid=internalId[2];

		QString content="SessionID: " + QString::number( m_sessionId ) ;

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

		//MSNP2P beleive that the transfer already startyed when sessionID !=0 )
		m_sessionId=0;

		sendP2PMessage(dataMessage);

		m_sessionId=internalId[0].toUInt(); //reset the sessionID
	}
}

void MSNP2P::slotFileTransferRefused( const KopeteFileTransferInfo &info )
{
	QStringList internalId=QStringList::split(":" , info.internalId() );
	kdDebug(14140) << k_funcinfo << internalId << endl;
	if(internalId[0].toUInt() == m_sessionId )
	{
		QString branch=internalId[1];
		QString callid=internalId[2];

		QString content="SessionID: " + QString::number( m_sessionId ) ;

		QCString dataMessage= QString(
					"MSNSLP/1.0 603 DECLINE\r\n"
					"To: <msnmsgr:"+m_msgHandle+">\r\n"
					"From: <msnmsgr:"+m_myHandle+">\r\n"
					"Via: MSNSLP/1.0/TLP ;branch={"+ branch +"}\r\n"
					"CSeq: 1\r\n"
					"Call-ID: {"+callid+"}\r\n"
					"Max-Forwards: 0\r\n"
					"Content-Type: application/x-msnmsgr-sessionreqbody\r\n"
					"Content-Length: "+ QString::number(content.length()+5)+"\r\n"
					"\r\n" + content + "\r\n\r\n").utf8(); //\0

		//MSNP2P beleive that the transfer already startyed when sessionID !=0 )
		m_sessionId=0;

		sendP2PMessage(dataMessage);

//		m_sessionId=internalId[0].toUInt(); //reset the sessionID
	}

}


void MSNP2P::abortCurrentTransfer()
{
	if(m_kopeteTransfer)
	{
		delete m_Rfile;
		m_Rfile=0L;

		QCString dataMessage= QString(
				"BYE MSNMSGR:"+m_msgHandle+"MSNSLP/1.0\r\n"
				"To: <msnmsgr:"+m_msgHandle+">\r\n"
				"From: <msnmsgr:"+m_myHandle+">\r\n"
				"Via: MSNSLP/1.0/TLP ;branch={A0D624A6-6C0C-4283-A9E0-BC97B4B46D32}\r\n"
				"CSeq: 0\r\n"
				"Call-ID: {"+m_CallID.upper()+"}\r\n"
				"Max-Forwards: 0\r\n"
				"Content-Type: application/x-msnmsgr-sessionclosebody\r\n"
				"Content-Length: 3\r\n\r\n" ).utf8();
		sendP2PMessage(dataMessage);


		m_sessionId=0;
		//TODO: send BYE message
	}
}


void MSNP2P::slotKopeteTransferDestroyed()
{
	m_kopeteTransfer=0L;
	kdDebug(14140) << k_funcinfo << endl;
}

#include "msnp2p.moc"

