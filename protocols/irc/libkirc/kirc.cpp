/*
    kirc.cpp - IRC Client

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>
    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

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

//Needed for getuid / getpwuid
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <qtextcodec.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kextsock.h>

#include "kircfunctors.h"
#include "ksslsocket.h"

#include "kirc.h"

/* Please note that the regular expression "[\\r\\n]*$" is used in a QString::replace statement many times.
 * This gets rid of trailing \r\n, \r, \n, and \n\r characters.
 */
const QRegExp KIRC::m_RemoveLinefeeds( QString::fromLatin1("[\\r\\n]*$") );

KIRCMethodFunctorCall *KIRC::IgnoreMethod( new KIRCMethodFunctor_Ignore() );

KIRC::KIRC( QObject *parent, const char *name) : QObject( parent, name ),
	  m_status(Disconnected),
	  m_useSSL(false),
	  m_IrcMethods(17, false),
	  m_IrcNumericMethods(101),
	  m_IrcCTCPQueryMethods(17, false),
	  m_IrcCTCPReplyMethods(17, false),
	  codecs(577,false)
{
	m_IrcMethods.setAutoDelete(true);
	m_IrcCTCPQueryMethods.setAutoDelete(true);
	m_IrcCTCPReplyMethods.setAutoDelete(true);

	setUserName(QString::null);

	registerCommands();
	registerNumericReplies();
	registerCtcp();

	m_VersionString = QString::fromLatin1("Anonymous client using the KIRC engine.");
	m_UserString = QString::fromLatin1("Response not supplied by user.");
	m_SourceString = QString::fromLatin1("Unknown client, known source.");

	defaultCodec = QTextCodec::codecForMib(4);
	kdDebug(14120) << "Setting defualt engine codec, " << defaultCodec->name() << endl;

	m_sock = 0L;

	connectTimeout = 20000;
	QString timeoutPath = locate( "config", "kioslaverc" );
	if( !timeoutPath.isEmpty() )
	{
		KConfig config( timeoutPath );
		connectTimeout = config.readNumEntry( "ConnectTimeout", 20 ) * 1000;
	}
}

KIRC::~KIRC()
{
	kdDebug(14120) << k_funcinfo << m_Host << endl;
	quitIRC("KIRC Deleted", true);
	if( m_sock )
		delete m_sock;
}

