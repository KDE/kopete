/*
   ssohandler.cpp - Windows Live Messenger SSO authentication

   Copyright (c) 2007 by Zhang PanYong <pyzhang@gmail.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

namespace Papillon 
{
class SSOHandler::Private
{
public:
	Private()
	 : stream(0), success(false)
	{}

	~Private()
	{
		delete stream;
	}

	QString sso;
	QString passportId;
	QString password;

	SecureStream *stream;
	bool success;

};

SSOHandler::SSOHandler(SecureStream *stream)
 : QObject(0), d(new Private)
{
	d->stream = stream;
	connect(d->stream, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}

SSOHandler::~SSOHandler()
{
	delete d;
}

void SSOHandler::setLoginInformation(const QString &sso, const QString &passportId, const QString &password)
{
	d->sso = sso;
	d->passportId = passportId;
	d->password = password;
}

void SSOHandler::start()
{
	qDebug() << PAPILLON_FUNCINFO << "Begin sso ticket negotiation.";
	Q_ASSERT( !d->sso.isEmpty() );
	Q_ASSERT( !d->passportId.isEmpty() );
	Q_ASSERT( !d->password.isEmpty() );

	d->state = SSOGetServer;
	d->stream->connectToServer("login.live.com");
}

void SSOHandler::changeServer(const QString &host)
{
	d->stream->disconnectFromServer();
	d->stream->connectToServer(host);
}

void SSOHandler::emitResult(bool success)
{
	d->stream->disconnectFromServer();

	d->success = success;
	emit result(this);
}

}
#include "ssohandler.moc"
