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

#ifndef MSNP2POUTGOING_H
#define MSNP2POUTGOING_H

#include "msnp2p.h"

class MSNP2POutgoing : public MSNP2P
{
	Q_OBJECT
public:
	MSNP2POutgoing( unsigned long int sessionID , MSNP2PDisplatcher *parent);
	~MSNP2POutgoing( );

	virtual void parseMessage(MessageStruct & );

	QFile *m_Sfile;
	QByteArray m_imageToSend;


private slots:
	void slotSendData();
};



#endif
