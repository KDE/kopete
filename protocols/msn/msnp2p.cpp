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


static QString randomid()
{
	return QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-"
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-"
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-"
			+ QString::number(rand()%0xAAFF+0x1111, 16) + "-"
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16);
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


//-------------------

MSNP2PDisplatcher::MSNP2PDisplatcher( QObject *parent , const char * /*name*/ )
	: MSNP2P( parent /*, name*/ )
{
}

MSNP2PDisplatcher::~MSNP2PDisplatcher()
{
	// not needed since we are the parent
	/*QMap<unsigned long int , MSNP2P* >::iterator it;
	for ( it = m_p2pList.begin(); it != m_p2pList.end(); ++it )
	delete it.data();*/
}


void MSNP2PDisplatcher::slotReadMessage( const QByteArray &msg )
{
	//parse the message
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
			error();
			return;
		}


		MessageStruct msgStr;

		//Read some interesting field from the binary header
		unsigned int sessionID=(int)(unsigned char)(msg.data()[startBinHeader])
				+ (int)((unsigned char)msg.data()[startBinHeader+1])*256
				+ (int)((unsigned char)msg.data()[startBinHeader+2])*256*256
				+ (int)((unsigned char)msg.data()[startBinHeader+3])*256*256*256;
		msgStr.dataMessageSize=(int)(unsigned char)(msg.data()[startBinHeader+24])
				+ (int)((unsigned char)msg.data()[startBinHeader+25])*256;
		msgStr.totalSize=(int)(unsigned char)(msg.data()[startBinHeader+16])
				+ (int)((unsigned char)msg.data()[startBinHeader+17])*256
				+ (int)((unsigned char)msg.data()[startBinHeader+18])*256*256
				+ (int)((unsigned char)msg.data()[startBinHeader+19])*256*256*256;
		msgStr.dataOffset=(int)(unsigned char)(msg.data()[startBinHeader+8])
				+ (int)((unsigned char)msg.data()[startBinHeader+9])*256
				+ (int)((unsigned char)msg.data()[startBinHeader+10])*256*256
				+ (int)((unsigned char)msg.data()[startBinHeader+11])*256*256*256;

		msgStr.message.duplicate( (msg.data()+startBinHeader) , msg.size()-startBinHeader);

		if(msgStr.dataMessageSize==0)
		{
			kdDebug(14140) << k_funcinfo << " I do not care, it's a ACK     - flag= "  << (int)(unsigned char)(msg.data()[startBinHeader+28])  << endl;
			return;
		}

		if(msg.size() < startBinHeader+48+msgStr.dataMessageSize)
		{
			//the message's size is shorter than the announced size
			error();
			return;
		}

		MSNP2P *p2p=this;

		if(sessionID != 0)
		{
			if(m_p2pList.contains(sessionID) )
				p2p =m_p2pList[sessionID];
			else
			{
				if( sessionID == 0x40 ) //image messages
				{
					p2p=new MSNP2PIncoming(sessionID , this);
					m_p2pList.insert(sessionID , p2p);
				}
				else
				{
					error();
					return;
				}
			}
		}
		else
		{
			QString dataMessage=QCString((msg.data()+startBinHeader+48) , msgStr.dataMessageSize);
			rx=QRegExp("SessionID: ([0-9]*)\r\n");
			rx.search( dataMessage );
			sessionID=rx.cap(1).toUInt();
			if(sessionID != 0)
			{
				if(m_p2pList.contains(sessionID) )
					p2p =m_p2pList[sessionID];
				if(!p2p)
				{
					p2p=this;
				}
			}
			else
			{
				rx= QRegExp("Call-ID: \\{([0-9A-F\\-]*)\\}\r\n");
				rx.search( dataMessage );
				QString callID=rx.cap(1);

				if(!callID.isEmpty())
				{
					QMap<unsigned long int , MSNP2P* >::iterator it;
					for ( it = m_p2pList.begin(); it != m_p2pList.end(); ++it )
					{
						if(it.data()->m_CallID == callID)
						{
							p2p=it.data();
							break;
						}
					}
				}
			}
		}
		p2p->parseMessage( msgStr );
	}
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

