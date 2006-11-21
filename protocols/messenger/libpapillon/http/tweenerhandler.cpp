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
#include <QtNetwork/QHttpHeader>
#include <QtNetwork/QHttpRequestHeader>
#include <QtNetwork/QHttpResponseHeader>

// Papillon includes
#include "Papillon/Http/SecureStream"

namespace Papillon 
{

class TweenerHandler::Private
{
public:
	Private()
	 : stream(0), success(false)
	{}

	~Private()
	{
		delete stream;
	}

	QString tweener;
	QString passportId;
	QString password;
	QString ticket;
	
	QString loginUrl;

	SecureStream *stream;
	bool success;

	TweenerHandler::TweenerState state;
};

TweenerHandler::TweenerHandler(SecureStream *stream)
 : QObject(0), d(new Private)
{
	d->stream = stream;
	connect(d->stream, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(d->stream, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
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
	qDebug() << PAPILLON_FUNCINFO << "Begin tweener ticket negotiation.";
	Q_ASSERT( !d->tweener.isEmpty() );
	Q_ASSERT( !d->passportId.isEmpty() );
	Q_ASSERT( !d->password.isEmpty() );

	d->state = TwnGetServer;
	d->stream->connectToServer("nexus.passport.com");
}

void TweenerHandler::slotConnected()
{
	qDebug() << PAPILLON_FUNCINFO << "We are connected";
	switch(d->state)
	{
		case TwnGetServer:
		{
			qDebug() << PAPILLON_FUNCINFO << "Getting real login server host...";
			QHttpRequestHeader getLoginServer( QLatin1String("GET"), QLatin1String("/rdr/pprdr.asp"), 1, 0 );
			sendRequest(getLoginServer);

			break;
		}
		case TwnAuth:
		{
			qDebug() << PAPILLON_FUNCINFO << "Sending auth...";
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
	QByteArray read = d->stream->read();
	QString temp(read);
	QHttpResponseHeader httpHeader( temp );
	if( !httpHeader.isValid() )
		qDebug() << PAPILLON_FUNCINFO << "QHttpResponseHeader is not valid !";
	
	// Handle Redirection(302)
	if( httpHeader.statusCode() == 302 )
	{
		QString location = httpHeader.value( QLatin1String("location") );
		QString loginServer = location.section("/", 0, 0);
		d->loginUrl = QLatin1String("/") + location.section("/", 1);

		qDebug() << PAPILLON_FUNCINFO << "Redirect to" << location;
		changeServer(loginServer);
	}
	// Handle failure(401 Unauthorized)
	else if( httpHeader.statusCode() == 401 )
	{
		qDebug() << PAPILLON_FUNCINFO << "Passport refused the password.";
		emitResult(false);
	}
	else if( httpHeader.statusCode() == 400 )
	{
		qDebug() << PAPILLON_FUNCINFO << "DEBUG: Bad request.";
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
				qDebug() << PAPILLON_FUNCINFO << "Connecting to auth server. Server:" << login;

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
				
				d->stream->disconnectFromServer();
				emitResult(true);
				break;
			}
			default:
				break;
		}
	}
}

void TweenerHandler::changeServer(const QString &host)
{
	d->stream->disconnectFromServer();
	d->stream->connectToServer(host);
}

void TweenerHandler::sendRequest(const QHttpRequestHeader &httpHeader)
{
// 	qDebug() << PAPILLON_FUNCINFO << "Sending: " << httpHeader.toString().replace("\r", "(r)").replace("\n", "(n)");
	QByteArray data;
	data += httpHeader.toString().toUtf8();
	// Insert empty body.
	data += "\r\n";

	d->stream->write( data );
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
	d->stream->disconnectFromServer();

	d->success = success;
	emit result(this);
}

}

#include "tweenerhandler.moc"
