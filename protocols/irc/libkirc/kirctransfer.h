/*
    kirctransfer.h - DCC Handler

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

#ifndef KIRCTRANSFER_H
#define KIRCTRANSFER_H

#include <qdatastream.h>
#include <qhostaddress.h>
#include <qobject.h>
#include <qtextstream.h>

class KIRC;

class KExtendedSocket;

class QFile;
class QTextCodec;

class KIRCTransfer
	: public QObject
{
	Q_OBJECT

public:
	enum Type {
		Unknown,
		Chat,
		FileOutgoing,
		FileIncoming
	};

	enum Status {
		Error_NoSocketSet	= -2,
		Error			= -1,
		Idle			= 0,
		HostLookup,
		Connecting,
		Connected,
		Closed
	};
public:
	KIRCTransfer(	KIRC *engine, QString nick,// QString nick_peer_adress
			Type type = Unknown,
			QObject *parent = 0L, const char *name = 0L );

	KIRCTransfer(	KIRC *engine, QString nick,// QString nick_peer_adress,
			QHostAddress peer_address, Q_UINT16 peer_port,
			KIRCTransfer::Type type,
			QObject *parent = 0L, const char *name = 0L );

	KIRCTransfer(	KIRC *engine, QString nick,// QString nick_peer_adress,
			QHostAddress peer_address, Q_UINT16 peer_port,
			KIRCTransfer::Type type,
			QFile *file, Q_UINT32 file_size,
			QObject *parent = 0L, const char *name = 0L );

	~KIRCTransfer();

	bool setSocket( KExtendedSocket *socket );

	Type getType()
	{
		return m_type;
	}

	Status status();
	void connectToPeer();

public slots:
	void setCodec( QTextCodec *codec );
	void writeLine( const QString &msg );

signals:
	void readLine( const QString &msg );

	void received( Q_UINT32 ); // percentage received if applicable
	void receivedBytes( Q_UINT32 );

	void sent( Q_UINT32 ); // percentage sent if applicable
	void sentBytes( Q_UINT32 );

protected:
	void initClient();
	void initServer();

	void emitSignals();

protected slots:
	void readyReadLine();

	void readyReadFileIncoming();

	void writeFileOutgoing();
	void readyReadFileOutgoing();

protected:
	Type			m_type;

	KExtendedSocket *	m_socket;

	// Text member data
	QTextStream		m_socket_textStream;
//	QTextCodec *		m_socket_codec;

	// File member data
	QFile *			m_file;
	Q_UINT32 /*usize_t*/	m_file_size_cur;
	Q_UINT32 /*usize_t*/	m_file_size_ack;
	QDataStream		m_socket_dataStream;
	char			m_buffer[1024];
	int			m_buffer_length;

	// Data transfer measures
	Q_UINT32		m_received_bytes_limit;
	Q_UINT32		m_received_bytes;

	Q_UINT32		m_sent_bytes_limit;
	Q_UINT32		m_sent_bytes;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

