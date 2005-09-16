/*
    kircengine.cpp - IRC Client

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

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qtextcodec.h>
#include <qtimer.h>

//Needed for getuid / getpwuid
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

using namespace KIRC;

class KIRC::ClientPrivate
{
public:
	QTextCodec *defaultCodec;

	QString host;
	Q_UINT16 port;

//	QUrl serverURL;
//	QUrl currentServerURL;
	QString nickname;
	QString username;
	QString realName;
	QString password;
	bool reqsPassword;
	bool failedNickOnLogin;
	bool useSSL;

//	KIRC::EntityPtrList entities;
	KIRC::EntityPtr server;
//	KIRC::EntityPtr self; => owner

	QString versionString;
	QString userString;
	QString sourceString;
	QString pendingNick;

//	QMap<QString, KIRC::MessageRedirector *> commands;
//	QMap<int, KIRC::MessageRedirector *> numericCommands;
//	QMap<QString, KIRC::MessageRedirector *> ctcpQueries;
//	QMap<QString, KIRC::MessageRedirector *> ctcpReplies;
};

Client::Client(QObject *parent)
	: KIRC::Socket(parent),
	  d( new ClientPrivate )
{
/*
	  d->defaultCodec(UTF8),
	  d->FailedNickOnLogin(false),
	  d->useSSL(false),
	  d->server(new Entity(QString::null, KIRC::Server)),
	  d->self(new Entity(QString::null, KIRC::User))
*/
	setUserName(QString::null);

//	d->entities << d->server << d->self;

//	bindCommands();
//	bindNumericReplies();
//	bindCtcp();

	d->versionString = QString::fromLatin1("Anonymous client using the KIRC engine.");
	d->userString = QString::fromLatin1("Response not supplied by user.");
	d->sourceString = QString::fromLatin1("Unknown client, known source.");

//	kdDebug(14120) << "Setting default engine codec, " << defaultCodec->name() << endl;
/*
	connect(this, SIGNAL(internalError(const QString &)),
		this, SLOT());
*/
	connect(this, SIGNAL(connectionStateChanged(KIRC::ConnectionState)),
		this, SLOT(onConnectionStateChanged(KIRC::ConnectionState)));

	connect(this, SIGNAL(receivedMessage(KIRC::Message &)),
		this, SLOT(onReceivedMessage(KIRC::Message &)));
}

Client::~Client()
{
//	kdDebug(14120) << k_funcinfo << d->Host << endl;
	quit("KIRC Deleted", true);
}

bool Client::isDisconnected() const
{
	return connectionState() == Idle;
}

bool Client::isConnected() const
{
	return connectionState() == Connected;
}

void Client::setVersionString(const QString &newString)
{
	d->versionString = newString;
}

void Client::setUserString(const QString &newString)
{
	d->userString = newString;
}

void Client::setSourceString(const QString &newString)
{
	d->sourceString = newString;
}

void Client::setUserName(const QString &newName)
{
	if(newName.isEmpty())
		d->username = QString::fromLatin1(getpwuid(getuid())->pw_name);
	else
		d->username = newName;
}

void Client::setRealName(const QString &newName)
{
	if(newName.isEmpty())
		d->realName = QString::fromLatin1(getpwuid(getuid())->pw_gecos);
	else
		d->realName = newName;
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
void Client::onConnectionStateChanged(KIRC::ConnectionState state)
{
	switch (state)
	{
	case Authentifying:
		// If password is given for this server, send it now, and don't expect a reply
		if (!(password()).isEmpty())
			pass(password());

		user(d->username, 0, d->realName);
		nick(d->nickname);

		break;
	default:
		// Do nothing for state
		break;
	}
}

void Client::onReceivedMessage( KIRC::Message &msg )
{
/*
	KIRC::MessageRedirector *mr;
	QStringList errors;

	if (msg.isNumeric())
	{
		if (d->FailedNickOnLogin)
		{
			// this is if we had a "Nickname in use" message when connecting and we set another nick.
			// This signal emits that the nick was accepted and we are now logged in
//			emit successfullyChangedNick(d->Nickname, d->PendingNick);
			d->Nickname = d->PendingNick;
			d->FailedNickOnLogin = false;
		}
//		mr = d->numericCommands[ msg.command().toInt() ];
		// we do this conversion because some dummy servers sends 1 instead of 001
		// numbers are stored as "1" instead of "001" to make convertion faster (no 0 pading).
		mr = d->commands[ QString::number(msg.command().toInt()) ];
	}
	else
		mr = d->commands[ msg.command() ];

	if (mr)
	{
//		errors = mr->operator()(msg);
	}
	else if (msg.isNumeric())
	{
//		kdWarning(14120) << "Unknown IRC numeric reply for line:" << msg.raw() << endl;
//		emit incomingUnknown(msg.raw());
	}
	else
	{
//		kdWarning(14120) << "Unknown IRC command for line:" << msg.raw() << endl;
//		emit internalError(UnknownCommand, msg);
	}

	if (!errors.isEmpty())
	{
//		kdDebug(14120) << "Method error for line:" << msg.raw() << endl;
//		emit internalError(MethodFailed, msg);
	}
*/
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
Entity *Client::server()
{
	return d->server;
}

void Client::ignoreMessage(KIRC::Message &/*msg*/)
{
}

void Client::receivedServerMessage(KIRC::Message &msg)
{
	receivedServerMessage(msg, msg.suffix());
}

void Client::receivedServerMessage(KIRC::Message &msg, const QString &message)
{
//	emit receivedMessage(InfoMessage, msg.prefix(), EntityPtrList(), message);
}

