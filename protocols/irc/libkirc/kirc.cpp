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
const QRegExp KIRC::isChannelRegex( QString::fromLatin1("^[#!+&][^\\s,:]+$") );
KIRCMethodFunctorCall *KIRC::IgnoreMethod( new KIRCMethodFunctor_Ignore() );

KIRC::KIRC( QObject *parent, const char *name) : QObject( parent, name ),
	  m_status(Disconnected),
	  m_useSSL(false),
	  m_IrcMethods(17, false),
	  m_IrcNumericMethods(101),
	  m_IrcCTCPQueryMethods(17, false),
	  m_IrcCTCPReplyMethods(17, false),
	  codecs( QDict<QTextCodec>(577,false) )
{
	m_IrcMethods.setAutoDelete(true);
	m_IrcCTCPQueryMethods.setAutoDelete(true);
	m_IrcCTCPReplyMethods.setAutoDelete(true);

	setUserName(QString::null);

//	The following order is based on the RFC2812.

//	Connection Registration
	addIrcMethod("NICK",	&KIRC::nickChange,	0,	0);
	addIrcMethod("QUIT",	new KIRCMethodFunctor_SS_PrefixSuffix<KIRC>(this, &KIRC::incomingQuitIRC,	0,	0));
//	addIrcMethod("SQUIT",	new KIRCMethodFunctor_SS_PrefixSuffix<KIRC>(this, &KIRC::incomingServerQuitIRC,	1,	1));

//	Channel operations
	addIrcMethod("JOIN",	&KIRC::joinChannel,	0,	1);
	addIrcMethod("PART",	&KIRC::partChannel,	1,	1);
	addIrcMethod("MODE",	&KIRC::modeChange,	1,	1);
	addIrcMethod("TOPIC",	&KIRC::topicChange,	1,	1);
	addIrcMethod("KICK",	&KIRC::kick,		2,	2);

//	Sending messages
	addIrcMethod("PRIVMSG",	&KIRC::privateMessage,	1,	1);
	addIrcMethod("NOTICE",	&KIRC::notice,		1,	1);

//	Miscellaneous messages
	addIrcMethod("PING",	&KIRC::ping,	0,	0);
	addIrcMethod("PONG",	&KIRC::pong,	0,	0);

	registerNumericReplies();
	registerCtcpMethods();

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
		KIRCMessage ircmsg = KIRCMessage::writeRawMessage(this, rawMsg, defaultCodec);
		emit sentMessage(ircmsg);
		return ircmsg;
	}

	kdDebug(14120) << "Must be connected error:" << rawMsg << endl;

	return KIRCMessage();
}

/* Message will be quoted before behing send.
 */
KIRCMessage KIRC::writeMessage(const QString &msg, bool mustBeConnected)
{
	if(canSend(mustBeConnected))
	{
		KIRCMessage ircmsg = KIRCMessage::writeMessage(this, msg, defaultCodec);
		emit sentMessage(ircmsg);
		return ircmsg;
	}

	kdDebug(14120) << "Must be connected error:" << msg << endl;

	return KIRCMessage();
}

KIRCMessage KIRC::writeMessage(const QString &command, const QString &arg, const QString &suffix, bool mustBeConnected)
{
	if(canSend(mustBeConnected))
	{
		KIRCMessage ircmsg = KIRCMessage::writeMessage(this, command, arg, suffix, codecForNick( arg ) );
		emit sentMessage(ircmsg);
		return ircmsg;
	}

	kdDebug(14120) << "Must be connected error:" << command << arg << suffix << endl;

	return KIRCMessage();
}