void MSNP2PIncoming::parseMessage(MessageStruct &msgStr)
{
	MSNP2P::parseMessage(msgStr);

	if(m_Rfile)  //we are already downloading something to this file
	{			//m_file->file()->writeBlock( (msg.data()+startBinHeader+48) , dataMessageSize );
		m_Rfile->writeBlock( (msgStr.message.data()+48) , msgStr.dataMessageSize );

		if(m_kopeteTransfer)
			m_kopeteTransfer->slotProcessed( msgStr.dataOffset+msgStr.dataMessageSize );

		if(msgStr.dataOffset+msgStr.dataMessageSize >= msgStr.totalSize) //the file is complete
		{
			if(m_file)
			{
				m_file->close();
				m_parent->fileReceived(m_file , m_obj);
				m_file=0;
				m_Rfile=0L;
			}
			else
			{
				if(m_kopeteTransfer) m_kopeteTransfer->slotComplete();
				m_Rfile->close();
				delete m_Rfile;
				m_Rfile=0L;
			}
/*
			delete m_file;*/

				//send the bye message
			makeMSNSLPMessage(BYE, QString::null);

			m_parent->finished(this);
				//deleteLater();
		}
	}
	else if(msgStr.message.data()[48] == '\0' )
	{  //This can be only the data preparaion message.   prepare to download
		m_file=new KTempFile( locateLocal( "tmp", "msnpicture-" ), ".png" );
		m_file->setAutoDelete(true);
		m_Rfile=m_file->file();

	}
	else
	{
		QString dataMessage=QCString((msgStr.message.data()+48) , msgStr.dataMessageSize);
		kdDebug(14141) << k_funcinfo <<" dataMessage: "  << dataMessage << endl;

		if (dataMessage.contains("INVITE") )
		{
			if(! m_kopeteTransfer )
			{
				error();
				return;
			}
			//Parse the message to get some info for replying
			QRegExp rx(";branch=\\{([0-9A-F\\-]*)\\}\r\n");
			rx.search( dataMessage );
			m_branch=rx.cap(1);

			rx=QRegExp("Call-ID: \\{([0-9A-F\\-]*)\\}\r\n");
			rx.search( dataMessage );
			m_CallID=rx.cap(1);

		    //dirrect connection is not yet implemented, use the connection via MSNP2P
			QString content="Bridge: TCPv1\r\n"
					"Listening: false\r\n"
					"Nonce: {00000000-0000-0000-0000-000000000000}\r\n\r\n";

			makeMSNSLPMessage(OK, content);
					
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
		}
		else if (dataMessage.contains("BYE"))
		{
			abortCurrentTransfer();
		}
		else
		{ //this seems to be _probably_ (just a guess) a utf-16 message.   we will download it completely.
			/*
			 * The message looks like this:
			 *
			MIME-Version: 1.0
			Content-Type: image/gif
			\0
			base64:[ENCODED-IMAGE]
			 *  Sometimes, the base64 ends with =  sometimes it does not.
			 */
	
			if(msgStr.dataOffset ==0)
				fullContentMessage=QString::null;
					
				
			/*
			 * The following line doesn't works, because, wihtout reason i understand, the string contains some \0
			 * (\0\0 in utf-16)  between the   Content-Type:   and the Base64:
				
			QTextCodec *codec = QTextCodec::codecForName("ISO-10646-UCS-2");
			if(!codec)
			return; //abort();
			fullContentMessage += codec->toUnicode(msg.data()+startBinHeader+48-1 , dataMessageSize)
				
				
			 * Quick hack to parse utf-16 and remove \0\0 :
			 * The message shouldn't contains non ASCII char  (it's base64)  so i think i could do like that.
			 * FIXME:  yes, this is not 100% correct
			 */
			for(unsigned int f= 48 ; f < 48 + msgStr.dataMessageSize ; f+=2)
			{
				if(msgStr.message[f] != 0)
				{
					fullContentMessage+=QChar( msgStr.message[f] );
				}
			}
			
			//the message may be splitted
			if(msgStr.dataOffset+msgStr.dataMessageSize >= msgStr.totalSize)
			{ //whe have the whole
				
				kdDebug(14141) << k_funcinfo <<"Analyse the image message: " << fullContentMessage <<  endl;
				
				QString ext;
				QRegExp rx("Content-Type: ([a-zA-Z0-9/]*)");
				if( rx.search( fullContentMessage ) != -1 )
				{
					QString contentType=rx.cap(1);
					if(contentType=="image/gif")
						ext=".gif";
					else if(contentType=="image/png")
						ext=".png";
					else
					{
						kdWarning(14140) << k_funcinfo << contentType << " is not recognized.  A MSN message is not displayed" <<  endl;
						return;
					}
				}
				else
					return;
					
				rx=QRegExp("base64:([a-zA-Z0-9+\\-.*/!]*)");
				if( rx.search( fullContentMessage ) != -1 )
				{
					QString base64=rx.cap(1);
					
					QByteArray image;
					KCodecs::base64Decode( base64.utf8() , image );
						
					KTempFile *imageFile=new KTempFile( locateLocal( "tmp", "msntypewrite-" ), ext );
					imageFile->setAutoDelete(true);
					imageFile->file()->writeBlock( image.data() , image.size() );
					imageFile->file()->close();

					m_parent->fileReceived( imageFile , "typewrite" );
				}
			}
		}
	}
}

