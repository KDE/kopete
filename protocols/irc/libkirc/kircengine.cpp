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

#include "kircengine.h"
#include "ksslsocket.h"

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
/*
#ifndef KIRC_SSL_SUPPORT
#define KIRC_SSL_SUPPORT
#endif
*/
using namespace KIRC;
using namespace KNetwork;

// FIXME: Remove slotConnected() and error(int errCode) while going to KNetwork namespace

/* Please note that the regular expression "[\\r\\n]*$" is used in a QString::replace statement many times.
 * This gets rid of trailing \r\n, \r, \n, and \n\r characters.
 */
const QRegExp Engine::m_RemoveLinefeeds( QString::fromLatin1("[\\r\\n]*$") );

Engine::Engine(QObject *parent, const char *name)
	: QObject(parent, QString::fromLatin1("[KIRC::Engine]%1").arg(name).latin1()),
	  m_socket(0),
	  m_defaultCodec(QTextCodec::codecForMib(4)),
	  m_status(Idle),
	  m_FailedNickOnLogin(false),
	  m_useSSL(false),
	  m_server(new Entity(QString::null, Entity::Server)),
	  m_self(new Entity(QString::null, Entity::User)),
	  m_commands(101, false),
//	  m_numericCommands(101),
	  m_ctcpQueries(17, false),
	  m_ctcpReplies(17, false)
{
	setUserName(QString::null);

	m_entities << m_server << m_self;

	m_commands.setAutoDelete(true);
	m_ctcpQueries.setAutoDelete(true);
	m_ctcpReplies.setAutoDelete(true);

	bindCommands();
	bindNumericReplies();
	bindCtcp();

	m_VersionString = QString::fromLatin1("Anonymous client using the KIRC engine.");
	m_UserString = QString::fromLatin1("Response not supplied by user.");
	m_SourceString = QString::fromLatin1("Unknown client, known source.");

//	kdDebug(14120) << "Setting default engine codec, " << defaultCodec->name() << endl;
}

Engine::~Engine()
{
	kdDebug(14120) << k_funcinfo << m_Host << endl;
	quit("KIRC Deleted", true);
	if (m_socket)
		delete m_socket;
}

//bool Engine::setupSocket(bool useSSL)
void Engine::setUseSSL(bool useSSL)
{
	kdDebug(14120) << k_funcinfo << useSSL << endl;

	if (m_socket)
		delete m_socket;

	if( !m_socket || useSSL != m_useSSL )
	{
		m_useSSL = useSSL;

		if( m_useSSL )
		{
		#ifdef KIRC_SSL_SUPPORT
			m_socket = new KSSLSocket(this);
			m_socket->setSocketFlags( KExtendedSocket::inetSocket );
		}
		else
		#else
			kdWarning(14120) << "You tried to use SSL, but this version of Kopete was"
				" not compiled with IRC SSL support. A normal IRC connection will be attempted." << endl;
		}
		#endif
		{
			m_socket = new KBufferedSocket(QString::null, QString::null, this);
//			m_socket->setSocketFlags( KExtendedSocket::inputBufferedSocket | KExtendedSocket::inetSocket );
		}

		QObject::connect(m_socket, SIGNAL(closed(int)),
				 this, SLOT(slotConnectionClosed()));
		QObject::connect(m_socket, SIGNAL(readyRead()),
				 this, SLOT(slotReadyRead()));
		QObject::connect(m_socket, SIGNAL(connectionSuccess()),
				 this, SLOT(slotConnected()));
		QObject::connect(m_socket, SIGNAL(connectionFailed(int)),
				 this, SLOT(error(int)));
	}
}

void Engine::setStatus(Engine::Status status)
{
	kdDebug(14120) << k_funcinfo << status << endl;

	if (m_status == status)
		return;

//	Engine::Status oldStatus = m_status;
	m_status = status;
	emit statusChanged(status);

	switch (m_status)
	{
	case Idle:
		// Do nothing.
		break;
	case Connecting:
		// Do nothing.
		break;
	case Authentifying:
		m_socket->enableRead(true);

		// If password is given for this server, send it now, and don't expect a reply
		if (!(password()).isEmpty())
			pass(password());

		user(m_Username, 0, m_realName);
		nick(m_Nickname);

		break;
	case Connected:
		// Do nothing.
		break;
	case Closing:
		m_socket->close();
		m_socket->reset();
		setStatus(Idle);
		break;
	case AuthentifyingFailed:
		setStatus(Closing);
		break;
	case Timeout:
		setStatus(Closing);
		break;
	case Disconnected:
		setStatus(Closing);
		break;
	}
}