KIRCMessage KIRC::writeCtcpMessage(const QString &command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QString &ctcpArg, const QString &ctcpSuffix, bool emitRepliedCtcp)
{
	QString nick =  KIRCMessage::nickFromPrefix(to);

	KIRCMessage msg = KIRCMessage::writeCtcpMessage(this, command, nick, suffix, ctcpCommand,
		codecForNick( nick ), ctcpArg, ctcpSuffix );

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

void KIRC::changeNickname(const QString &newNickname)
{
	m_PendingNick = newNickname;
	writeMessage("NICK", newNickname, QString::null, false);
}

bool KIRC::nickChange(const KIRCMessage &msg)
{
	/* Nick name of a user changed
	 * "<nickname>" */
	QString oldNick = msg.prefix().section('!', 0, 0);
	QString newNick = msg.suffix();

	if( codecs[ oldNick ] )
	{
		QTextCodec *c = codecs[ oldNick ];
		codecs.remove( oldNick );
		codecs.insert( newNick, c );
	}

	if (oldNick.lower() == m_Nickname.lower())
	{
		emit successfullyChangedNick(oldNick, msg.suffix());
		m_Nickname = msg.suffix();
	}
	else
		emit incomingNickChange(oldNick, msg.suffix());

	return true;
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

void KIRC::changeUser(const QString &newUsername, const QString &hostname, const QString &newRealname)
{
	/* RFC1459: "<username> <hostname> <servername> <realname>"
	 * The USER command is used at the beginning of connection to specify
	 * the username, hostname and realname of a new user.
	 * hostname is usualy set to "127.0.0.1" */
	m_Username = newUsername;
	m_Realname = newRealname;

	writeMessage("USER", join( m_Username, hostname, m_Host ), m_Realname, false);
}

void KIRC::changeUser(const QString &newUsername, Q_UINT8 mode, const QString &newRealname)
{
	/* RFC2812: "<user> <mode> <unused> <realname>"
	 * mode is a numeric value (from a bit mask).
	 * 0x00 normal
	 * 0x04 request +w
	 * 0x08 request +i */
	m_Username = newUsername;
	m_Realname = newRealname;

	writeMessage("USER", join(m_Username, QString::number(mode), QChar('*') ), m_Realname, false);
}

void KIRC::showInfoDialog()
{
	if( m_useSSL )
	{
		static_cast<KSSLSocket*>( m_sock )->showInfoDialog();
	}
}

void KIRC::quitIRC(const QString &reason, bool now)
{
	kdDebug(14120) << k_funcinfo << endl;
	if( now || !canSend(true) )
	{
		setStatus(Disconnected);
		m_sock->close();
		m_sock->reset();
	}
	else
	{
		writeMessage("QUIT", QString::null, reason, false);
		setStatus(Closing);
		QTimer::singleShot(5000, this, SLOT(quitTimeout()));
	}
}

void KIRC::quitTimeout()
{
	if(	m_sock->socketStatus() > KExtendedSocket::nothing &&
		m_sock->socketStatus() < KExtendedSocket::done &&
		m_status == Closing)
	{
		setStatus(Disconnected);
		m_sock->close();
		m_sock->reset();
	}
}

bool KIRC::quitIRC(const KIRCMessage &msg)
{
	/* This signal emits when a user quits irc.
	 */
	kdDebug(14120) << "User quiting" << endl;
	emit incomingQuitIRC(msg.prefix(), msg.suffix());
	return true;
}

void KIRC::joinChannel(const QString &name, const QString &key)
{
	if ( !key.isNull() )
		writeMessage("JOIN", join( name, key ) );
	else
		writeMessage("JOIN", name);
}

bool KIRC::joinChannel(const KIRCMessage &msg)
{
	/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
	 * suspected: ":<channel> *(" "/"," <channel>)"
	 * assumed ":<channel>"
	 * This is the response of someone joining a channel.
	 * Remember that this will be emitted when *you* /join a room for the first time */

	if (msg.argsSize()==1)
		emit incomingJoinedChannel(msg.arg(0), msg.nickFromPrefix());
	else
		emit incomingJoinedChannel(msg.suffix(), msg.nickFromPrefix());

	return true;
}

void KIRC::partChannel(const QString &channel, const QString &reason)
{
	/* This will part a channel with 'reason' as the reason for parting
	 */
	writeMessage("PART", channel, reason);
}

bool KIRC::partChannel(const KIRCMessage &msg)
{
	/* This signal emits when a user parts a channel
	 * "<channel> *( "," <channel> ) [ <Part Message> ]"
	 */
	kdDebug(14120) << "User parting" << endl;
	emit incomingPartedChannel(msg.arg(0), msg.nickFromPrefix(), msg.suffix());
	return true;
}

void KIRC::changeMode(const QString &target, const QString &mode)
{
	writeMessage("MODE", join( target, mode ) );
}

bool KIRC::modeChange(const KIRCMessage &msg)
{
	/* Change the mode of a user.
	 * "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
	 */
	QStringList args = msg.args();
	args.pop_front();
	if( isChannel( msg.arg(0) ) )
		emit incomingChannelModeChange( msg.arg(0), msg.nickFromPrefix(), args.join(" "));
	else
		emit incomingUserModeChange( msg.nickFromPrefix(), args.join(" "));
	return true;
}

void KIRC::setTopic(const QString &channel, const QString &topic)
{
	writeMessage("TOPIC", channel, topic);
}

bool KIRC::topicChange(const KIRCMessage &msg)
{
	/* The topic of a channel changed. emit the channel, new topic, and the person who changed it.
	 * "<channel> [ <topic> ]"
	 */
	emit incomingTopicChange(msg.arg(0), msg.nickFromPrefix(), msg.suffix());
	return true;
}

void KIRC::list()
{
	writeMessage("LIST", QString::null);
}

void KIRC::motd(const QString &server)
{
	writeMessage("MOTD", server);
}

void KIRC::kickUser(const QString &user, const QString &channel, const QString &reason)
{
	writeMessage("KICK", join( channel, user, reason ) );
}

bool KIRC::kick(const KIRCMessage &msg)
{
	/* The given user is kicked.
	 * "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
	 */
	emit incomingKick( msg.arg(0), msg.nickFromPrefix(), msg.arg(1), msg.suffix());
	return true;
}

//void KIRC::sendPrivateMessage(const QString &contact, const QString &message)
void KIRC::messageContact(const QString &contact, const QString &message)
{
	writeMessage("PRIVMSG", contact, message);
}

bool KIRC::privateMessage(const KIRCMessage &msg)
{
	/* This is a signal that indicates there is a new message.
	 * This can be either from a channel or from a specific user. */
	KIRCMessage m = msg;
	if (!m.suffix().isEmpty())
	{
		QString user = m.arg(0);
		m = KIRCMessage::parse( codecForNick( user )->toUnicode( m.raw() ) );

		QString message = m.suffix();

		if( isChannel(user) )
			emit incomingMessage(msg.nickFromPrefix(), msg.arg(0), message );
		else
			emit incomingPrivMessage(msg.nickFromPrefix(), msg.arg(0), message );
	}

	if( msg.hasCtcpMessage() )
	{
		invokeCtcpCommandOfMessage(msg, m_IrcCTCPQueryMethods);
	}

	return true;
}

void KIRC::sendNotice(const QString &target, const QString &message)
{
	writeMessage("NOTICE", target, message);
}

bool KIRC::notice(const KIRCMessage &msg)
{
	if(!msg.suffix().isEmpty())
		emit incomingNotice(msg.prefix(), msg.suffix());

	if(msg.hasCtcpMessage())
		invokeCtcpCommandOfMessage(msg, m_IrcCTCPReplyMethods);

	return true;
}

void KIRC::whoisUser(const QString &user)
{
	writeMessage("WHOIS", user);
}

bool KIRC::ping(const KIRCMessage &msg)
{
	writeMessage("PONG", msg.arg(0), msg.suffix(), false);
	return true;
}

bool KIRC::pong(const KIRCMessage &/*msg*/)
{
	return true;
}

void KIRC::setAway(bool isAway, const QString &awayMessage)
{
	if(isAway)
		if( !awayMessage.isEmpty() )
			writeMessage("AWAY", QString::null, awayMessage);
		else
			writeMessage("AWAY", QString::null, QString::fromLatin1("I'm away."));
	else
		writeMessage("AWAY", QString::null);
}

void KIRC::isOn(const QStringList &nickList)
{
	if (!nickList.isEmpty())
	{
		QString statement = QString::fromLatin1("ISON");
		for (QStringList::ConstIterator it = nickList.begin(); it != nickList.end(); ++it)
		{
			if ((statement.length()+(*it).length())>509) // 512(max buf)-2("\r\n")-1(<space separator>)
			{
				writeMessage(statement);
				statement = QString::fromLatin1("ISON ") +  (*it);
			}
			else
				statement.append(QChar(' ') + (*it));
		}
		writeMessage(statement);
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

bool KIRC::CtcpReply_errorMsg(const KIRCMessage &)
{
	// should emit one signal
	return true;
}

#include "kirc.moc"