void MSNP2POutgoing::parseMessage(MessageStruct &msgStr)
{
	MSNP2P::parseMessage(msgStr);

	QString dataMessage=QCString((msgStr.message.data()+48) , msgStr.dataMessageSize);
	kdDebug(14141) << k_funcinfo <<" dataMessage: "  << dataMessage << endl;

	if (dataMessage.contains("BYE"))
	{
		m_parent->finished(this);
	}

}


void MSNP2PDisplatcher::parseMessage( MessageStruct & msgStr)
{
	MSNP2P::parseMessage(msgStr);

	QString dataMessage=QCString((msgStr.message.data()+48) , msgStr.dataMessageSize);

	if (dataMessage.contains("INVITE"))
	{
		kdDebug(14141) << k_funcinfo <<" dataMessage: "  << dataMessage << endl;
		
		//Parse the message to get some info for replying
		QRegExp rx(";branch=\\{([0-9A-F\\-]*)\\}\r\n");
		rx.search( dataMessage );
		m_branch=rx.cap(1);

		rx=QRegExp("Call-ID: \\{([0-9A-F\\-]*)\\}\r\n");
		rx.search( dataMessage );
		m_CallID=rx.cap(1);

		rx=QRegExp("SessionID: ([0-9]*)\r\n");
		rx.search( dataMessage );
		m_sessionId=rx.cap(1).toUInt();
	
		rx=QRegExp("AppID: ([0-9]*)\r\n");
		rx.search( dataMessage );
		unsigned long int AppID=rx.cap(1).toUInt();
			
		if(AppID==1) //the peer ask for a msn picture, or emoticon download.
		{       //  currently, we always send the display picture

			MSNP2POutgoing *p2p=new MSNP2POutgoing( m_sessionId , this);
			m_p2pList.insert(m_sessionId , p2p);
			p2p->m_msgHandle=m_msgHandle;
			p2p->m_myHandle=m_myHandle;
			p2p->m_CallID=m_CallID;
			p2p->m_branch=m_branch;
			p2p->m_msgIdentifier=m_msgIdentifier;
			m_msgIdentifier=0;

			//Send the OK message.
			QString content="SessionID: " + QString::number( m_sessionId ) + "\r\n\r\n" ;
			p2p->makeMSNSLPMessage( OK, content );
	
			//prepare to send the file
			p2p->m_Sfile = new QFile( locateLocal( "appdata", "msnpicture-" +
						m_myHandle.lower().replace(QRegExp("[./~]"),"-")  +".png" ) );
			if(!p2p->m_Sfile->open(IO_ReadOnly))  {/* TODO: error?*/ }

			p2p->m_footer='\1' ;
						
			//send the data preparation message
			QByteArray initM(4);
			initM.fill('\0');
			p2p->sendP2PMessage(initM);
			
			p2p->m_totalDataSize=  p2p->m_Sfile->size();
			p2p->m_offset=0;
	
			QTimer::singleShot( 10, p2p, SLOT(slotSendData()) ); //Go for upload
		}
		else if(AppID==2) //the peer want to transfer a file.
		{
			//extract the context from the invitation contents
			rx=QRegExp("Context: ([0-9a-zA-Z+/=]*)");
			rx.search( dataMessage );
			QString context=rx.cap(1);

			//Context is a base64 encoded dump of the internal memory of MSN messanger.
			// the filesize is contained in the bytes 8..11
			// the filename is from the byte 19
			// I don't know what other fields are.
	
			QByteArray binaryContext;
			KCodecs::base64Decode( context.utf8() , binaryContext );
			if(binaryContext.size() < 21 )   //security,  (don't crash)
			{
				error();
				return; 
			}
	
						
			//the filename is conteined in the context from the 19st char to the end.  (in utf-16)
			QTextCodec *codec = QTextCodec::codecForName("ISO-10646-UCS-2");
			if(!codec)
				return; //abort();
			QString filename = codec->toUnicode(binaryContext.data()+19 , binaryContext.size()-19-16) ;
			filename=filename.left(filename.find(QChar('\0')));
	
			//the size is placed in the context in the bytes 8..12  (source: the amsn code)
			unsigned long int filesize= (unsigned char)(binaryContext[8]) + (unsigned char)(binaryContext[9]) *256 + (unsigned char)(binaryContext[10]) *65536 + (unsigned char)(binaryContext[11]) *16777216 ;

			MSNP2PIncoming	*p2p=new MSNP2PIncoming( m_sessionId , this );
			p2p->m_CallID=m_CallID;
			p2p->m_branch=m_branch;
			p2p->m_msgHandle=m_msgHandle;
			p2p->m_myHandle=m_myHandle;
			p2p->m_msgIdentifier=m_msgIdentifier;
			m_msgIdentifier=0;
			m_p2pList.insert(m_sessionId ,p2p);
	
			//ugly hack to get the Kopete::Contact.
			Kopete::Contact *c=0L;
			if(parent())
			{
				Kopete::ChatSession *kmm=dynamic_cast<Kopete::ChatSession*>(parent()->parent());
				if(kmm)
					c=kmm->account()->contacts()[m_msgHandle];
			}
			if(!c)
			{
				// while the contact ptr shouldn't be needed, kopete crash if one pass a null contact.
				//  cf  Bug 89818
				kdWarning(14140) << " impossible to get the contact for initiating file transfer " << endl;
				error();
				return;
			}
			disconnect(Kopete::TransferManager::transferManager(), 0L , this, 0L);
			connect(Kopete::TransferManager::transferManager() , SIGNAL(accepted(Kopete::Transfer*, const QString& )) ,
					this, SLOT(slotTransferAccepted(Kopete::Transfer*, const QString& )));
			connect(Kopete::TransferManager::transferManager() , SIGNAL(refused( const Kopete::FileTransferInfo & ) ),
					this, SLOT( slotFileTransferRefused( const Kopete::FileTransferInfo & ) ) );
	
			//show a dialog to ask the transfer.
			Kopete::TransferManager::transferManager()->askIncomingTransfer(c  , filename , filesize, QString::null, QString::number(m_sessionId));
	
		}
		else  //unknwon AppID
		{
			error();
		}
	}
	else
		error();
}