void Engine::connectToServer(const QString &host, Q_UINT16 port, const QString &nickname, bool useSSL )
{
	setUseSSL(useSSL);

	m_Nickname = nickname;
	m_Host = host;
	m_Port = port;

	kdDebug(14120) << "Trying to connect to server " << m_Host << ":" << m_Port << endl;
	kdDebug(14120) << "Sock status: " << m_socket->state() << endl;

	if (m_socket->connect(m_Host, QString::number(m_Port)))
	{
		kdDebug(14120) << k_funcinfo << "Success!. Status: " << m_socket->state() << endl;
		setStatus(Connecting);
	}
	else
	{
		kdDebug(14120) << k_funcinfo << "Failed. Status: " << m_socket->state() << endl;
		setStatus(Disconnected);
	}
}

void Engine::setDefaultCodec( QTextCodec* codec )
{
	m_defaultCodec = codec;
}

QTextCodec *Engine::defaultCodec() const
{
	return m_defaultCodec;
}

void Engine::slotConnected()
{
	setStatus(Authentifying);
}

void Engine::slotConnectionClosed()
{
	setStatus(Disconnected);
}

void Engine::error(int errCode)
{
	if (errCode == KSocketBase::WouldBlock)
		// ignore non-fatal error
		return;

	kdDebug(14120) << k_funcinfo << "Socket error: " << errCode << endl;

	setStatus(Disconnected);
}

void Engine::setVersionString(const QString &newString)
{
	m_VersionString = newString;
	m_VersionString.remove(m_RemoveLinefeeds);
}

void Engine::setUserString(const QString &newString)
{
	m_UserString = newString;
	m_UserString.remove(m_RemoveLinefeeds);
}

void Engine::setSourceString(const QString &newString)
{
	m_SourceString = newString;
	m_SourceString.remove(m_RemoveLinefeeds);
}

void Engine::setUserName(const QString &newName)
{
	if(newName.isEmpty())
		m_Username = QString::fromLatin1(getpwuid(getuid())->pw_name);
	else
		m_Username = newName;
	m_Username.remove(m_RemoveLinefeeds);
}

void Engine::setRealName(const QString &newName)
{
	if(newName.isEmpty())
		m_realName = QString::fromLatin1(getpwuid(getuid())->pw_gecos);
	else
		m_realName = newName;
	m_realName.remove(m_RemoveLinefeeds);
}

