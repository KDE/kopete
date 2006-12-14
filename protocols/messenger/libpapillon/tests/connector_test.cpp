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
#include "Papillon/Transfer"
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
	connect(d->stream, SIGNAL(readyRead()), this, SLOT(slotReadTransfer()));
	
	doLoginProcess();
}

void Connector_Test::doLoginProcess()
{
	Transfer *usrTransfer = new Transfer(Transfer::TransactionTransfer);
	usrTransfer->setCommand( QLatin1String("VER") );
	usrTransfer->setTransactionId( QString::number(++d->trId) );
	usrTransfer->setArguments( QString("MSNP11 MSNP10 CVR0") );

	d->stream->write(usrTransfer);
}

void Connector_Test::loginProcessCvr()
{
	Transfer *cvrTransfer = new Transfer(Transfer::TransactionTransfer);
	cvrTransfer->setCommand( QLatin1String("CVR") );
	cvrTransfer->setTransactionId( QString::number(++d->trId) );
	cvrTransfer->setArguments( QString("0x040c winnt 5.1 i386 MSNMSGR 7.0.0777 msmsgs "PASSPORT_ID) );

	d->stream->write(cvrTransfer);
}

void Connector_Test::loginProcessTwnI()
{
	Transfer *twnTransfer = new Transfer(Transfer::TransactionTransfer);
	twnTransfer->setCommand("USR");
	twnTransfer->setTransactionId( QString::number(++d->trId) );
	twnTransfer->setArguments( QString("TWN I "PASSPORT_ID) );

	d->stream->write(twnTransfer);
}

void Connector_Test::loginProcessTwnS()
{
	Transfer *twnTransfer = new Transfer(Transfer::TransactionTransfer);
	twnTransfer->setCommand("USR");
	twnTransfer->setTransactionId( QString::number(++d->trId) );
	QString args = QString("TWN S t=7tLSuqR92Bo*17x76PDg87IVMA5FKqxccJDUocFzfCXbipUMZuMv4HatazUwTBVqsFkTxkS0qFCSbzA9SUFjM*SHzGKIIC7kgZAEikfzfAUufs*L!3B5i0aSrNo03BAeQP&p=7nhP1TIX4BGQ*k4JKZw0JHP5rb3A9wk8fw!ZYadtXN0OFiN*yZr6UaFwBAdUOKwoQkYfK1gEzWE*Op16WDcE*9J3Hv4JWG1TF3eSoAq71CITPmkZONAReXYlGz5Rk5l7ZFwbAPq6NxxqxzK24mx74JkLst2Z7gEm*hbz9gfIUu!0M$");
	twnTransfer->setArguments(args);

	d->stream->write(twnTransfer);
}

void Connector_Test::slotExit()
{
	deleteLater();
}

void Connector_Test::slotReadTransfer()
{
	Transfer *readTransfer = d->stream->read();
	if(readTransfer)
	{
		qDebug() << PAPILLON_FUNCINFO << "Data received: " << readTransfer->toString().replace("\r\n", "");
		if(readTransfer->command() == QLatin1String("VER"))
		{
			loginProcessCvr();
		}
		if(readTransfer->command() == QLatin1String("CVR"))
		{
			loginProcessTwnI();
		}
		if(readTransfer->command() == QLatin1String("XFR"))
		{
			QString server = readTransfer->arguments()[1].section(":", 0, 0);
			QString port = readTransfer->arguments()[1].section(":", 1, 1);
			bool dummy;

			d->stream->close();
			d->stream->connectToServer(server, port.toUInt(&dummy));
		}
		if(readTransfer->command() == QLatin1String("USR"))
		{
			if(readTransfer->arguments()[0] == QLatin1String("TWN") && readTransfer->arguments()[1] == QLatin1String("S"))
			{
				loginProcessTwnS();
			}
		}

		bool isNumber;
		int errorCode = readTransfer->command().toUInt(&isNumber);
		if(isNumber)
		{
			qDebug() << PAPILLON_FUNCINFO << "Received error code" << errorCode << ". Closing...";
			d->stream->close();
			deleteLater();
			QCoreApplication::exit(0);
		}
	}
	else
	{
		qDebug() << PAPILLON_FUNCINFO << "Error in the transfer.";
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
