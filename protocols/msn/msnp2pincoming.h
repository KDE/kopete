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

#ifndef MSNP2PINCOMING_H
#define MSNP2PINCOMING_H

#include "msnp2p.h"


class MSNP2PIncoming : public MSNP2P
{
	Q_OBJECT
public:
	MSNP2PIncoming( unsigned long int sessionID , MSNP2PDisplatcher *parent);
	~MSNP2PIncoming( );


	virtual void parseMessage(MessageStruct & );
	virtual void error();

	//for the display image
	KTempFile *m_file;
	QFile *m_Rfile;
	QString m_obj;

	QString fullContentMessage;  //used for typewrited images messages


	Kopete::Transfer *m_kopeteTransfer;

public slots:
	/**
	 * Abort the current transfer.
	 */
		void abortCurrentTransfer();
		void slotKopeteTransferDestroyed();

};

#endif