void MSNP2PDisplatcher::requestDisplayPicture( const QString &myHandle, const QString &msgHandle, QString msnObject)
{
	unsigned long int sessID=rand()%0xFFFFFF00+4;

	MSNP2PIncoming *p2p=new MSNP2PIncoming(sessID, this);
	m_p2pList.insert(sessID, p2p);
	p2p->m_obj=msnObject;
	p2p->m_myHandle=myHandle;
	p2p->m_msgHandle=msgHandle;

	kdDebug(14141) << k_funcinfo << msnObject << endl;


	p2p->m_branch=randomid();
	p2p->m_CallID=randomid();

	msnObject=QString::fromUtf8(KCodecs::base64Encode( msnObject.utf8() ));
	msnObject.replace("=" , QString::null ) ;


	QString content="EUF-GUID: {A4268EEC-FEC5-49E5-95C3-F126696BDBF6}\r\n"
			"SessionID: "+ QString::number(sessID)+"\r\n"
			"AppID: 1\r\n"
			"Context: "  + msnObject +"\r\n\r\n";

	p2p->makeMSNSLPMessage( INVITE , content );
}



void MSNP2POutgoing::slotSendData()
{
	char ptr[1200];
	char *data;
	int bytesRead =0;
	if(m_Sfile)
	{
		bytesRead=m_Sfile->readBlock( ptr,1200 );
		data=ptr;
	}
	else if(m_imageToSend.size()>0)
	{
		data=m_imageToSend.data()+m_offset;
		bytesRead=QMIN(1200, m_imageToSend.size()-m_offset);
	}
	else return;

	QByteArray dataBA(bytesRead);
	for (  int f = 0; f < bytesRead; f++ )
		dataBA[f] = data[f];

//	kdDebug(14140) << "MSNP2PDisplatcher::slotSendData: offset="  << m_offset << "  size=" << bytesRead << "   totalSize=" << m_totalDataSize << "     sent=" << m_offset+bytesRead <<   endl;

	sendP2PMessage(dataBA);

	if( m_totalDataSize == 0  ) //this has been reseted bacause the file is completely send
	{
//		kd(14140) << "MSNP2PDisplatcher::slotSendData: FINISHED! wait for the BYE message" <<   endl;
		delete m_Sfile;
		m_Sfile=0L;
		m_imageToSend=QByteArray();
		m_footer='\0';
	}
	else
		QTimer::singleShot( 10, this, SLOT(slotSendData()) );
}



