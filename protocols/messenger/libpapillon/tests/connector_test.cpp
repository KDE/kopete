/*
   connector_test.cpp - Test for the connector and stream.

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
#include "connector_test.h"

// Qt includes
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtDebug>

// Papillon includes
#include "Papillon/ClientStream"
#include "Papillon/QtConnector"
#include "Papillon/NetworkMessage"
#include "Papillon/Macros"

#define PASSPORT_ID "klj345sdas765d@passport.com"

using namespace Papillon;

class Connector_Test::Private
{
public:
	Private()
	 : stream(0), qtConnector(0), trId(0)
	{}
	ClientStream *stream;
	QtConnector *qtConnector;

	int trId;
};

Connector_Test::Connector_Test(QObject *parent)
 : QObject(parent), d(new Private)
{
	d->qtConnector = new QtConnector(this);
	d->stream = new ClientStream(d->qtConnector, this);
}

Connector_Test::~Connector_Test()
{
	delete d;
}

void Connector_Test::connectToServer()
{
	d->stream->connectToServer("messenger.hotmail.com", 1863);
	connect(d->stream, SIGNAL(connected()), this, SLOT(slotConnected()));
}

void Connector_Test::slotConnected()
{
	connect(d->stream, SIGNAL(readyRead()), this, SLOT(slotReadNetworkMessage()));
	
	doLoginProcess();
}

void Connector_Test::doLoginProcess()
{
	NetworkMessage *usrNetworkMessage = new NetworkMessage(NetworkMessage::TransactionMessage);
	usrNetworkMessage->setCommand( QLatin1String("VER") );
	usrNetworkMessage->setTransactionId( QString::number(++d->trId) );
	usrNetworkMessage->setArguments( QString("MSNP11 MSNP10 CVR0") );

	d->stream->write(usrNetworkMessage);
}

void Connector_Test::loginProcessCvr()
{
	NetworkMessage *cvrNetworkMessage = new NetworkMessage(NetworkMessage::TransactionMessage);
	cvrNetworkMessage->setCommand( QLatin1String("CVR") );
	cvrNetworkMessage->setTransactionId( QString::number(++d->trId) );
	cvrNetworkMessage->setArguments( QString("0x040c winnt 5.1 i386 MSNMSGR 7.0.0777 msmsgs "PASSPORT_ID) );

	d->stream->write(cvrNetworkMessage);
}

void Connector_Test::loginProcessTwnI()
{
	NetworkMessage *twnNetworkMessage = new NetworkMessage(NetworkMessage::TransactionMessage);
	twnNetworkMessage->setCommand("USR");
	twnNetworkMessage->setTransactionId( QString::number(++d->trId) );
	twnNetworkMessage->setArguments( QString("TWN I "PASSPORT_ID) );

	d->stream->write(twnNetworkMessage);
}

void Connector_Test::loginProcessTwnS()
{
	NetworkMessage *twnNetworkMessage = new NetworkMessage(NetworkMessage::TransactionMessage);
	twnNetworkMessage->setCommand("USR");
	twnNetworkMessage->setTransactionId( QString::number(++d->trId) );
	QString args = QString("TWN S t=7tLSuqR92Bo*17x76PDg87IVMA5FKqxccJDUocFzfCXbipUMZuMv4HatazUwTBVqsFkTxkS0qFCSbzA9SUFjM*SHzGKIIC7kgZAEikfzfAUufs*L!3B5i0aSrNo03BAeQP&p=7nhP1TIX4BGQ*k4JKZw0JHP5rb3A9wk8fw!ZYadtXN0OFiN*yZr6UaFwBAdUOKwoQkYfK1gEzWE*Op16WDcE*9J3Hv4JWG1TF3eSoAq71CITPmkZONAReXYlGz5Rk5l7ZFwbAPq6NxxqxzK24mx74JkLst2Z7gEm*hbz9gfIUu!0M$");
	twnNetworkMessage->setArguments(args);

	d->stream->write(twnNetworkMessage);
}

void Connector_Test::slotExit()
{
	deleteLater();
}

void Connector_Test::slotReadNetworkMessage()
{
	NetworkMessage *readNetworkMessage = d->stream->read();
	if(readNetworkMessage)
	{
		qDebug() << Q_FUNC_INFO << "Data received: " << readNetworkMessage->toString().remove("\r\n");
		if(readNetworkMessage->command() == QLatin1String("VER"))
		{
			loginProcessCvr();
		}
		if(readNetworkMessage->command() == QLatin1String("CVR"))
		{
			loginProcessTwnI();
		}
		if(readNetworkMessage->command() == QLatin1String("XFR"))
		{
			QString server = readNetworkMessage->arguments()[1].section(":", 0, 0);
			QString port = readNetworkMessage->arguments()[1].section(":", 1, 1);
			bool dummy;

			d->stream->close();
			d->stream->connectToServer(server, port.toUInt(&dummy));
		}
		if(readNetworkMessage->command() == QLatin1String("USR"))
		{
			if(readNetworkMessage->arguments()[0] == QLatin1String("TWN") && readNetworkMessage->arguments()[1] == QLatin1String("S"))
			{
				loginProcessTwnS();
			}
		}

		bool isNumber;
		int errorCode = readNetworkMessage->command().toUInt(&isNumber);
		if(isNumber)
		{
			qDebug() << Q_FUNC_INFO << "Received error code" << errorCode << ". Closing...";
			d->stream->close();
			deleteLater();
			QCoreApplication::exit(0);
		}
	}
	else
	{
		qDebug() << Q_FUNC_INFO << "Error in the transfer.";
	}
}


int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);

	Connector_Test *cTest = new Connector_Test();
	cTest->connectToServer();
	
	return app.exec();
}

#include "connector_test.moc"
