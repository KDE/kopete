/*
   papillon_console.cpp - GUI Papillon debug console.

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
#include "papillon_console.h"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>

// Papillon includes
#include "connection.h"
#include "papillonclientstream.h"
#include "logintask.h"
#include "qtconnector.h"
#include "transfer.h"

// Little hack to access writeCommand method in Client
#define private public
#include "client.h"
#undef private

#include "connection_test.h"

using namespace Papillon;

namespace PapillonConsole
{

// "Global" access to the console.
PapillonConsole *console;

class PapillonConsole::Private
{
public:
	Private()
	 : logged(false), client(0)
	{}

	QTextEdit *textDebugOutput;
	QLineEdit *lineCommand;
	QLineEdit *linePayload;
	QPushButton *buttonSend;
	QPushButton *buttonConnect;

	bool logged;
	Client *client;
};

PapillonConsole::PapillonConsole(QWidget *parent)
 : QWidget(parent), d(new Private)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	d->textDebugOutput = new QTextEdit(this);
	layout->addWidget(d->textDebugOutput);
	
	d->buttonConnect = new QPushButton( QLatin1String("Connect"), this );
	layout->addWidget(d->buttonConnect);

	QHBoxLayout *commandLayout = new QHBoxLayout(this);
	d->lineCommand = new QLineEdit(this);
	d->buttonSend = new QPushButton( QLatin1String("Send"), this );
	commandLayout->addWidget(d->lineCommand);
	commandLayout->addWidget(d->buttonSend);
	
	layout->addLayout(commandLayout);

	QHBoxLayout *payloadLayout = new QHBoxLayout(this);
	payloadLayout->addWidget( new QLabel( QLatin1String("Payload data:"), this ) );
	d->linePayload = new QLineEdit(this);
	payloadLayout->addWidget( d->linePayload );

	layout->addLayout(payloadLayout);	

	connect(d->buttonSend, SIGNAL(clicked()), this, SLOT(buttonSendClicked()));
	connect(d->lineCommand, SIGNAL(returnPressed()), this, SLOT(buttonSendClicked()));
	connect(d->buttonConnect, SIGNAL(clicked()), this, SLOT(buttonConnectClicked()));
}

PapillonConsole::~PapillonConsole()
{
	delete d;
}

void PapillonConsole::buttonSendClicked()
{
	QString parsedLine = d->lineCommand->text();
	QStringList commandList = parsedLine.split(" ");
			
	QString command;
	QStringList arguments;
	QByteArray payloadData;
			
	Transfer::TransferType transferType;
	bool dummy, isNumber;
	int trId = 0, payloadLength = 0;
			
	command = commandList.at(0);
			
	// Determine the transfer type.
	if(isPayloadCommand(command) && !d->linePayload->text().isEmpty())
	{
		transferType |= Transfer::PayloadTransfer;
		// Remove the last parameter from the command list and set the payload length.
		// So it will not be in the arguments.
		payloadLength = commandList.takeLast().toUInt(&dummy);
		payloadData = d->linePayload->text().toUtf8();
	}
			
	// Check for a transaction ID.
	// Do not check for a transaction if the commandList size is lower than 2.
	if( commandList.size() >= 2 )
	{
		trId = commandList[1].toUInt(&isNumber);
		if(isNumber)
			transferType |= Transfer::TransactionTransfer;
	}
			
	// Begin at the third command arguments if we have a transaction ID.
	int beginAt = isNumber ? 2 : 1;
	// Fill the arguments.
	for(int i = beginAt; i < commandList.size(); ++i)
	{
		arguments << commandList[i];
	}
			
	Papillon::Transfer *receivedTransfer = new Transfer(transferType);
	receivedTransfer->setCommand(command);
	receivedTransfer->setArguments(arguments);
			
	if(isNumber)
		receivedTransfer->setTransactionId( QString::number(trId) );

	if( !payloadData.isEmpty() )
		receivedTransfer->setPayloadData(payloadData);

	d->client->writeCommand(receivedTransfer);

	d->lineCommand->clear();
	d->linePayload->clear();
}

void PapillonConsole::buttonConnectClicked()
{
	if(d->client)
	{
		// TODO: Read client info for a file in tests directory.
		d->client->setClientInfo( QLatin1String("test@test.com"), QLatin1String("test") );
		d->client->connectToServer( QLatin1String("muser.messenger.hotmail.com"), 1863 );
	}
	else
	{
		d->client = new Client(new QtConnector(this), this);
		connect(d->client, SIGNAL(connected()), this, SLOT(clientConnected()));
	}

	
}

void PapillonConsole::clientConnected()
{
	if( !d->logged )
	{
		d->logged = true;
		d->client->login();
	}
}

bool PapillonConsole::isPayloadCommand(const QString &command)
{
	if( command == QLatin1String("ADL") ||
	    command == QLatin1String("GCF") ||
	    command == QLatin1String("MSG") ||
	    command == QLatin1String("QRY") ||
	    command == QLatin1String("RML") ||
	    command == QLatin1String("UBX") ||
	    command == QLatin1String("UBN") ||
	    command == QLatin1String("UUN") ||
	    command == QLatin1String("UUX")
	  )
		return true;
	else
		return false;
}

void guiDebugOutput(QtMsgType type, const char *msg)
{
	if( console )
	{
		switch(type)
		{
			case QtDebugMsg:
				console->d->textDebugOutput->append( QString(msg) );
				break;
			case QtWarningMsg:
				console->d->textDebugOutput->append( QString("Warning: %1").arg( QString(msg) ) );
				break;
			default:
				break;
		}
	}
}

}

int main(int argc, char **argv)
{
	qInstallMsgHandler(PapillonConsole::guiDebugOutput);

	QApplication app(argc, argv);
	
	PapillonConsole::console = new PapillonConsole::PapillonConsole;
	PapillonConsole::console->show();

	return app.exec();
}

#include "papillon_console.moc"
