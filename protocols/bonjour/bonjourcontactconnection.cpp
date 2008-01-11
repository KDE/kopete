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
#include <QHostAddress>
#include <QThread>
#include <QTime>
#include <QXmlStreamReader>

#include "kdebug.h"

#include "kopetemessage.h"
#include "kopetecontact.h"
#include "kopeteaccount.h"

#include "bonjourcontactconnection.h"

// Declare the tokenTable
BonjourContactConnection::TokenTable BonjourContactConnection::tokenTable;

BonjourContactConnection::TokenTable::TokenTable()
{
	insert("", BonjourXmlTokenNone);
	insert("stream:stream", BonjourXmlTokenStream);
	insert("message", BonjourXmlTokenMessage);
	insert("body", BonjourXmlTokenBody);
	insert("html", BonjourXmlTokenHtml);
	insert("x", BonjourXmlTokenX);
}

void BonjourContactConnection::setSocket(QTcpSocket *aSocket)
{
	socket = aSocket;

	socket->setParent(this);
	parser.setDevice(socket);

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
	//If We Cannot Connect within 3 seconds, that's an error :(
	if (! (socket->waitForConnected()))
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

const BonjourContactConnection::BonjourXmlToken BonjourContactConnection::getNextToken()
{
	BonjourXmlToken ret;

	if (parser.atEnd())
	{
		ret.type = QXmlStreamReader::Invalid;
		ret.name = BonjourXmlTokenError;
		return ret;
	}

	parser.readNext();

	ret.type = parser.tokenType();
	ret.qualifiedName = parser.qualifiedName();
	ret.name = tokenTable[ret.qualifiedName.toString()];
	ret.attributes = parser.attributes();
	ret.text = parser.text();

	kDebug()<<"Read Token: "<<ret.qualifiedName.toString();
	return ret;
}

const BonjourContactConnection::BonjourXmlToken BonjourContactConnection::getNextToken(BonjourXmlTokenName name)
{
	BonjourXmlToken token;

	// Scan Until We Get the Token
	do {
		token = getNextToken();
		if (token.name == name)
			break;
	} while (token.name != BonjourXmlTokenError);

	return token;
}

void BonjourContactConnection::dataInSocket()
{
	BonjourXmlToken token;

	kDebug()<<"Data Available!";
	
	switch (connectionState) {
		
		case BonjourConnectionConnected:
			token = getNextToken();
			kDebug()<<token.qualifiedName.toString();
			readData(token);
			break;

		case BonjourConnectionToWho:
			getWho();
			break;

		case BonjourConnectionNewIncoming:
		case BonjourConnectionNewOutgoing:
			token = getNextToken(BonjourXmlTokenStream);
			getStreamTag(token);
			break;
	}
}

void BonjourContactConnection::getStreamTag(const BonjourXmlToken &token)
{

	// If we haven't gotten the stream token yet, just ignore the packet
	if (token.name != BonjourXmlTokenStream)
		return;
	
	// If This was an Outgoing Stream, we do not need to check if the username is in the stream
	if (connectionState == BonjourConnectionNewOutgoing) {
		connectionState = BonjourConnectionConnected;
		return;
	}

	// From Now On, We are Guaranteed It it an Incoming Connection
	remote = token.attributes.value("from").toString();
	local = token.attributes.value("to").toString();
	kDebug()<<"Local: "<<local<<" Remote: "<<remote;

	if (local != "" && remote != "") {
		connectionState = BonjourConnectionConnected;
		emit discoveredUserName(this, remote);
	}
	else
		connectionState = BonjourConnectionToWho;

	sayStream();
}

void BonjourContactConnection::sayStream()
{
	kDebug()<<"Sending <stream>";

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

// We May Getting A Message or a </stream> here
void BonjourContactConnection::readData(BonjourXmlToken &token)
{
	QString type;

	switch (token.name) {

		case BonjourXmlTokenMessage:
			type = token.attributes.value("type").toString();
			if (type == "chat" || type == "")
				readMessage(token);
			break;

		case BonjourXmlTokenStream:
			connectionState = BonjourConnectionDisconnected;
			// Stream About to Close
			break;
	}
}

void BonjourContactConnection::readMessage(BonjourXmlToken &token)
{
	QString plaintext;
	QString HTMLVersion;
	bool inHtml = false;

	Kopete::Message message;

	// We Now Scan Each Token One by one
	do {
		token = getNextToken();

		switch (token.name) {

			case BonjourXmlTokenBody:
				if (inHtml)
					;			// FIXME: No HTML Stuff Implemented!!
				else
					plaintext = parser.readElementText();
				break;

			case BonjourXmlTokenHtml:
				if (token.type == QXmlStreamReader::StartElement)
					inHtml = true;
				else
					inHtml = false;
		}

	} while (token.name != BonjourXmlTokenError && token.name != BonjourXmlTokenMessage);
	// Exit When We Read The </message>, or out of packet data

	
	// If No Message, then return
	if (HTMLVersion == "" && plaintext == "")
		return;

	// We Are Now Guaranteed to have a message to show
	message = newMessage(Kopete::Message::Inbound);

	if (HTMLVersion != "")
		message.setHtmlBody(HTMLVersion);
	else
		message.setPlainBody(plaintext);

	emit messageReceived(message);
}

void BonjourContactConnection::sayGoodBye()
{
	if (connectionState == BonjourConnectionConnected)
		socket->write("</stream:stream>");
}
