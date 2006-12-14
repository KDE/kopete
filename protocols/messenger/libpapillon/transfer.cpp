/*
   transfer.cpp - Represent a transfer between the Messenger server.

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
#include "Papillon/Transfer"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QByteArray>

namespace Papillon {

class Transfer::Private
{
public:
	Transfer::TransferType type;
	QString command;
	QString transactionId;
	QStringList arguments;
	QByteArray payloadData;
};

Transfer::Transfer(const TransferType &type)
 : d(new Private)
{
	d->type = type;
}

Transfer::~Transfer()
{
	delete d;
}

Transfer::TransferType Transfer::type() const
{
	return d->type;
}

QString Transfer::command() const
{
	return d->command;
}

void Transfer::setCommand(const QString &command)
{
	d->command = command;
}

QString Transfer::transactionId() const
{
	Q_ASSERT(type() & Transfer::TransactionTransfer);

	return d->transactionId;
}

void Transfer::setTransactionId(const QString &transactionId)
{
	Q_ASSERT(type() & Transfer::TransactionTransfer);
	
	d->transactionId = transactionId;
}

QStringList Transfer::arguments() const
{
	return d->arguments;
}

void Transfer::setArguments(const QStringList &arguments)
{
	d->arguments = arguments;
}

void Transfer::setArguments(const QString &argumentString)
{
	d->arguments = argumentString.split(" ");
}

int Transfer::payloadLength() const
{
	Q_ASSERT(type() & Transfer::PayloadTransfer);

	return d->payloadData.size();
}

QByteArray Transfer::payloadData() const
{
	Q_ASSERT(type() & Transfer::PayloadTransfer);

	return d->payloadData;
}

void Transfer::setPayloadData(const QByteArray &data)
{
	Q_ASSERT(type() & Transfer::PayloadTransfer);

	d->payloadData = data;
}

QString Transfer::toString() const
{
	QString result;
	
	result += command();
	
	if( type() & Transfer::TransactionTransfer && !transactionId().isEmpty() )
	{
		result += ' ' + transactionId();
	}

	if( !d->arguments.empty() )
	{
		result += ' ' + arguments().join(" ");
	}

	if( type() & Transfer::PayloadTransfer )
	{
		result += ' ' + QString::number(payloadLength());
	}
	
	result += "\r\n";

	return result;
}

QByteArray Transfer::toRawCommand() const
{
	QByteArray result;
	
	result += toString().toUtf8();

	if( !d->payloadData.isEmpty() )
		result += d->payloadData;

	return result;
}

}
