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
	else if(msgStr.message.data()[48] == '\0' && msgStr.dataMessageSize==4)
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
		else if(dataMessage.contains("200 OK"))
		{
			//ignore
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

#include "msnp2pincoming.moc"
