/*
    datareceiver.cpp - Peer To Peer Session Notifier

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "sessionnotifier.h"

namespace PeerToPeer
{

SessionNotifier::SessionNotifier(QObject *parent) : QObject(parent), _type(0)
{
}

SessionNotifier::~SessionNotifier()
{
}

void SessionNotifier::setType(const Q_INT32 type)
{
	_type = type;
}

const Q_INT32 SessionNotifier::type()
{
	return _type;
}

void SessionNotifier::fireDataReceived(const QByteArray& data)
{
	emit dataReceived(data);
}

void SessionNotifier::fireEndOfData(const Q_INT32 identifier)
{
	emit endOfData(identifier);
}

void SessionNotifier::fireMessageAcknowledged(const Q_INT32 identifier)
{
	emit messageAcknowledged(identifier);
}

void SessionNotifier::fireMessageReceived(const QByteArray& message, const Q_INT32 identifier, const Q_INT32 relatesTo)
{
	emit messageReceived(message, identifier, relatesTo);
}

void SessionNotifier::fireTransactionTimedout(const Q_INT32 identifier, const Q_INT32 relatesTo)
{
	emit transactionTimedout(identifier, relatesTo);
}

}

#include "sessionnotifier.moc"
