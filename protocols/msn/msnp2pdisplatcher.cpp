/*
    msn p2p protocol

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
				{
					c=kmm->account()->contacts()[m_msgHandle];
					kmm->setCanBeDeleted( false );
				}
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
	/* For example, if the context of a filetransfer invitation is split, it's impossible to determine the sessid
	 * but we don't care, only the first part of the context interest us.
	else
		error();
	 */
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

#include "msnp2pdisplatcher.moc"
