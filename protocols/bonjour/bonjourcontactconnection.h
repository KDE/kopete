/*
    bonjourcontactconnection.h - Kopete Bonjour Protocol

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

#ifndef BONJOURCONTACTCONNECTION_H
#define BONJOURCONTACTCONNECTION_H

#include <QTcpSocket>
#include <QString>

enum BonjourConnectionState {
	BonjourConnectionNewOutgoing,		// New Outgoing Stream
	BonjourConnectionNewIncoming,		// New Incoming Stream

	BonjourConnectionOutgoingStream,	// Expect a <stream>
	BonjourConnectionToWho,			// We are Unsure who we connect to

	BonjourConnectionConnected,		// Connected

	BonjourConnectionError			// Reserved for Future Use
};

/* The BonjourContactConnection contains a QTcpSocket which has a connection to
 * some contact.
 *
 * Pass Two Strings for an outgoing connection (second constructor)
*/

class BonjourContactConnection : public QObject {

	Q_OBJECT

	// The State of the Connection...
	// Basically if we got a <stream> yet
	BonjourConnectionState connectionState;

	// The Actual Connection
	QTcpSocket *socket;

	// The local and remote names (as defined by them for incoming)
	QString local;
	QString remote;

    public:

	// Public Constructor for incoming connections
	BonjourContactConnection(QTcpSocket *aSocket, QObject *parent = NULL);

	// This is For OutGoing Connections
	BonjourContactConnection(QTcpSocket *aSocket, QString local, QString remote, QObject *parent = NULL);

	// Destructor
	~BonjourContactConnection();

	// Write <stream:stream> in socket
	void sayStream();

    signals:

	// This Signal is Emitted when there is new data
	void newData(BonjourContactConnection *);

	// This signal is emitted when we discover who we are connected to (unverified)
	// Incoming Connections Only, the username is sent back
	void discoveredUserName(BonjourContactConnection *, QString);

	// This Signal is basically forwarding disconnect signal from socket
	void disconnected(BonjourContactConnection *);

    public slots:
	
	// This slot is called by the socket, to signify new data
	void dataInSocket();

	// This is Called when the socket disconnects
	void socketDisconnected();

	// Send a message
	void say(QString message);

	// Disconnect
	void closeConnection();
};

#endif