void MSNP2PDisplatcher::slotTransferAccepted(Kopete::Transfer* transfer, const QString& /*filename*/ )
{
	unsigned long int sid=transfer->info().internalId().toUInt();
	MSNP2PIncoming *p2p=0L;
	if(m_p2pList.contains(sid))
		p2p=dynamic_cast<MSNP2PIncoming *>(m_p2pList[sid]);

	if(p2p)
	{
		QObject::connect(transfer , SIGNAL(transferCanceled()), p2p, SLOT(abortCurrentTransfer()));
		QObject::connect(transfer,  SIGNAL(destroyed()) , p2p , SLOT(slotKopeteTransferDestroyed()));

		QString content="SessionID: " + QString::number( sid ) +"\r\n\r\n";
		
		p2p->makeMSNSLPMessage( OK, content);
		p2p->m_kopeteTransfer=transfer;
	}
}

void MSNP2PDisplatcher::slotFileTransferRefused( const Kopete::FileTransferInfo &info )
{
	unsigned long int sid=info.internalId().toUInt();
			MSNP2PIncoming *p2p=0L;
	if(m_p2pList.contains(sid))
		p2p=dynamic_cast<MSNP2PIncoming *>(m_p2pList[sid]);

	if(p2p)
	{
		QString content="SessionID: " + QString::number( sid ) +"\r\n\r\n";
		
		p2p->makeMSNSLPMessage( DECLINE , content );
		delete p2p;
		m_p2pList.remove(sid);
	}
}



