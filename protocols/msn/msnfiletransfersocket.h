/***************************************************************************
                          msnfiletransfersocket.h  -  description
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

#ifndef MSNFILETRANSFERSOCKET_H
#define MSNFILETRANSFERSOCKET_H

#include <qwidget.h>

#include "msnsocket.h"

class QFile;

class KExtendedSocket;

class KopeteTransfer;
class MSNProtocol;

/**
 * @author Olivier Goffart
 */
class MSNFileTransferSocket : public MSNSocket
{
	Q_OBJECT

public:
	MSNFileTransferSocket( bool incoming, QObject* parent = 0L );
	~MSNFileTransferSocket();

	void setKopeteTransfer( KopeteTransfer *kt );
	KopeteTransfer* kopeteTransfer() { return m_kopeteTransfer; }
	void setFileName( const QString &fn );
	void setAuthCookie( const QString &c ) { m_authcook = c; }
	void setCookie( long unsigned int c ) { m_cookie = c; }
	long unsigned int cookie() { return m_cookie; }
	bool incoming() { return m_incoming; }
	QString fileName() { return m_fileName;}
	long unsigned int size() { return m_size;}
	void listen( int port );

public slots:
	void abort();

signals:
	void done( MSNFileTransferSocket * );

protected:
	/**
	 * This reimplementation sets up the negotiating with the server and
	 * suppresses the change of the status to online until the handshake
	 * is complete.
	 */
	virtual void doneConnect();

	/**
	 * Handle an MSN command response line.
	 */
	virtual void parseCommand(const QString & cmd, uint id, const QString & data);
	virtual void bytesReceived(const QByteArray & data);

protected slots:
	virtual void slotReadyWrite();

private slots:
	void slotSocketClosed();
	void slotReadBlock(const QByteArray &);
	void slotAcceptConnection();
	void slotTimer();
	void slotSendFile();

private:
	MSNProtocol *m_protocol;

	long unsigned int m_cookie;
	long unsigned int m_size;
	long unsigned int m_downsize;
	QString m_authcook;
	QString m_fileName;
	bool m_incoming;
	KopeteTransfer* m_kopeteTransfer;
	QFile *m_file ;
	KExtendedSocket *m_server;

	bool ready;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