void KIRC::setUseSSL( bool useSSL )
{
	kdDebug(14120) << k_funcinfo << useSSL << endl;

	if( !m_sock || useSSL != m_useSSL )
	{
		if( m_sock )
			delete m_sock;

		m_useSSL = useSSL;

		if( m_useSSL )
		{
			m_sock = new KSSLSocket;
			m_sock->setSocketFlags( KExtendedSocket::inetSocket );
		}
		else
		{
			m_sock = new KExtendedSocket;
			m_sock->setSocketFlags( KExtendedSocket::inputBufferedSocket | KExtendedSocket::inetSocket );
		}

		QObject::connect(m_sock, SIGNAL(closed(int)), this, SLOT(slotConnectionClosed()));
		QObject::connect(m_sock, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
		QObject::connect(m_sock, SIGNAL(connectionSuccess()), this, SLOT(slotConnected()));
		QObject::connect(m_sock, SIGNAL(connectionFailed(int)), this, SLOT(error(int)));
	}
}

void KIRC::setStatus(EngineStatus status)
{
	kdDebug(14120) << k_funcinfo << status << endl;
	if( status == Disconnected && m_status != Closing )
	{
		emit disconnected();
	}

	m_status = status;
	emit statusChanged(status);
}

void KIRC::connectToServer(const QString &host, Q_UINT16 port, const QString &nickname, bool useSSL )
{
	setUseSSL(useSSL);

	m_Nickname = nickname;
	m_Host = host;
	m_Port = port;

	kdDebug(14120) << "Trying to connect to server " << m_Host << ":" << m_Port << endl;
	kdDebug(14120) << "Sock status: " << m_sock->socketStatus() << endl;

	if( !m_sock->setAddress(m_Host, m_Port) )
		kdDebug(14120) << k_funcinfo << "setAddress failed. Status:  " << m_sock->socketStatus() << endl;

	if( m_sock->lookup() ) // necessary to avoid QDns
		kdDebug(14120) << k_funcinfo << "lookup() failed. Status: " << m_sock->socketStatus() << endl;

	if( m_sock->startAsyncConnect() == 0 )
	{
		kdDebug(14120) << k_funcinfo << "Success!. Status: " << m_sock->socketStatus() << endl;
		setStatus(Connecting);

		//If we don't get a reply within 15 seconds, give up
		QTimer::singleShot(connectTimeout, this, SLOT(slotAuthFailed()));
	}
	else
	{
		kdDebug(14120) << k_funcinfo << "Failed. Status: " << m_sock->socketStatus() << endl;
		setStatus(Disconnected);
	}
}

void KIRC::slotAuthFailed()
{
	kdDebug(14120) << k_funcinfo << endl;
	if( m_status != Connected )
	{
		setStatus(Disconnected);
		m_sock->close();
		m_sock->reset();
		emit connectionTimeout();
	}
}

void KIRC::slotConnected()
{
	kdDebug(14120) << k_funcinfo << "Connected" << endl;
	setStatus(Authentifying);
	m_sock->enableRead(true);

	// If password is given for this server, send it now, and don't expect a reply
	if (!(password()).isEmpty())
		writeMessage("PASS", password() , m_Realname, false);

	changeUser(m_Username, 0, QString::fromLatin1("Kopete User"));
	changeNickname(m_Nickname);

	//If we don't get a reply within 15 seconds, give up
	QTimer::singleShot(connectTimeout, this, SLOT(slotAuthFailed()));
}

void KIRC::slotConnectionClosed()
{
	kdDebug(14120) << k_funcinfo << "Connection Closed - local status: " << m_status << " sock status: " << m_sock->socketStatus() << endl;
	if(m_status == Closing)
		emit successfulQuit();

	if(m_status!=Disconnected)
		setStatus(Disconnected);
	m_sock->reset();
}

void KIRC::error(int errCode)
{
	kdDebug(14120) << k_funcinfo << "Socket error: " << errCode << endl;
	if (m_sock->socketStatus () != KExtendedSocket::connecting)
	{
		// Connection in progress.. This is a signal fired wrong
		setStatus(Disconnected);
		m_sock->reset();
	}
}

void KIRC::setVersionString(const QString &newString)
{
	m_VersionString = newString;
	m_VersionString.remove(m_RemoveLinefeeds);
}

void KIRC::setUserString(const QString &newString)
{
	m_UserString = newString;
	m_UserString.remove(m_RemoveLinefeeds);
}

void KIRC::setSourceString(const QString &newString)
{
	m_SourceString = newString;
	m_SourceString.remove(m_RemoveLinefeeds);
}

void KIRC::setUserName(const QString &newName)
{
	if(newName.isEmpty())
		m_Username = QString::fromLatin1(getpwuid(getuid())->pw_name);
	else
		m_Username = newName;
	m_Username.remove(m_RemoveLinefeeds);
}

void KIRC::addIrcMethod(QDict<KIRCMethodFunctorCall> &map, const char *str, KIRCMethodFunctorCall *method)
{
	map.replace( QString::fromLatin1(str), method);
}

void KIRC::addIrcMethod(QDict<KIRCMethodFunctorCall> &map, const char *str,
			bool (KIRC::*method)(const KIRCMessage &msg),
			int argsSize_min, int argsSize_max, const char *helpMessage)
{
	addIrcMethod(map, str, new KIRCMethodFunctor_Forward<KIRC>(this, method, argsSize_min, argsSize_max, helpMessage));
}

void KIRC::addNumericIrcMethod(int id, KIRCMethodFunctorCall *method)
{
	m_IrcNumericMethods.replace(id, method);
}

void KIRC::addNumericIrcMethod(int id, bool (KIRC::*method)(const KIRCMessage &msg),
			int argsSize_min, int argsSize_max, const char *helpMessage)
{
	addNumericIrcMethod(id, new KIRCMethodFunctor_Forward<KIRC>(this, method, argsSize_min, argsSize_max, helpMessage) );
}

bool KIRC::canSend( bool mustBeConnected ) const
{
	return	( !mustBeConnected || m_status == Connected );
}

/* Message will be send as passed.
 */
KIRCMessage KIRC::writeRawMessage(const QString &rawMsg, bool mustBeConnected)
{
	if(canSend(mustBeConnected))
	{
		KIRCMessage ircmsg = KIRCMessage::writeRawMessage(this, defaultCodec, rawMsg);
		emit sentMessage(ircmsg);
		return ircmsg;
	}

//	kdDebug(14120) << "Must be connected error:" << rawMsg << endl;
	kdDebug(14120) << k_funcinfo << "Must be connected error:" << rawMsg << endl;

	return KIRCMessage();
}

/* Message will be quoted before beeing send.
 */
KIRCMessage KIRC::writeMessage(const QString &msg, bool mustBeConnected)
{
	if(canSend(mustBeConnected))
	{
		KIRCMessage ircmsg = KIRCMessage::writeMessage(this, defaultCodec, msg);
		emit sentMessage(ircmsg);
		return ircmsg;
	}

//	kdDebug(14120) << "Must be connected error:" << msg << endl;
	kdDebug(14120) << k_funcinfo << "Must be connected error:" << msg << endl;

	return KIRCMessage();
}

KIRCMessage KIRC::writeMessage(const QString &command, const QStringList &args, const QString &suffix, bool mustBeConnected)
{
	if(canSend(mustBeConnected))
	{
		KIRCMessage ircmsg = KIRCMessage::writeMessage(this, defaultCodec,
			command, args, suffix );
		emit sentMessage(ircmsg);
		return ircmsg;
	}

//	kdDebug(14120) << "Must be connected error:" << command << args.join(' ') << suffix << endl;
	kdDebug(14120) << k_funcinfo << "Must be connected error:" << command << args.join(" ") << suffix << endl;

	return KIRCMessage();
}

KIRCMessage KIRC::writeCtcpMessage(const QString &command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix, bool emitRepliedCtcp)
{
	QString nick =  KIRCEntity::userNick(to);

	KIRCMessage msg = KIRCMessage::writeCtcpMessage(this, codecForNick( nick ),
		command, nick, suffix,
		ctcpCommand, ctcpArgs, ctcpSuffix );

	emit sentMessage(msg);

	if(emitRepliedCtcp && msg.isValid() && msg.hasCtcpMessage())
		emit repliedCtcp(msg.ctcpMessage().command(), msg.ctcpMessage().ctcpRaw());

	return msg;
}

void KIRC::slotReadyRead()
{
	// This condition is buggy when the peer server
	// close the socket unexpectedly
	bool parseSuccess;

	if( m_sock->socketStatus() == KExtendedSocket::connected && m_sock->canReadLine())
	{
		KIRCMessage msg = KIRCMessage::parse(this, defaultCodec, &parseSuccess);
		if(parseSuccess)
		{
			KIRCMethodFunctorCall *method;
			if( msg.isNumeric() )
				method = m_IrcNumericMethods[ msg.command().toInt() ];
			else
				method = m_IrcMethods[ msg.command() ];

			if(method && method->isValid())
			{
				if (method->checkMsgValidity(msg) &&
					(!msg.isNumeric() ||
					 (/*msg.isNumeric()&&*/msg.argsSize() > 0 && msg.arg(0) == m_Nickname || msg.arg(0) == "*")))
				{
					emit receivedMessage(msg);
					if (!method->operator()(msg))
					{
						kdDebug(14120) << "Method error for line:" << msg.raw() << endl;
						emit internalError(MethodFailed, msg);
					}
				}
				else
				{
					kdDebug(14120) << "Args are invalid for line:" << msg.raw() << endl;
					emit internalError(InvalidNumberOfArguments, msg);
				}
			}
			else if (msg.isNumeric())
			{
				kdDebug(14120) << "Unknown IRC numeric reply for line:" << msg.raw() << endl;
				emit internalError(UnknownNumericReply, msg);
			}
			else
			{
				kdDebug(14120) << "Unknown IRC command for line:" << msg.raw() << endl;
				emit internalError(UnknownCommand, msg);
			}
		}
		else
		{
			emit incomingUnknown(msg.raw());
			emit internalError(ParsingFailed, msg);
		}

		QTimer::singleShot( 0, this, SLOT( slotReadyRead() ) );
	}

	if(m_sock->socketStatus()!=KExtendedSocket::connected)
		error();
}

const QTextCodec *KIRC::codecForNick( const QString &nick ) const
{
	if( nick.isEmpty() )
		return defaultCodec;

	QTextCodec *codec = codecs[ nick ];
	kdDebug(14120) << nick << " has codec " << codec << endl;

	if( !codec )
		return defaultCodec;
	else
		return codec;
}

void KIRC::showInfoDialog()
{
	if( m_useSSL )
	{
		static_cast<KSSLSocket*>( m_sock )->showInfoDialog();
	}
}

/*
 * The ctcp commands seems to follow the same message behaviours has normal IRC command.
 * (Only missing the \n\r final characters)
 * So applying the same parsing rules to the messages.
 */
bool KIRC::invokeCtcpCommandOfMessage(const KIRCMessage &msg, const QDict<KIRCMethodFunctorCall> &map)
{
	if(msg.hasCtcpMessage() && msg.ctcpMessage().isValid())
	{
		const KIRCMessage &ctcpMsg = msg.ctcpMessage();

		KIRCMethodFunctorCall *method = map[ctcpMsg.command()];
		if(method && method->isValid())
		{
			if (method->checkMsgValidity(ctcpMsg))
			{
				if(method->operator()(msg))
				{
					return true;
				}
				else
				{
					kdDebug(14120) << "Method error for line:" << ctcpMsg.raw();
					writeCtcpErrorMessage(msg.prefix(), msg.ctcpRaw(), "Internal error");
				}
			}
			else
			{
				kdDebug(14120) << "Args are invalid for line:" << ctcpMsg.raw();
				writeCtcpErrorMessage(msg.prefix(), msg.ctcpRaw(), "Invalid number of arguments");
			}
		}
		else
		{
			kdDebug(14120) << "Unknow IRC/CTCP command for line:" << ctcpMsg.raw();
			writeCtcpErrorMessage(msg.prefix(), msg.ctcpRaw(), "Unknown CTCP command");

			emit incomingUnknownCtcp(msg.ctcpRaw());
		}
	}
	else
	{
		kdDebug(14120) << "Message do not embed a CTCP message:" << msg.raw();
	}
	return false;
}

#include "kirc.moc"