void MSNP2PDisplatcher::sendImage(const QString& fileName)
{
	kdDebug(14140) << k_funcinfo << fileName <<endl;
	
	QFile pictFile( fileName );
	if(!pictFile.open(IO_ReadOnly))
	{
		kdWarning(14140) << k_funcinfo << "impossible to open " <<fileName <<endl;
		return;
	}
	
	QByteArray ar=KCodecs::base64Encode(pictFile.readAll());
	
	QString header="MIME-Version: 1.0\r\n"
					"Content-Type: image/gif\r\n"
					"\r\n\1"  //\1 will be replaced by \0 later
					"base64:";

	int s=(header.length()+ar.size()) *2;

	QByteArray toSend(s);
	toSend.fill(0);
	for(unsigned int f=0;f<header.length();f++)
	{
		if(header[f] != '\1')
			toSend[2*f]=header[f].latin1();
	}
	for(unsigned int f=0;f<ar.size();f++)
	{
		toSend[(header.length()+f)*2]=ar[f];
	}

	unsigned long int sid=0x40;

	MSNP2POutgoing *p2p=new MSNP2POutgoing( sid , this);
	m_p2pList.insert(sid , p2p);
	p2p->m_msgHandle=m_msgHandle;
	p2p->m_myHandle=m_myHandle;
	p2p->m_branch=randomid();
	p2p->m_CallID=randomid();

	//Send the OK message.
	QString content="SessionID: " + QString::number( sid ) + "\r\n\r\n" ;
	p2p->makeMSNSLPMessage( OK, content );

	p2p->m_footer='\3' ;
	
	p2p->m_imageToSend=toSend;
	p2p->m_offset=0;
	p2p->m_totalDataSize= toSend.size();
	QTimer::singleShot( 10, p2p, SLOT(slotSendData()) ); //Go for upload
}


void MSNP2PDisplatcher::finished( MSNP2P *f)
{
	if(f!=this)
	{
		m_p2pList.remove(f->m_sessionId);
		f->deleteLater();
	}
}


//-----------------------------

MSNP2POutgoing::MSNP2POutgoing( unsigned long int sessionID , MSNP2PDisplatcher *parent )
	: MSNP2P(sessionID , parent)
{
	m_Sfile=0L;
}

MSNP2POutgoing::~MSNP2POutgoing()
{
	delete m_Sfile;
}


MSNP2PIncoming::MSNP2PIncoming( unsigned long int sessionID , MSNP2PDisplatcher *parent )
	: MSNP2P(sessionID , parent)
{
	m_file=0l;
	m_Rfile=0L;
	m_kopeteTransfer=0L;
}

MSNP2PIncoming::~MSNP2PIncoming()
{
	if(m_kopeteTransfer)
	{
		m_kopeteTransfer->slotError( KIO::ERR_INTERNAL , i18n("Connection closed")  );
	}
	
	if(m_file)
		delete m_file;
	else
		delete m_Rfile;
}



void MSNP2PIncoming::abortCurrentTransfer()
{
	if(m_kopeteTransfer)
	{
		delete m_Rfile;
		m_Rfile=0L;
		
		//this need to be reseted before sending the BYE message.
		m_totalDataSize=0;
		m_offset=0;
		m_footer='\0';

		//FIXME: i'm not sure it's like that i should abort the transfer.
		makeMSNSLPMessage(BYE, "\r\n\r\n" );
	}
	m_parent->finished(this);
}


void MSNP2PIncoming::slotKopeteTransferDestroyed()
{
	m_kopeteTransfer=0L;
	kdDebug(14140) << k_funcinfo << endl;
}

void MSNP2PIncoming::error()
{
	MSNP2P::error();

	if(m_kopeteTransfer)
	{
		m_kopeteTransfer->slotError( KIO::ERR_INTERNAL , i18n("Malformed packet received")  );
		m_kopeteTransfer=0L;
	}
}



#include "msnp2p.moc"

