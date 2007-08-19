/*
	roamingservicebinding.cpp: Roaming Content Service Binding

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
class RoamingServiceBinding::Private
{
public:
	Private()
	 : connection(0)
	{}

	QString 		TicketToken;
	QString			server;
	HttpConnection 	*connection;
};

RoamingServiceBinding::RoamingServiceBinding(HttpConnection *connection, QObject *parent)
 : QObject(parent), d(new Private)
{
	d->connection = connection;
	connect(d->connection, SIGNAL(readyRead()), this, SLOT(connectionReadyRead()));
}

RoamingServiceBinding::~RoamingServiceBinding()
{
	delete d;
}

RoamingServiceBinding::setTicketToken(const QString &Compact6)
{
	d->TicketToken = Compact6;
}

void RoamingServiceBinding::connectToServer(const QString &server)
{
	QEventLoop tempEventLoop;
	connect(d->connection, SIGNAL(connected()), &tempEventLoop, SLOT(quit()));
	// FIXME: React on a connecting error.
	d->connection->connectToServer(server);

	tempEventLoop.exec();
}

void RoamingServiceBinding::getProfile()
{
	// Connect to server
	d->server = QLatin1String("blu1.storage.msn.com");
	connectToServer(d->server);

	HttpTransfer *transfer = new HttpTransfer;
	transfer->setRequest( QLatin1String("POST"), QLatin1String("/abservice/abservice.asmx") );
	transfer->setContentType( QLatin1String("text/xml") );
	transfer->setValue( QLatin1String("Host"), server );
	transfer->setValue( QLatin1String("SOAPAction"), QLatin1String("http://www.msn.com/webservices/AddressBook/ABFindAll") );
	transfer->setValue( QLatin1String("User-Agent"), QLatin1String("libpapillon") );
	transfer->setValue( QLatin1String("Cookie"), d->connection->cookie() );

	// TODO: Use Papillon Soap classes instead.
	QByteArray soapData = QString::fromUtf8("
<?xml version=\"1.0\" encoding=\"utf-8\"?><soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soapenc=\"http://schemas.xmlsoap.org/soap/encoding/\">
  <soap:Header>
    <StorageApplicationHeader xmlns=\"http://www.msn.com/webservices/storage/w10\">
      <ApplicationID>Messenger Client 8.5</ApplicationID>
      <Scenario>Initial</Scenario>
    </StorageApplicationHeader>
    <StorageUserHeader xmlns=\"http://www.msn.com/webservices/storage/w10\">
      <Puid>0</Puid>
      <TicketToken>%s</TicketToken>
    </StorageUserHeader>
  </soap:Header>
  <soap:Body>
    <GetProfile xmlns=\"http://www.msn.com/webservices/storage/w10\">
      <profileHandle>
        <Alias>
          <Name>-7779442052170503227</Name>
          <NameSpace>MyCidStuff</NameSpace>
        </Alias>
        <RelationshipName>MyProfile</RelationshipName>
      </profileHandle>
      <profileAttributes>
        <ResourceID>true</ResourceID>
        <DateModified>true</DateModified>
        <ExpressionProfileAttributes>
          <ResourceID>true</ResourceID>
          <DateModified>true</DateModified>
          <DisplayName>true</DisplayName>
          <DisplayNameLastModified>true</DisplayNameLastModified>
          <PersonalStatus>true</PersonalStatus>
          <PersonalStatusLastModified>true</PersonalStatusLastModified>
          <StaticUserTilePublicURL>true</StaticUserTilePublicURL>
          <Photo>true</Photo>
          <Flags>true</Flags>
        </ExpressionProfileAttributes>
      </profileAttributes>
    </GetProfile>
  </soap:Body>
</soap:Envelope>").args(d->TicketToken).toUtf8();

	transfer->setBody( soapData );

	d->connection->write(transfer);
}

void RoamingServiceBinding::connectionReadyRead()
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
			QDomElement abElement = bodyElement.firstChild().toElement();
			if( !abElement.isNull() )
			{
				if( (abElement.tagName() == QLatin1String("GetProfileResponse")) )
				{

				}
				else if( (abElement.tagName() == QLatin1String("FindDocumentsResponse")) )
				{

				}
			}
		}
		node = node.nextSibling();
	}
}

void RoamingServiceBinding::findDocuments()
{
	// Connect to server
	d->server = QString("blu1.storage.msn.com");
	connectToServer(d->server);

	HttpTransfer *transfer = new HttpTransfer;
	transfer->setRequest( QLatin1String("POST"), QLatin1String("/storageservice/SchematizedStore.asmx") );
	transfer->setContentType( QLatin1String("text/xml") );
	transfer->setValue( QLatin1String("Host"), server );
	transfer->setValue( QLatin1String("SOAPAction"), QLatin1String("http://www.msn.com/webservices/storage/w10/FindDocuments") );
	transfer->setValue( QLatin1String("User-Agent"), QLatin1String("libpapillon") );
	transfer->setValue( QLatin1String("Cookie"), d->connection->cookie() );

	// TODO: Use Papillon Soap classes instead.
	QByteArray soapData = QString::fromUtf8("
<?xml version=\"1.0\" encoding=\"utf-8\"?><soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soapenc=\"http://schemas.xmlsoap.org/soap/encoding/\">
  <soap:Header>
    <AffinityCacheHeader xmlns=\"http://www.msn.com/webservices/storage/w10\">
      <CacheKey>1pOMpl9ZfTSONUtexkqVS7h8II-lGcj0gCTslFfvkV2VCvGJBoIMaKeA</CacheKey>
    </AffinityCacheHeader>
    <StorageApplicationHeader xmlns=\"http://www.msn.com/webservices/storage/w10\">
      <ApplicationID>Messenger Client 8.5</ApplicationID>
      <Scenario>Initial</Scenario>
    </StorageApplicationHeader>
    <StorageUserHeader xmlns=\"http://www.msn.com/webservices/storage/w10\">
      <Puid>0</Puid>
      <TicketToken>%s</TicketToken>
    </StorageUserHeader>
  </soap:Header>
  <soap:Body>
    <FindDocuments xmlns=\"http://www.msn.com/webservices/storage/w10\">
      <objectHandle>
        <RelationshipName>/UserTiles</RelationshipName>
        <Alias>
          <Name>-7779442052170503227</Name>
          <NameSpace>MyCidStuff</NameSpace>
        </Alias>
      </objectHandle>
      <documentAttributes>
        <ResourceID>true</ResourceID>
        <Name>true</Name>
      </documentAttributes>
      <documentFilter>
        <FilterAttributes>None</FilterAttributes>
      </documentFilter>
      <documentSort>
        <SortBy>DateModified</SortBy>
      </documentSort>
      <findContext>
        <FindMethod>Default</FindMethod>
        <ChunkSize>25</ChunkSize>
      </findContext>
    </FindDocuments>
  </soap:Body>
</soap:Envelope>").args(d->TicketToken).toUtf8();

	transfer->setBody( soapData );

	d->connection->write(transfer);
}

}
#include "roamingservicebinding.moc"
