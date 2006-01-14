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
#include <qfile.h>
#include <qhostaddress.h>
#include <qobject.h>
#include <qtextstream.h>

class KExtendedSocket;

class QFile;
class QTextCodec;

namespace KIRC
{
class Engine;

class Transfer
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
		Error_NoSocket	= -2,
		Error		= -1,
		Idle		= 0,
		HostLookup,
		Connecting,
		Connected,
		Closed
	};
public:
	Transfer(	KIRC::Engine *engine, QString nick,// QString nick_peer_adress
			Type type = Unknown,
			QObject *parent = 0L, const char *name = 0L );

	Transfer(	KIRC::Engine *engine, QString nick,// QString nick_peer_adress,
			QHostAddress peer_address, Q_UINT16 peer_port,
			Transfer::Type type,
			QObject *parent = 0L, const char *name = 0L );

	Transfer(	KIRC::Engine *engine, QString nick,// QString nick_peer_adress,
			Transfer::Type type,
			QString fileName, Q_UINT32 fileSize,
			QObject *parent = 0L, const char *name = 0L );

	Transfer(	KIRC::Engine *engine, QString nick,// QString nick_peer_adress,
			QHostAddress peer_address, Q_UINT16 peer_port,
			Transfer::Type type,
			QString fileName, Q_UINT32 fileSize,
			QObject *parent = 0L, const char *name = 0L );
/*
	For a file transfer properties are:

		KExntendedSocket	*socket
	or
		QHostAddress		peerAddress
		Q_UINT16		peerPort
	for determining the socket.

		QString			fileName
		Q_UINT32		fileSize
	for detemining the file propeties.
*//*
	Transfer(	KIRC *engine, QString nick,// QString nick_peer_adress,
			Transfer::Type type, QVariant properties,
			QObject *parent = 0L, const char *name = 0L );
*/
	~Transfer();

	KIRC::Engine *engine() const
		{ return m_engine; }
	QString nick() const
		{ return m_nick; }
	Type type() const
		{ return m_type; }
	Status status() const;

	/* Start the transfer.
	 * If not connected connect to client.
	 * Allow receiving/emitting data.
	 */
	bool initiate();

	QString fileName() const
		{ return m_fileName; }
	/* Change the file name.
	 */
	void setFileName(QString fileName)
		{ m_fileName = fileName; }
	unsigned long fileSize() const
		{ return m_fileSize; }
public slots:
	bool setSocket( KExtendedSocket *socket );
	void closeSocket();

	void setCodec( QTextCodec *codec );
	void writeLine( const QString &msg );

	void flush();

	void userAbort(QString);

signals:
	void readLine( const QString &msg );

	void fileSizeCurrent( unsigned int );
	void fileSizeAcknowledge( unsigned int );

//	void received(Q_UINT32);
//	void sent(Q_UINT32);

	void abort(QString);

	/* Emited when the transfer is complete.
	 * Usually it means that the file transfer has successfully finished.
	 */
	void complete();

protected slots:
	void slotError(int);

	void readyReadLine();

	void readyReadFileIncoming();

	void writeFileOutgoing();
	void readyReadFileOutgoing();

protected:
//	void emitSignals();
	void checkFileTransferEnd( Q_UINT32 fileSizeAck );

	KIRC::Engine *	m_engine;
	QString		m_nick;

	Type		m_type;
	KExtendedSocket *m_socket;
	bool		m_initiated;

	// Text member data
	QTextStream	m_socket_textStream;
//	QTextCodec *	m_socket_codec;

	// File member data
	QFile		m_file;
	QString		m_fileName;
	Q_UINT32	m_fileSize;
	Q_UINT32 /*usize_t*/	m_fileSizeCur;
	Q_UINT32 /*usize_t*/	m_fileSizeAck;
	QDataStream	m_socketDataStream;
	char		m_buffer[1024];
	int		m_bufferLength;

	// Data transfer measures
	Q_UINT32	m_receivedBytes;
	Q_UINT32	m_receivedBytesLimit;

	Q_UINT32	m_sentBytes;
	Q_UINT32	m_sentBytesLimit;
};

}

#endif
