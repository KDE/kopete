/*
    bonjourcontactconnection.cpp - Kopete Bonjour Protocol

    Copyright (c) 2007      by Tejas Dinkar	<tejas@gja.in>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <QAbstractSocket>
#include <QTcpSocket>

#include "bonjourcontactconnection.h"

BonjourContactConnection::BonjourContactConnection(QTcpSocket *aSocket, 
		QObject *parent) : QObject(parent)
{
	socket = aSocket;

	connect(socket, SIGNAL(readyRead()), this, SLOT(dataInSocket()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

	connectionState = BonjourConnectionNewIncoming;
}

BonjourContactConnection::BonjourContactConnection(QTcpSocket *aSocket, 
		QString alocal, QString aremote, QObject *parent) : QObject(parent)
{
	BonjourContactConnection(aSocket, parent);

	connectionState = BonjourConnectionNewOutgoing;

	local = alocal;
	remote = aremote;
}

BonjourContactConnection::~BonjourContactConnection()
{
	closeConnection();
}

void BonjourContactConnection::dataInSocket()
{
	emit newData(this);
}

void BonjourContactConnection::socketDisconnected()
{
	emit disconnected(this);
}

void BonjourContactConnection::say(QString message)
{
}

void BonjourContactConnection::closeConnection()
{
	if (socket) {
		if (socket->state() == QAbstractSocket::ConnectedState) 
			socket->close();
		delete socket;
		socket = NULL;
	}
}
