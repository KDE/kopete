/*
   networkmessage.cpp - Represent a network message between the Messenger server.

   Copyright (c) 2006-2007 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#include "Papillon/NetworkMessage"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QByteArray>

namespace Papillon {

class NetworkMessage::Private
{
public:
	NetworkMessage::NetworkMessageType type;
	QString command;
	QString transactionId;
	QStringList arguments;
	QByteArray payloadData;
};

NetworkMessage::NetworkMessage(const NetworkMessageType &type)
 : d(new Private)
{
	d->type = type;
}

NetworkMessage::~NetworkMessage()
{
	delete d;
}

NetworkMessage::NetworkMessageType NetworkMessage::type() const
{
	return d->type;
}

QString NetworkMessage::command() const
{
	return d->command;
}

void NetworkMessage::setCommand(const QString &command)
{
	d->command = command;
}

QString NetworkMessage::transactionId() const
{
	Q_ASSERT(type() & NetworkMessage::TransactionMessage);

	return d->transactionId;
}

void NetworkMessage::setTransactionId(const QString &transactionId)
{
	Q_ASSERT(type() & NetworkMessage::TransactionMessage);
	
	d->transactionId = transactionId;
}

QStringList NetworkMessage::arguments() const
{
	return d->arguments;
}

void NetworkMessage::setArguments(const QStringList &arguments)
{
	d->arguments = arguments;
}

void NetworkMessage::setArguments(const QString &argumentString)
{
	d->arguments = argumentString.split(" ");
}

int NetworkMessage::payloadLength() const
{
	Q_ASSERT(type() & NetworkMessage::PayloadMessage);

	return d->payloadData.size();
}

QByteArray NetworkMessage::payloadData() const
{
	Q_ASSERT(type() & NetworkMessage::PayloadMessage);

	return d->payloadData;
}

void NetworkMessage::setPayloadData(const QByteArray &data)
{
	Q_ASSERT(type() & NetworkMessage::PayloadMessage);

	d->payloadData = data;
}

QString NetworkMessage::toString() const
{
	QString result;
	
	result += command();
	
	if( type() & NetworkMessage::TransactionMessage && !transactionId().isEmpty() )
	{
		result += ' ' + transactionId();
	}

	if( !d->arguments.empty() )
	{
		result += ' ' + arguments().join(" ");
	}

	if( type() & NetworkMessage::PayloadMessage )
	{
		result += ' ' + QString::number(payloadLength());
	}
	
	result += "\r\n";

	return result;
}

QByteArray NetworkMessage::toRawCommand() const
{
	QByteArray result;
	
	result += toString().toUtf8();

	if( !d->payloadData.isEmpty() )
		result += d->payloadData;

	return result;
}

}
