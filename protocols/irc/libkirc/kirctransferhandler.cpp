/*
    kirctransferhandler.cpp - DCC Handler

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kglobal.h>
#include <klocale.h>
#include <kextsock.h>

#include <qfile.h>
#include <qregexp.h>
#include <qtextcodec.h>

#include "kirctransferserver.h"

#include "kirctransferhandler.h"

using namespace KIRC;

KIRCTransferHandler *KIRCTransferHandler::self()
{
	static KIRCTransferHandler sm_self;
	return &sm_self;
}

KIRCTransferServer *KIRCTransferHandler::server()
{
	if( m_server )
//		server( m_default_server_port, m_default_server_backlog );
		server( 0, 1 );
	return m_server;
}

KIRCTransferServer *KIRCTransferHandler::server( Q_UINT16 port, int backlog )
{
//	if( m_server )
//		m_server->terminate();
	KIRCTransferServer *m_server = new KIRCTransferServer( port, backlog, this );

	// here connect the slots of the server

	return m_server;
}

KIRCTransferServer *KIRCTransferHandler::createServer(Engine *engine, QString m_userName,
		KIRCTransfer::Type type,
		QString fileName, Q_UINT32 fileSize)
{
	KIRCTransferServer *server = new KIRCTransferServer(engine, m_userName, type, fileName, fileSize, this);
	transferServerCreated(server);
	return server;
}

KIRCTransfer *KIRCTransferHandler::createClient(
	Engine *engine, QString nick,// QString nick_peer_adress,
	QHostAddress peer_address, Q_UINT16 peer_port,
	KIRCTransfer::Type type,
	QString fileName, Q_UINT32 fileSize )
{
	KIRCTransfer *client = new KIRCTransfer(
		engine, nick,// QString nick_peer_adress,
		peer_address, peer_port,
		type,
		fileName, fileSize,
		this );
	transferCreated(client);
	return client;
}

/*
File *DCCHandler::openFile( QString file, int mode = IO_ReadWrite )
{
	QFile *file = new QFile(filename);
	if (!file->open(mode))
	{
		delete file;
		file = 0L;
	}
	return file;
}
*/

#include "kirctransferhandler.moc"

// vim: set noet ts=4 sts=4 sw=4:
