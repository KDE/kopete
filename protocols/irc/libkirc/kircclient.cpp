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
#include "kircstdcommands.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include <qtextcodec.h>
#include <qtimer.h>

using namespace KIRC;

class KIRC::Client::Private
{
public:
	bool failedNickOnLogin;

//	KIRC::Entity::List entities;
	KIRC::Entity::Ptr server;
};

Client::Client(QObject *parent)
	: KIRC::Socket(parent),
	  d( new Private )
{
/*
	  d->FailedNickOnLogin(false),
	  d->useSSL(false),
	  d->server(new Entity(QString::null, KIRC::Server)),
	  d->self(new Entity(QString::null, KIRC::User))
*/
//	setUserName(QString::null);

//	d->entities << d->server << d->self;

//	bindCommands();
//	bindNumericReplies();
//	bindCtcp();

//	d->versionString = QString::fromLatin1("Anonymous client using the KIRC engine.");
//	d->userString = QString::fromLatin1("Response not supplied by user.");
//	d->sourceString = QString::fromLatin1("Unknown client, known source.");

/*
	connect(this, SIGNAL(internalError(const QString &)),
		this, SLOT());
*/
	connect(this, SIGNAL(receivedMessage(KIRC::Message &)),
		this, SLOT(onReceivedMessage(KIRC::Message &)));
}

Client::~Client()
{
//	kdDebug(14120) << k_funcinfo << d->Host << endl;
	StdCommands::quit(this, QString::fromLatin1("KIRC Deleted"));

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
/*
bool Engine::_bind(QMap<QString, KIRC::MessageRedirector *> &dict,
		const char *command, QObject *object, const char *member,
		int minArgs, int maxArgs, const QString &helpMessage)
{
//	FIXME: Force upper case.
//	FIXME: Force number format.

	MessageRedirector *mr = dict[command];

	if (!mr)
	{
		mr = new MessageRedirector(this, minArgs, maxArgs, helpMessage);
		dict.replace(command, mr);
	}

	return mr->connect(object, member);
}

bool Engine::bind(const char *command, QObject *object, const char *member,
	int minArgs, int maxArgs, const QString &helpMessage)
{
	return _bind(d->commands, command, object, member,
		minArgs, maxArgs, helpMessage);
}

bool Engine::bind(int id, QObject *object, const char *member,
		int minArgs, int maxArgs, const QString &helpMessage)
{
	return _bind(d->commands, QByteArray::number(id), object, member,
		     minArgs, maxArgs, helpMessage);
}

bool Engine::bindCtcpQuery(const char *command, QObject *object, const char *member,
	int minArgs, int maxArgs, const QString &helpMessage)
{
	return _bind(d->ctcpQueries, command, object, member,
		minArgs, maxArgs, helpMessage);
}

bool Engine::bindCtcpReply(const char *command, QObject *object, const char *member,
	int minArgs, int maxArgs, const QString &helpMessage)
{
	return _bind(d->ctcpReplies, command, object, member,
		minArgs, maxArgs, helpMessage);
}
*/
void Client::authentify()
{
	// If password is given for this server, send it now, and don't expect a reply
	const KURL &url = this->url();

	if (url.hasPass())
		StdCommands::pass(this, url.pass());

	#warning make the following string arguments static const
	StdCommands::user(this, url.user(), StdCommands::Normal, url.queryItem(URL_REALNAME));
	StdCommands::nick(this, url.queryItem(URL_NICKNAME));
}

/*
 * The ctcp commands seems to follow the same message behaviours has normal IRC command.
 * (Only missing the \n\r final characters)
 * So applying the same parsing rules to the messages.
 */
/*
bool Client::invokeCtcpCommandOfMessage(const QMap<QString, MessageRedirector *> &map, Message &msg)
{
//	appendMessage( i18n("CTCP %1 REPLY: %2").arg(type).arg(messageReceived) );

	if(msg.hasCtcpMessage() && msg.ctcpMessage().isValid())
	{
		Message &ctcpMsg = msg.ctcpMessage();

		MessageRedirector *mr = map[ctcpMsg.command()];
		if (mr)
		{
			QStringList errors = mr->operator()(msg);

			if (errors.isEmpty())
				return true;

//			kdDebug(14120) << "Method error for line:" << ctcpMsg.raw();
//			writeCtcpErrorMessage(msg.prefix(), msg.ctcpRaw(),
//				QString::fromLatin1("%1 internal error(s)").arg(errors.size()));
		}
		else
		{
//			kdDebug(14120) << "Unknow IRC/CTCP command for line:" << ctcpMsg.raw();
//			writeCtcpErrorMessage(msg.prefix(), msg.ctcpRaw(), "Unknown CTCP command");

//			emit incomingUnknownCtcp(msg.ctcpRaw());
		}
	}
	else
	{
//		kdDebug(14120) << "Message do not embed a CTCP message:" << msg.raw();
	}
	return false;
}
*/
Entity::Ptr Client::server()
{
	return d->server;
}

ClientCommandHandler *Client::clientCommandHandler()
{
//	return dynamic_cast<ClientCommandHandler *>(commandHandler());
	return 0;
}

