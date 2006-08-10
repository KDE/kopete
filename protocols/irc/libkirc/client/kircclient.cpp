/*
    kircclient.cpp - IRC Client

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kircclient.moc"

#include "kircclientcommandhandler.h"
#include "kircentity.h"
#include "kircstdmessages.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>

class KIrc::Client::Private
{
public:
	Private()
		: server(new Entity(QString::null, KIrc::Entity::Server))
		, failedNickOnLogin(false)
	{ }

//	KIrc::Entity::List entities;
	KIrc::Entity::Ptr server;

	bool failedNickOnLogin : 1;
};

using namespace KIrc;

Client::Client(QObject *parent)
	: Socket(parent),
	  d( new Private )
{
	kDebug(14120) << k_funcinfo << endl;

//	setUserName(QString::null);

//	d->entities << d->server << d->self;

//	d->versionString = QString::fromLatin1("Anonymous client using the KIRC engine.");
//	d->userString = QString::fromLatin1("Response not supplied by user.");
//	d->sourceString = QString::fromLatin1("Unknown client, known source.");

/*
	connect(this, SIGNAL(internalError(const QString &)),
		this, SLOT());
*/
	connect(this, SIGNAL(receivedMessage(KIrc::Message &)),
		this, SLOT(onReceivedMessage(KIrc::Message &)));
}

Client::~Client()
{
	kDebug(14120) << k_funcinfo << endl;
//	StdCommands::quit(this, QLatin1String("KIRC Deleted"));

	delete d;
}

bool Client::isDisconnected() const
{
	return connectionState() == Idle;
}

bool Client::isConnected() const
{
	return connectionState() == Open;
}

void Client::authentify()
{
/*
	// If password is given for this server, send it now, and don't expect a reply
	const KUrl &url = this->url();

	if (url.hasPass())
		StdCommands::pass(this, url.pass());

	#warning make the following string arguments static const
	StdCommands::user(this, url.user(), StdCommands::Normal, url.queryItem(URL_REALNAME));
	StdCommands::nick(this, url.queryItem(URL_NICKNAME));

	KIrc::Socket::authentify();
*/
}

Entity::Ptr Client::server()
{
	return d->server;
}

ClientCommandHandler *Client::clientCommandHandler()
{
//	return dynamic_cast<ClientCommandHandler *>(commandHandler());
	return 0;
}

