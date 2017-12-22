/*
    kirctransferhandler.cpp - DCC Handler

    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kirctransferserver.h"

#include <kglobal.h>
#include <klocale.h>
#include <kextsock.h>

#include <qfile.h>
#include <qregexp.h>
#include <qtextcodec.h>

using namespace KIRC;

TransferHandler *TransferHandler::self()
{
	static TransferHandler sm_self;
	return &sm_self;
}

TransferServer *TransferHandler::server()
{
	if( m_server )
//		server( m_default_server_port, m_default_server_backlog );
		server( 0, 1 );
	return m_server;
}

TransferServer *TransferHandler::server( quint16 port, int backlog )
{
//	if( m_server )
//		m_server->terminate();
	TransferServer *m_server = new TransferServer( port, backlog, this );

	// here connect the slots of the server

	return m_server;
}

TransferServer *TransferHandler::createServer(Engine *engine, QString m_userName,
		Transfer::Type type,
		QString fileName, quint32 fileSize)
{
	TransferServer *server = new TransferServer(engine, m_userName, type, fileName, fileSize, this);
	transferServerCreated(server);
	return server;
}

Transfer *TransferHandler::createClient(
	Engine *engine, QString nick,// QString nick_peer_adress,
	QHostAddress peer_address, quint16 peer_port,
	Transfer::Type type,
	QString fileName, quint32 fileSize )
{
	Transfer *client = new Transfer(
		engine, nick,// QString nick_peer_adress,
		peer_address, peer_port,
		type,
		fileName, fileSize,
		this );
	transferCreated(client);
	return client;
}

/*
File *DCCHandler::openFile( QString file, int mode = QIODevice::ReadWrite )
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

