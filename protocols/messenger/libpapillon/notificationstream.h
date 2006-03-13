/*
   notificationstream.h - Represent a stream with the Notification server.

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONNOTIFICATIONSTREAM_H
#define PAPILLONNOTIFICATIONSTREAM_H

#include <stream.h>
#include <papillon_macros.h>

namespace Papillon 
{

class Connector;
class Transfer;

/**
 * Stream for the Notification server.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
*/
class PAPILLON_EXPORT NotificationStream : public Stream
{
	Q_OBJECT
public:
	NotificationStream(Connector *connector, QObject *parent = 0);
	~NotificationStream();

	void connectToServer(const QString &server, quint16 port);

	virtual void close();
	virtual int errorCondition() const;
 	virtual QString errorText() const;

	virtual bool transfersAvailable() const;

	virtual Transfer *read();
	virtual void write(Transfer *transfer);

	void reset(bool all);

signals:
	void connected();

private slots:
	void slotConnectorConnected();

	void slotProtocolOutgoingData(const QByteArray &data);
	void slotProtocolIncomingData();

	void slotByteStreamConnectionClosed();
	//void slotByteStreamDelayedClose();
	void slotByteStreamReadyRead();
	void slotByteStreamBytesWritten(int bytes);

private:
	class Private;
	Private *d;
};

}

#endif
