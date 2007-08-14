/*
   tweenerhandler.cpp - Negociation with Passport to get the login ticket.

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
#include "Papillon/Http/TweenerHandler"

// Qt includes
#include <QtDebug>
#include <QtCore/QRegExp>
#include <QtCore/QUrl>
#include <QtCore/QEventLoop>
#include <QtNetwork/QHttpHeader>
#include <QtNetwork/QHttpRequestHeader>
#include <QtNetwork/QHttpResponseHeader>

// Papillon includes
#include "Papillon/Network/IpEndpointConnector"
#include "Papillon/Network/NetworkStream"

namespace Papillon 
{

class TweenerHandler::Private
{
public:
	Private()
	 : connector(0), success(false), connectionTry(0)
	{}

	~Private()
	{
		delete connector;
	}

	QString tweener;
	QString passportId;
	QString password;
	QString ticket;
	
	QString loginUrl;

	IpEndpointConnector *connector;
	bool success;

	TweenerHandler::TweenerState state;
	int connectionTry;
};

TweenerHandler::TweenerHandler(QObject *parent)
 : QObject(parent), d(new Private)
{
	d->connector = new IpEndpointConnector(true, this); // Use TLS/SSL
	connect(d->connector, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(d->connector, SIGNAL(faulted()), this, SLOT(connector_OnFaulted()));
}

TweenerHandler::~TweenerHandler()
{
	delete d;
}

void TweenerHandler::setLoginInformation(const QString &tweener, const QString &passportId, const QString &password)
{
	d->tweener = tweener;
	d->passportId = passportId;
	d->password = password;
}

void TweenerHandler::start()
{
	qDebug() << Q_FUNC_INFO << "Begin tweener ticket negotiation.";
	Q_ASSERT( !d->tweener.isEmpty() );
	Q_ASSERT( !d->passportId.isEmpty() );
	Q_ASSERT( !d->password.isEmpty() );

	disconnect(d->connector->networkStream());

	d->state = TwnGetServer;
	d->connector->connectWithAddressInfo("nexus.passport.com", 443);	
}

void TweenerHandler::slotConnected()
{
	qDebug() << Q_FUNC_INFO << "We are connected";
	
	connect(d->connector->networkStream(), SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

	switch(d->state)
	{
		case TwnGetServer:
		{
			qDebug() << Q_FUNC_INFO << "Getting real login server host...";
			QHttpRequestHeader getLoginServer( QLatin1String("GET"), QLatin1String("/rdr/pprdr.asp"), 1, 0 );
			sendRequest(getLoginServer);

			break;
		}
		case TwnAuth:
		{
			qDebug() << Q_FUNC_INFO << "Sending auth...";
			QHttpRequestHeader login( QLatin1String("GET"), d->loginUrl );
			login.setValue( QLatin1String("Host"), QLatin1String("login.passport.com") );

			QString authRequest = QLatin1String("Passport1.4 OrgVerb=GET,OrgURL=http%3A%2F%2Fmessenger%2Emsn%2Ecom,sign-in=") +
 								QUrl::toPercentEncoding(d->passportId) +
								QLatin1String(",pwd=") + QUrl::toPercentEncoding(d->password ).replace(',',"%2C") +
								QLatin1String(",") + d->tweener;
			login.setValue( QLatin1String("Authorization"), authRequest );
			
			sendRequest(login);

			break;
		}
		default:
			break;
	}
}

void TweenerHandler::slotReadyRead()
{
	QByteArray read = d->connector->networkStream()->readAll();
	QString temp(read);
	QHttpResponseHeader httpHeader( temp );
	if( !httpHeader.isValid() )
		qDebug() << Q_FUNC_INFO << "QHttpResponseHeader is not valid !";
	
	// Handle Redirection(302)
	if( httpHeader.statusCode() == 302 )
	{
		QString location = httpHeader.value( QLatin1String("location") );
		QString loginServer = location.section("/", 0, 0);
		d->loginUrl = QLatin1String("/") + location.section("/", 1);

		qDebug() << Q_FUNC_INFO << "Redirect to" << location;
		changeServer(loginServer);
	}
	// Handle failure(401 Unauthorized)
	else if( httpHeader.statusCode() == 401 )
	{
		qDebug() << Q_FUNC_INFO << "Passport refused the password.";
		emitResult(false);
	}
	else if( httpHeader.statusCode() == 400 )
	{
		qDebug() << Q_FUNC_INFO << "DEBUG: Bad request.";
	}
	// 200 OK, do the result parsing
	else if( httpHeader.statusCode() == 200 )
	{
		switch(d->state)
		{
			case TwnGetServer:
			{
				// Retrieve login url from resulting HTTP header.
				QString passportUrls = httpHeader.value( QLatin1String("passporturls") );
				QRegExp rx("DARealm=(.*),DALogin=(.*),DAReg=");
				rx.indexIn(passportUrls);
				
				QString login = rx.cap(2);
				QString loginServer = login.section("/", 0, 0);
				d->loginUrl = QLatin1String("/") + login.section("/", 1);
	
				// Change state of negotiation process.
				d->state = TwnAuth;
				qDebug() << Q_FUNC_INFO << "Connecting to auth server. Server:" << login;

				// Connect to given URL.
				changeServer(loginServer);
				break;
			}
			case TwnAuth:
			{
				QString authInfo = httpHeader.value( QLatin1String("authentication-info") );
				QRegExp rx("from-PP='(.*)'");
				rx.indexIn(authInfo);

				d->ticket = rx.cap(1);
				
				d->connector->close();
				emitResult(true);
				break;
			}
			default:
				break;
		}
	}
}

void TweenerHandler::connector_OnFaulted()
{
	QEventLoop waiter(this);
	connect(d->connector, SIGNAL(closed()), &waiter, SLOT(quit()));

	d->connector->close();
	waiter.exec();

	if( d->connectionTry + 1 > 5 )
	{
		// Connection try has failed
		emitResult(false);
		return;
	}
	else
	{
		d->connectionTry++;
		qDebug() << Q_FUNC_INFO << "IpEndpointConnector has failed. Retry a connection. Try:" << d->connectionTry;
		// Restart negotiation process.
		start();
	}
}

void TweenerHandler::changeServer(const QString &host)
{
	d->connector->close();
	QEventLoop waiter(this);
	connect(d->connector, SIGNAL(closed()), &waiter, SLOT(quit()));

	d->connector->close();
	waiter.exec();

	d->connector->connectWithAddressInfo(host, 443); // FIXME: Maybe not hardcode the SSL port
}

void TweenerHandler::sendRequest(const QHttpRequestHeader &httpHeader)
{
// 	qDebug() << Q_FUNC_INFO << "Sending: " << httpHeader.toString().replace("\r", "(r)").replace("\n", "(n)");
	QByteArray data;
	data += httpHeader.toString().toUtf8();
	// Insert empty body.
	data += "\r\n";

	d->connector->networkStream()->write( data );
}

bool TweenerHandler::success() const
{
	return d->success;
}

QString TweenerHandler::ticket() const
{
	return d->ticket;
}

void TweenerHandler::emitResult(bool success)
{
	d->connector->close();

	d->success = success;
	emit result(this);
}

}

#include "tweenerhandler.moc"
