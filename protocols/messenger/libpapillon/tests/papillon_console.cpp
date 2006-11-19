/*
   papillon_console.cpp - GUI Papillon debug console.

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
#include "papillon_console.h"

// Qt includes
#include <QtDebug>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <QtCore/QSettings>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QInputDialog>

// Papillon includes
#include "connection.h"
#include "papillonclientstream.h"
#include "logintask.h"
#include "qtconnector.h"
#include "transfer.h"
#include "securestream.h"

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
	 : logged(false), client(0), settings(0), sslStream(0), soapResultFile(0)
	{
		settings = new QSettings( QLatin1String("papillonconsole.ini"), QSettings::IniFormat );
	}
	~Private()
	{
		delete client;
		delete settings;
	}

	QTextEdit *textDebugOutput;
	QLineEdit *lineCommand;
	QLineEdit *linePayload;
	QPushButton *buttonSend;
	QPushButton *buttonConnect;

	bool logged;
	Client *client;

	QSettings *settings;
	SecureStream *sslStream;
	QFile *soapResultFile;
	QTextStream *textStream;
};

PapillonConsole::PapillonConsole(QWidget *parent)
 : QWidget(parent), d(new Private)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	d->textDebugOutput = new QTextEdit(this);
	d->textDebugOutput->setReadOnly(true);
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

	QPushButton *soap = new QPushButton( QLatin1String("Test SOAP"), this );
	connect(soap, SIGNAL(clicked()), this, SLOT(buttonTestSOAP()));
	layout->addWidget(soap);

	connect(d->buttonSend, SIGNAL(clicked()), this, SLOT(buttonSendClicked()));
	connect(d->lineCommand, SIGNAL(returnPressed()), this, SLOT(buttonSendClicked()));
	connect(d->buttonConnect, SIGNAL(clicked()), this, SLOT(buttonConnectClicked()));

	// Create the client
	d->client = new Client(new QtConnector(this), this);
	connect(d->client, SIGNAL(connected()), this, SLOT(clientConnected()));
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
	QString passportId, password;
	if( d->settings->value( QLatin1String("passportId") ).toString().isEmpty() && d->settings->value( QLatin1String("password") ).toString().isEmpty() )
	{
		passportId = QInputDialog::getText( this, QString("Passport ID"), QString("Enter your Microsoft Passport account ID:") );
		password = QInputDialog::getText( this, QString("Passport password"), QString("Enter your Microsoft Passport password:"), QLineEdit::Password);
	
		d->settings->setValue( QLatin1String("passportId"), passportId );
		d->settings->setValue( QLatin1String("password"), password );
	}
	else
	{
		passportId = d->settings->value( QLatin1String("passportId") ).toString();
		password = d->settings->value( QLatin1String("password") ).toString();
	}

	d->client->setClientInfo( passportId, password );
	d->client->connectToServer( QLatin1String("messenger.hotmail.com"), 1863 );
}

void PapillonConsole::buttonTestSOAP()
{
	if(!d->sslStream)
	{
		d->sslStream = new SecureStream(new QtConnector(this));
		connect(d->sslStream, SIGNAL(connected()), this, SLOT(streamConnected()));
		connect(d->sslStream, SIGNAL(readyRead()), this, SLOT(streamReadyRead()));

		d->soapResultFile = new QFile( QLatin1String("soapresult.txt"), this);
		d->soapResultFile->open( QIODevice::ReadWrite | QIODevice::Truncate );
		d->textStream = new QTextStream(d->soapResultFile);
	}

	d->sslStream->connectToServer( QLatin1String("omega.contacts.msn.com") );
}

void PapillonConsole::streamConnected()
{
	qDebug() << "SOAP Test connected.";
	QByteArray soapData = QString::fromUtf8("<?xml version='1.0' encoding='utf-8'?>\r\n"
"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
    "<soap:Header xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "<ABApplicationHeader xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
            "<ApplicationId xmlns=\"http://www.msn.com/webservices/AddressBook\">09607671-1C32-421F-A6A6-CBFAA51AB5F4</ApplicationId>\r\n"
            "<IsMigration xmlns=\"http://www.msn.com/webservices/AddressBook\">false</IsMigration>\r\n"
            "<PartnerScenario xmlns=\"http://www.msn.com/webservices/AddressBook\">Initial</PartnerScenario>\r\n"
        "</ABApplicationHeader>\r\n"
        "<ABAuthHeader xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
            "<ManagedGroupRequest xmlns=\"http://www.msn.com/webservices/AddressBook\">false</ManagedGroupRequest>\r\n"
        "</ABAuthHeader>\r\n"
    "</soap:Header>\r\n"
    "<soap:Body xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "<FindMembership xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
            "<serviceFilter xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
                "<Types xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
                    "<ServiceType xmlns=\"http://www.msn.com/webservices/AddressBook\">Messenger</ServiceType>\r\n"
                    "<ServiceType xmlns=\"http://www.msn.com/webservices/AddressBook\">Invitation</ServiceType>\r\n"
                    "<ServiceType xmlns=\"http://www.msn.com/webservices/AddressBook\">SocialNetwork</ServiceType>\r\n"
                    "<ServiceType xmlns=\"http://www.msn.com/webservices/AddressBook\">Space</ServiceType>\r\n"
                    "<ServiceType xmlns=\"http://www.msn.com/webservices/AddressBook\">Profile</ServiceType>\r\n"
                "</Types>\r\n"
            "</serviceFilter>\r\n"
        "</FindMembership>\r\n"
    "</soap:Body>\r\n"
"</soap:Envelope>\r\n").toUtf8();

	QByteArray soapHeader = QString::fromUtf8("POST /abservice/SharingService.asmx HTTP/1.1\r\n"
"SOAPAction: http://www.msn.com/webservices/AddressBook/FindMembership\r\n"
"Content-Type: text/xml; charset=utf-8\r\n"
"Cookie: MSPAuth=%1\r\n"
"Host: omega.contacts.msn.com\r\n"
"Content-Length: %2\r\n"
"\r\n"
).arg( d->client->passportAuthTicket() ).arg( soapData.size() ).toUtf8();

	QByteArray soapRequest;
	soapRequest = soapHeader + soapData;

	d->sslStream->write( soapRequest );
}

void PapillonConsole::streamReadyRead()
{
	qDebug() << "PapillonConsole::streamReadyRead()";
	QByteArray read = d->sslStream->read();
	*d->textStream << read;
	qDebug() << "PapillonConsole::streamReadyRead(): Read" << read.size();

	if( QString(read).indexOf("</soap:Envelope>") != -1 )
	{
		qDebug() << "SOAP result end.";
		d->soapResultFile->close();
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
