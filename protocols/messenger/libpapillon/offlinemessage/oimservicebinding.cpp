/*
	oimservicebinding.cpp: Address Book OIM Service Binding

    Copyright (c) 2007		by Zhang Panyong	        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers	<kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
namespace Papillon
{

class OIMServiceBinding::Private
{
public:
	Private()
	 : connection(0)
	{}

	QString 	t;
	QString		p;
	QString		server;
	HttpConnection *connection;
};

OIMServiceBinding::OIMServiceBinding(HttpConnection *connection, QObject *parent)
 : QObject(parent), d(new Private)
{
	d->server = QLatin1String("rsi.hotmail.com");
	d->connection = connection;
	connect(d->connection, SIGNAL(readyRead()), this, SLOT(connectionReadyRead()));
}

OIMServiceBinding::~OIMServiceBinding()
{
	delete d;
}

OIMServiceBinding::setTicketToken(const QString& t, const QString &p)
{
	d->t = t;
	d->p = p;
}

void OIMServiceBinding::connectToServer(const QString &server)
{
	QEventLoop tempEventLoop;
	connect(d->connection, SIGNAL(connected()), &tempEventLoop, SLOT(quit()));
	// FIXME: React on a connecting error.
	d->connection->connectToServer(server);

	tempEventLoop.exec();
}

void OIMServiceBinding::getOfflineMessage(const QString &messageId)
{
	// Connect to server
	connectToServer(d->server);

	HttpTransfer *transfer = new HttpTransfer;
	transfer->setRequest( QLatin1String("POST"), QLatin1String("/rsi/rsi.asmx") );
	transfer->setContentType( QLatin1String("text/xml") );
	transfer->setValue( QLatin1String("Host"), server );
	transfer->setValue( QLatin1String("SOAPAction"), QLatin1String("http://www.hotmail.msn.com/ws/2004/09/oim/rsi/GetMessage") );
	transfer->setValue( QLatin1String("User-Agent"), QLatin1String("libpapillon") );
	transfer->setValue( QLatin1String("Cookie"), d->connection->cookie() );

	// TODO: Use Papillon Soap classes instead.
	QByteArray soapData = QString::fromUtf8(
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"\
"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"\
	"<soap:Header>"\
		"<PassportCookie xmlns=\"http://www.hotmail.msn.com/ws/2004/09/oim/rsi\">"\
			"<t>%s</t>"\
			"<p>%s</p>"\
		"</PassportCookie>"\
	"</soap:Header>"\
	"<soap:Body>"\
		"<GetMessage xmlns=\"http://www.hotmail.msn.com/ws/2004/09/oim/rsi\">"\
			"<messageId>%s</messageId>"\
			"<alsoMarkAsRead>false</alsoMarkAsRead>"\
		"</GetMessage>"\
	"</soap:Body>"\
"</soap:Envelope>"
			).args(d->t).args(d->p).args(messageId).toUtf8();

	transfer->setBody( soapData );

	d->connection->write(transfer);
}

void OIMServiceBinding::sendOfflineMessage()
{
	// Connect to server
	d->server = QLatin1String("ows.messenger.msn.com");
	connectToServer(d->server);

	HttpTransfer *transfer = new HttpTransfer;
	transfer->setRequest( QLatin1String("POST"), QLatin1String("/OimWS/oim.asmx") );
	transfer->setContentType( QLatin1String("text/xml") );
	transfer->setValue( QLatin1String("Host"), server );
	transfer->setValue( QLatin1String("SOAPAction"), QLatin1String("http://messenger.msn.com/ws/2006/09/oim/Store2") );
	transfer->setValue( QLatin1String("User-Agent"), QLatin1String("libpapillon") );
	transfer->setValue( QLatin1String("Cookie"), d->connection->cookie() );

	// TODO: Use Papillon Soap classes instead.
	QByteArray soapData = QString::fromUtf8("
<?xml version=\"1.0\" encoding=\"utf-8\"?>"\
"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"\
	"<soap:Header>"\
		"<From memberName=\"%s\" friendlyName=\"%s\" xml:lang=\"en-US\" proxy=\"MSNMSGR\" xmlns=\"http://messenger.msn.com/ws/2004/09/oim/\" msnpVer=\"MSNP15\" buildVer=\"8.5.1238\"/>"\
		"<To memberName=\"%s\" xmlns=\"http://messenger.msn.com/ws/2004/09/oim/\"/>"\
		"<Ticket passport=\"%s\" appid=\"%s\" lockkey=\"%s\" xmlns=\"http://messenger.msn.com/ws/2004/09/oim/\"/>"\
		"<Sequence xmlns=\"http://schemas.xmlsoap.org/ws/2003/03/rm\">"\
			"<Identifier xmlns=\"http://schemas.xmlsoap.org/ws/2002/07/utility\">http://messenger.msn.com</Identifier>"\
			"<MessageNumber>%d</MessageNumber>"\
		"</Sequence>"\
	"</soap:Header>"\
	"<soap:Body>"\
		"<MessageType xmlns=\"http://messenger.msn.com/ws/2004/09/oim/\">text</MessageType>"\
		"<Content xmlns=\"http://messenger.msn.com/ws/2004/09/oim/\">%s</Content>"\
	"</soap:Body>"\
"</soap:Envelope>").args(d->TicketToken).toUtf8();

	transfer->setBody( soapData );

	d->connection->write(transfer);
}

void OIMServiceBinding::deleteOfflineMessage(QString &messageId)
{
	// Connect to server
	connectToServer(d->server);

	HttpTransfer *transfer = new HttpTransfer;
	transfer->setRequest( QLatin1String("POST"), QLatin1String("/rsi/rsi.asmx") );
	transfer->setContentType( QLatin1String("text/xml") );
	transfer->setValue( QLatin1String("Host"), server );
	transfer->setValue( QLatin1String("SOAPAction"), QLatin1String("http://www.hotmail.msn.com/ws/2004/09/oim/rsi/DeleteMessages") );
	transfer->setValue( QLatin1String("User-Agent"), QLatin1String("libpapillon") );
	transfer->setValue( QLatin1String("Cookie"), d->connection->cookie() );

	// TODO: Use Papillon Soap classes instead.
	QByteArray soapData = QString::fromUtf8(
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"\
"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"\
	"<soap:Header>"\
		"<PassportCookie xmlns=\"http://www.hotmail.msn.com/ws/2004/09/oim/rsi\">"\
			"<t>%s</t>"\
			" <p>%s</p>"\
		"</PassportCookie>"\
	"</soap:Header>"\
	"<soap:Body>"\
		"<DeleteMessages xmlns=\"http://www.hotmail.msn.com/ws/2004/09/oim/rsi\">"\
			"<messageIds>"\
				"<messageId>%s</messageId>"\
			"</messageIds>"\
		"</DeleteMessages>"\
	"</soap:Body>"\
"</soap:Envelope>").args(d->t).args(d->p).args(messageId).toUtf8();

	transfer->setBody( soapData );

	d->connection->write(transfer);
}

void OIMServiceBinding::connectionReadyRead()
{
	HttpTransfer *transfer = d->connection->read();
	if( !transfer )
	{
		//TODO:Error processing
	}

	// We longer need the HTTP connection, close it.
	d->connection->disconnectFromServer();

	QByteArray data = transfer->body();

	QDomDocument responseDocument;
	responseDocument.setContent(data, true);

	QDomNode node = responseDocument.documentElement().firstChild();
	while( !node.isNull() )
	{
		QDomElement bodyElement = node.toElement();
		if( (!bodyElement.isNull()) && ( bodyElement.tagName() == QLatin1String("Body") ) )
		{
			QDomElement oimElement = bodyElement.firstChild().toElement();
			if( !oimElement.isNull() )
			{
				if( abElement.tagName() == QLatin1String("Fault") )
				{
					emit StoreFault();
				}
				else if ( oimElement.tagName() == QLatin1String("StoreResponse") )
				{
					emit sendOfflineMessage(true);
				}
				else if( oimElement.tagName() == QLatin1String("GetMessageResponse") )
				{
					parseGetOfflineMessage( );
				}
				else if( oimElement.tagName() == QLatin1String("DeleteMessagesResponse") )
				{
					if(oimElement.text() == "")
					{
						emit deleteMessage(true);
					}
				}

			}
		}
		node = node.nextSibling();
	}
}

}
#include "oimservicebinding.moc"
