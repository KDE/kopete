/*
   papillonclientstream.h - Represent a stream with a Messenger service.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONCLIENTSTREAM_H
#define PAPILLONCLIENTSTREAM_H

#include <Papillon/Base/Stream>
#include <Papillon/Macros>

namespace Papillon
{

class Connector;
class NetworkMessage;

/**
 * @class ClientStream papillonclientstream.h <Papillon/ClientStream>
 * @brief A client stream to a Messenger service.
 *
 * @author Michaël Larouche <larouche@kde.org>
*/
class PAPILLON_EXPORT ClientStream : public Stream
{
	Q_OBJECT
public:
	explicit ClientStream(Connector *connector, QObject *parent = 0);
	~ClientStream();

	void connectToServer(const QString &server, quint16 port);

	virtual void close();
	virtual int errorCondition() const;
 	virtual QString errorText() const;

	virtual bool networkMessagesAvailable() const;

	virtual NetworkMessage *read();
	virtual void write(NetworkMessage *networkMessage);

	void reset(bool all);

signals:
	void connected();

private slots:
	void slotConnectorConnected();

	void slotProtocolOutgoingData(const QByteArray &data);
	void slotProtocolIncomingData();

	void slotByteStreamConnectionClosed();
	void slotByteStreamReadyRead();
	void slotByteStreamBytesWritten(int bytes);

private:
	class Private;
	Private *d;
};

}

#endif // PAPILLONCLIENTSTREAM_H
