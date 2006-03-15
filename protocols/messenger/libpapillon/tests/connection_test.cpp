/*
   connection_test.cpp - Test for the connection architecture.

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "connection_test.h"

// Qt includes
#include <QCoreApplication>
#include <QtDebug>

// Papillon includes
#include "notificationstream.h"
#include "qtconnector.h"
#include "transfer.h"

#define PASSPORT_ID ""

using namespace Papillon;

class Connection_Test::Private
{
public:
	Private()
	 : stream(0), qtConnector(0), trId(0)
	{}
	NotificationStream *stream;
	QtConnector *qtConnector;

	int trId;
};

Connection_Test::Connection_Test(QObject *parent)
 : QObject(parent), d(new Private)
{
	d->qtConnector = new QtConnector(this);
	d->stream = new NotificationStream(d->qtConnector, this);
}

Connection_Test::~Connection_Test()
{
	delete d;
}

void Connection_Test::connectToServer()
{
	d->stream->connectToServer("messenger.hotmail.com", 1863);
	connect(d->stream, SIGNAL(connected()), this, SLOT(slotConnected()));
}

void Connection_Test::slotConnected()
{
	connect(d->stream, SIGNAL(readyRead()), this, SLOT(slotReadTransfer()));
	
	doLoginProcess();
}

void Connection_Test::doLoginProcess()
{
	Transfer *usrTransfer = new Transfer(Transfer::TransactionTransfer);
	usrTransfer->setCommand( QLatin1String("VER") );
	usrTransfer->setTransactionId( QString::number(++d->trId) );
	QStringList arguments;
	arguments  = QString("MSNP11 MSNP10 CVR0").split(" ");
	usrTransfer->setArguments(arguments);

	d->stream->write(usrTransfer);
}

void Connection_Test::loginProcessCvr()
{
	Transfer *cvrTransfer = new Transfer(Transfer::TransactionTransfer);
	cvrTransfer->setCommand( QLatin1String("CVR") );
	cvrTransfer->setTransactionId( QString::number(++d->trId) );
	QStringList args;
	args = QString("0x040c winnt 5.1 i386 MSNMSGR 7.0.0777 msmsgs "PASSPORT_ID).split(" ");
	cvrTransfer->setArguments(args);

	d->stream->write(cvrTransfer);
}

void Connection_Test::loginProcessTwnI()
{
	Transfer *twnTransfer = new Transfer(Transfer::TransactionTransfer);
	twnTransfer->setCommand("USR");
	twnTransfer->setTransactionId( QString::number(++d->trId) );
	QStringList args = QString("TWN I "PASSPORT_ID).split(" ");
	twnTransfer->setArguments(args);

	d->stream->write(twnTransfer);
}

void Connection_Test::loginProcessTwnS()
{
	Transfer *twnTransfer = new Transfer(Transfer::TransactionTransfer);
	twnTransfer->setCommand("USR");
	twnTransfer->setTransactionId( QString::number(++d->trId) );
	QStringList args = QString("TWN S t=7tLSuqR92Bo*17x76PDg87IVMA5FKqxccJDUocFzfCXbipUMZuMv4HatazUwTBVqsFkTxkS0qFCSbzA9SUFjM*SHzGKIIC7kgZAEikfzfAUufs*L!3B5i0aSrNo03BAeQP&p=7nhP1TIX4BGQ*k4JKZw0JHP5rb3A9wk8fw!ZYadtXN0OFiN*yZr6UaFwBAdUOKwoQkYfK1gEzWE*Op16WDcE*9J3Hv4JWG1TF3eSoAq71CITPmkZONAReXYlGz5Rk5l7ZFwbAPq6NxxqxzK24mx74JkLst2Z7gEm*hbz9gfIUu!0M$").split(" ");
	twnTransfer->setArguments(args);

	d->stream->write(twnTransfer);
}

void Connection_Test::slotExit()
{
	deleteLater();
}

void Connection_Test::slotReadTransfer()
{
	Transfer *readTransfer = d->stream->read();
	if(readTransfer)
	{
		qDebug() << "Data received: " << readTransfer->toString();
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
			qDebug() << "Received error code" << errorCode << ". Closing...";
			d->stream->close();
			deleteLater();
			QCoreApplication::exit(0);
		}
	}
	else
	{
		qDebug() << "Error in the transfer.";
	}
}


int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);

	Connection_Test *cTest = new Connection_Test();
	cTest->connectToServer();
	
	return app.exec();
}

#include "connection_test.moc"
