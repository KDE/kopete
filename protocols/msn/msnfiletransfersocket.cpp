/***************************************************************************
                          msnfiletransfersocket.cpp  -  description
                             -------------------
    begin                : mer jui 31 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "msnfiletransfersocket.h"

#include "msnprotocol.h"
#include "kopete.h"
#include "kopetetransfermanager.h"
// kde
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
//qt
#include <qfile.h>




MSNFileTransferSocket::MSNFileTransferSocket(const QString msnid, const QString cook, const QString filename , QObject* parent) : MSNSocket(parent)
{
  m_msnId=msnid;
  m_authcook=cook;
  m_fileName=filename;

  m_kopeteTransfer=0l;

  QObject::connect( this, SIGNAL( socketClosed( int ) ), this, SLOT( slotSocketClosed( ) ) );

 	QObject::connect( this, SIGNAL( blockRead( const QByteArray & ) ),	this, SLOT(slotReadBlock( const QByteArray & ) ) );

  
}

MSNFileTransferSocket::~MSNFileTransferSocket(){
}
/** 
	 * Handle an MSN command response line.
	 */
void MSNFileTransferSocket::parseCommand(const QString & cmd, uint id, const QString & data)
{
	if( cmd == "VER" )
	{
		sendCommand( "USR" , m_msnId+ " " +m_authcook ,false);
	}
	else if( cmd == "FIL" )
	{
    m_size=id; //data.toUInt(); //BUG: the size is take as id bye MSNSocket because it is a number

    m_downsize=0;
    m_file=new QFile(m_fileName);

    if( m_file->open( IO_WriteOnly ))
        sendCommand( "TFR" ,NULL,false);
    else
    {
      kdDebug() << "Impossible d'écrire dans le fichier " <<endl;
      disconnect();
    }

	}
 	else if( cmd == "BYE" )
	{
    kdDebug() << "MSNFileTransferSocket::parseCommand : end of transfer " <<endl;
 	}

	else
	{
		kdDebug() << "MSNFileTransferSocket::parseCommand : COMMANDE INCONNUE"<< cmd << " " << id << " " << data << "' from server!" << endl;

	}
}

/** 
	 * This reimplementation sets up the negotiating with the server and
	 * suppresses the change of the status to online until the handshake
	 * is complete.
	 */
void MSNFileTransferSocket::doneConnect()
{
  sendCommand( "VER", "MSNFTP", false );
}
/** No descriptions */
void MSNFileTransferSocket::bytesReceived(const QByteArray & head)
{
   unsigned int sz=(int)((unsigned char)head.data()[2])*256+(int)((unsigned char)head.data()[1]);
   readBlock(sz);

}
/** No descriptions */
void MSNFileTransferSocket::slotSocketClosed()
{
		kdDebug() << "MSNFileTransferSocket::slotSocketClose "<<  endl;
    m_file->close();
    delete this;
}
/** No descriptions */
void MSNFileTransferSocket::slotReadBlock(const QByteArray &block)
{
   m_file->writeBlock( block.data(), block.size() );      // write to file
   m_downsize+=block.size();

  int percent=0;    
  if(m_size!=0)   percent=100*m_downsize/m_size;

   if(m_kopeteTransfer) m_kopeteTransfer->slotPercentCompleted(percent);

   kdDebug() << "MSNFileTransferSocket  -  " <<percent <<"% done"<<endl;

   if(m_downsize==m_size) sendCommand( "BYE" ,"16777989",false);

}
/** No descriptions */
void MSNFileTransferSocket::setKopeteTransfer(KopeteTransfer *kt)
{
  m_kopeteTransfer=kt;

}

#include "msnfiletransfersocket.moc"
