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
#include <QRegExp>
#include <QHostAddress>
#include <QThread>
#include <QTime>

#include "kdebug.h"

#include "kopetemessage.h"
#include "kopetecontact.h"
#include "kopeteaccount.h"

#include "bonjourcontactconnection.h"

void BonjourContactConnection::setSocket(QTcpSocket *aSocket)
{
	socket = aSocket;

	socket->setParent(this);

	connect(socket, SIGNAL(readyRead()), this, SLOT(dataInSocket()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
}

BonjourContactConnection::BonjourContactConnection(QTcpSocket *aSocket, 
		QObject *parent) : QObject(parent)
{
	setSocket(aSocket);
	
	connectionState = BonjourConnectionNewIncoming;
}

BonjourContactConnection::BonjourContactConnection(const QHostAddress &address, short int port,
		QString alocal, QString aremote, QObject *parent) : QObject(parent)
{
	QTcpSocket *aSocket = new QTcpSocket;
	aSocket->connectToHost(address, port);

	setSocket(aSocket);

	connectionState = BonjourConnectionNewOutgoing;

	local = alocal;
	remote = aremote;

	kDebug()<<"Starting to Wait for Connection";
	//If We Cannot Connect within 5 seconds, that's an error :(
	if (! (socket->waitForConnected(5000)))
	{
		connectionState = BonjourConnectionError;
		emit errorCouldNotConnect();
		return;
	}

	sayStream();
}

BonjourContactConnection::~BonjourContactConnection()
{
	if (socket) {

		// Remove Socket From All Connections, so that the disconnect doesn't cascade
		socket->disconnect();

		delete socket;
		socket = NULL;
	}
}

void BonjourContactConnection::dataInSocket()
{
	switch (connectionState) {
		
		case BonjourConnectionConnected:
			readData();
			break;

		case BonjourConnectionNewIncoming:
		case BonjourConnectionNewOutgoing:
			getStreamTag();
			break;
		
		case BonjourConnectionToWho:
			getWho();
			break;
	}
}

void BonjourContactConnection::getStreamTag()
{
	int size = socket->bytesAvailable();
	QByteArray data = socket->read(size);
	QRegExp re;

	// First check for a valid stream tag
	// Ignore all tags that are not stream here
	// Warning: This Packet (whatever it is, will be lost)
	re = QRegExp("stream:stream");
	if (re.indexIn(data) <= -1) {
		return;
	}

	// If This was an Outgoing Stream, we do not need to check if the username is in the stream
	if (connectionState == BonjourConnectionNewOutgoing) {
		connectionState = BonjourConnectionConnected;
		return;
	}

	// From Here on, we are guaranteed that this is an incoming connection

	// Check if from and to was encoded in stream
        re = QRegExp("<stream:stream.*\\sfrom=\"(.*)\".*\\sto=\"(.*)\".*>");
        if (re.indexIn(data) > -1) {
		connectionState = BonjourConnectionConnected;

                remote = re.cap(1);
                local = re.cap(2);

		emit discoveredUserName(this, remote);
	}
	else
		connectionState = BonjourConnectionToWho;

	sayStream();
}

void BonjourContactConnection::sayStream()
{
	QString response;
	QTextStream stream(&response);

	stream	<<"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		<<"<stream:stream xmlns=\"jabber:client\" "
		<<"xmlns:stream=\"http://etherx.jabber.org/streams\"";

	if (connectionState != BonjourConnectionToWho)
		stream<<" from=\""<<local<<"\" to=\""<<remote<<"\"";

	stream<<">";

	socket->write(response.toUtf8());

}

void BonjourContactConnection::getWho()
{
}

void BonjourContactConnection::socketDisconnected()
{
	connectionState = BonjourConnectionDisconnected;
	emit disconnected(this);
}

void BonjourContactConnection::sendMessage(const Kopete::Message &message)
{
	QString response;
	QTextStream stream(&response);

	stream	<<"<message to='"<<remote<<"' from='"<<local<<"' type='chat'>"
		<<"<body>"<<message.plainBody()<<"</body>"
		<<"<html xmlns='http://www.w3.org/1999/xhtml'>"
		<<"<body>"<<message.escapedBody()<<"</body>"
		<<"</html>"
		<<"<x xmlns='jabber:x:event'><composing/></x>"
		<<"</message>";

	kDebug()<<response;
	socket->write(response.toUtf8());
}

QHostAddress BonjourContactConnection::getHostAddress()
{
	return socket->peerAddress();
}

Kopete::Message BonjourContactConnection::newMessage(Kopete::Message::MessageDirection direction)
{
	// Our Parent is the remote contact
	Kopete::Contact *remote = (Kopete::Contact *) parent();

	// Get the Myself
	Kopete::Contact *myself = remote->account()->myself();

	Kopete::Message message;

	if (direction == Kopete::Message::Inbound)
		message = Kopete::Message(remote, myself);
	else
		message = Kopete::Message(myself, remote);

	message.setDirection(direction);

	return message;
}

void BonjourContactConnection::readData()
{
	int size = socket->bytesAvailable();
	QByteArray data = socket->read(size);

	kDebug()<<data;

	if (data.contains("message"))
		readMessage(data);
	else if (data.contains("</stream"))
		;				// Should Do Something
}

void BonjourContactConnection::readMessage(const QByteArray &data)
{
	// The Structure of a message is <message .... <body>plaintextver</body>....<body>HTMLVER</body>...</message>
	// or                            <message>.... <body>plaintextver</body>....<body /></message>
	// We Check in that Order
	
	QRegExp re;
	QString plaintext;
	QString HTMLVersion;

	Kopete::Message message;

	// First Get HTML Version
	re = QRegExp("<html.*><body>(.*)</body></html>");
	if (re.indexIn(data) > -1) {
		HTMLVersion = re.cap(1);

		if (HTMLVersion.size()) {
			message = newMessage(Kopete::Message::Inbound);
			message.setHtmlBody(HTMLVersion);

			emit messageReceived(message);
			return;
		}
	}


	// Then Get Plaintext Version
	re = QRegExp("<message.*<body>([^<]*)</body>");
	if (re.indexIn(data) > -1) {
		plaintext = re.cap(1);

		if (plaintext.size()) {
			message = newMessage(Kopete::Message::Inbound);
			message.setPlainBody(plaintext);

			emit messageReceived(message);
			return;
		}
	}
}

bool BonjourContactConnection::waitReady(int msecs)
{
	// FIXME: This Really Wastes Resources
	// Maybe I should Speed it up somehow?
	
	QTime stopWatch;
	stopWatch.start();

	while (connectionState != BonjourConnectionConnected && stopWatch.elapsed() < msecs)
		;
		//QThread::msleep(100);			// Take a 100ms nap
	return connectionState == BonjourConnectionConnected;		
}

void BonjourContactConnection::sayGoodBye()
{
	if (connectionState == BonjourConnectionConnected)
		socket->write("</stream:stream>");
}