bool Engine::_bind(QDict<KIRC::MessageRedirector> &dict,
		QString command, QObject *object, const char *member,
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

bool Engine::bind(const QString &command, QObject *object, const char *member,
	int minArgs, int maxArgs, const QString &helpMessage)
{
	return _bind(m_commands, command, object, member,
		minArgs, maxArgs, helpMessage);
}

bool Engine::bind(int id, QObject *object, const char *member,
		int minArgs, int maxArgs, const QString &helpMessage)
{
	return _bind(m_commands, QString::number(id), object, member,
		     minArgs, maxArgs, helpMessage);
}

bool Engine::bindCtcpQuery(const QString &command, QObject *object, const char *member,
	int minArgs, int maxArgs, const QString &helpMessage)
{
	return _bind(m_ctcpQueries, command, object, member,
		minArgs, maxArgs, helpMessage);
}

bool Engine::bindCtcpReply(const QString &command, QObject *object, const char *member,
	int minArgs, int maxArgs, const QString &helpMessage)
{
	return _bind(m_ctcpReplies, command, object, member,
		minArgs, maxArgs, helpMessage);
}

/* Message will be send as passed.
 */
void Engine::writeRawMessage(const QString &rawMsg)
{
	Message::writeRawMessage(this, m_defaultCodec, rawMsg);
}

/* Message will be quoted before beeing send.
 */
void Engine::writeMessage(const QString &msg, const QTextCodec *codec)
{
	Message::writeMessage(this, codec ? codec : m_defaultCodec, msg);
}

void Engine::writeMessage(const QString &command, const QStringList &args, const QString &suffix, const QTextCodec *codec)
{
	Message::writeMessage(this, codec ? codec : m_defaultCodec, command, args, suffix );
}

void Engine::writeCtcpMessage(const QString &command, const QString &to, const QString &ctcpMessage)
{
	Message::writeCtcpMessage(this, m_defaultCodec, command, to, ctcpMessage);
}

void Engine::writeCtcpMessage(const QString &command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix, bool )
{
//	Message::writeCtcpMessage(this, nick, command, to, suffix,
//		ctcpCommand, ctcpArgs, ctcpSuffix );
}

void Engine::slotReadyRead()
{
	// This condition is buggy when the peer server
	// close the socket unexpectedly
	bool parseSuccess;

//	if (m_socket->state() == KSocketBase::Connected && m_socket->canReadLine())
	if (m_socket->canReadLine())
	{
		Message msg = Message::parse(this, &parseSuccess);
		if (parseSuccess)
		{
			emit receivedMessage(msg);

			KIRC::MessageRedirector *mr;
			if (msg.isNumeric())
//				mr = m_numericCommands[ msg.command().toInt() ];
				// we do this conversion because some dummy servers sends 1 instead of 001
				// numbers are stored as "1" instead of "001" to make convertion faster (no 0 pading).
				mr = m_commands[ QString::number(msg.command().toInt()) ];
			else
				mr = m_commands[ msg.command() ];

			if (mr)
			{
				QStringList errors = mr->operator()(msg);

				if (!errors.isEmpty())
				{
//					kdDebug(14120) << "Method error for line:" << msg.raw() << endl;
					emit internalError(MethodFailed, msg);
				}
			}
			else if (msg.isNumeric())
			{
//				kdWarning(14120) << "Unknown IRC numeric reply for line:" << msg.raw() << endl;
//				emit incomingUnknown(msg.raw());
			}
			else
			{
//				kdWarning(14120) << "Unknown IRC command for line:" << msg.raw() << endl;
				emit internalError(UnknownCommand, msg);
			}
		}
		else
		{
//			emit incomingUnknown(msg.raw());
			emit internalError(ParsingFailed, msg);
		}

		QTimer::singleShot( 0, this, SLOT( slotReadyRead() ) );
	}

//	if(m_socket->socketStatus() != KExtendedSocket::connected)
//		error();
}

void Engine::showInfoDialog()
{
/*	if( m_useSSL )
	{
		static_cast<KSSLSocket*>( m_socket )->showInfoDialog();
	}*/
}

/*
 * The ctcp commands seems to follow the same message behaviours has normal IRC command.
 * (Only missing the \n\r final characters)
 * So applying the same parsing rules to the messages.
 */
bool Engine::invokeCtcpCommandOfMessage(const QDict<MessageRedirector> &map, Message &msg)
{
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

EntityPtr Engine::getEntity(const QString &name)
{
	Entity *entity = 0;

	#warning Do the searching code here.

	if (!entity)
	{
		entity = new Entity(name);
		m_entities.append(entity);
	}

	connect(entity, SIGNAL(destroyed(KIRC::Entity *)),
		this, SLOT(destroyed(KIRC::Entity *)));
	return EntityPtr(entity);
}

EntityPtr Engine::server()
{
	return m_server;
}

EntityPtr Engine::self()
{
	return m_self;
}

void Engine::destroyed(KIRC::Entity *entity)
{
	m_entities.remove(entity);
}

void Engine::ignoreMessage(KIRC::Message &/*msg*/)
{
}

void Engine::receivedServerMessage(KIRC::Message &msg)
{
	receivedServerMessage(msg, msg.suffix());
}

void Engine::receivedServerMessage(KIRC::Message &msg, const QString &message)
{
//	emit receivedMessage(InfoMessage, msg.prefix(), EntityPtrList(), message);
}

#include "kircengine.moc"
