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


#include "msnp2poutgoing.h"
#include "msnp2pdisplatcher.h"

// qt
#include <qregexp.h>
#include <qfile.h>
#include <qtextcodec.h>
#include <qtimer.h>

// kde
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>



MSNP2POutgoing::MSNP2POutgoing( unsigned long int sessionID , MSNP2PDisplatcher *parent )
	: MSNP2P(sessionID , parent)
{
	m_Sfile=0L;
}

MSNP2POutgoing::~MSNP2POutgoing()
{
	delete m_Sfile;
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


#include "msnp2poutgoing.moc"
