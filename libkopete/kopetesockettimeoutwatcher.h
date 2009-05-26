/*
	kopetesockettimeoutwatcher.h - Kopete Socket Timeout Watcher

	Copyright (c) 2009 Roman Jarosz <kedgedev@centrum.cz>
	Kopete (c) 2009 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/
#ifndef KOPETESOCKETTIMEOUTWATCHER_H
#define KOPETESOCKETTIMEOUTWATCHER_H

#include "kopete_export.h"

#include <QObject>
#include <QAbstractSocket>

class QTimer;

namespace Kopete
{
/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/
class KOPETE_EXPORT SocketTimeoutWatcher : public QObject
{
Q_OBJECT
public:
	/**
	 * Returns new SocketTimeoutWatcher or 0 if SocketTimeoutWatcher was already created for this socket
	 *
	 * SocketTimeoutWatcher watches if outgoing data has been sent out.
	 * The implementation check if ACK was received after data has been sent
	 * and emits error(QAbstractSocket::SocketError) if ACK hasn't been
	 * received before @p msecTimeout milliseconds.
	 * @note abort is called on socket when timeout is reached.
	 */
	static SocketTimeoutWatcher* watch( QAbstractSocket* socket, quint32 msecTimeout = 15000 );

	~SocketTimeoutWatcher();

signals:
	/**
	 * Emitted when timeout is reached.
	 * @note socketError is always QAbstractSocket::RemoteHostClosedError
	 */
	void error( QAbstractSocket::SocketError socketError );
	void errorInt( int socketError );

private slots:
	void bytesWritten();
	void ackTimeoutCheck();

private:
	SocketTimeoutWatcher( QAbstractSocket* socket, quint32 msecTimeout );

	static QSet<QAbstractSocket*> watchedSocketSet;
	QAbstractSocket* mSocket;
	QTimer* mAckCheckTimer;
	bool mActive;
	quint32 mTimeoutThreshold;
};

}

#endif
