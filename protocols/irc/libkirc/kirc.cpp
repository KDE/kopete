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

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <unistd.h>
#include <pwd.h>

#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include "kircfunctors.h"

#include "kirc.h"


/* Please note that the regular expression "[\\r\\n]*$" is used in a QString::replace statement many times.
 * This gets rid of trailing \r\n, \r, \n, and \n\r characters.
 */
const QRegExp KIRC::m_RemoveLinefeeds(QString::fromLatin1("[\\r\\n]*$"));


KIRC::KIRC(const QString &host, const Q_UINT16 port, QObject *parent, const char *name)
	: QObject( parent, name ),
	  m_sock(host, port, KExtendedSocket::bufferedSocket | KExtendedSocket::inetSocket),
	  m_status(Disconnected),
	  m_IrcMethods(101, false),
	  m_IrcCTCPQueryMethods(17, false),
	  m_IrcCTCPReplyMethods(17, false),
	  codecs(577,false)
{
	m_IrcMethods.setAutoDelete(true);

	m_IrcCTCPQueryMethods.setAutoDelete(true);
	m_IrcCTCPReplyMethods.setAutoDelete(true);

	m_Host = host;
	m_Port = (port==0)?6667:port;
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

//	Server queries and commands

//	Service Query and Commands

//	User based queries
	//Whowas

//	Miscellaneous messages
	addIrcMethod("PING",	&KIRC::ping,	0,	0);
	addIrcMethod("PONG",	&KIRC::pong,	0,	0);
//	addIrcMethod("ERROR",	new KIRCMethodFunctor_SS_PrefixSuffix<KIRC>(this, &KIRC::incomingError,		0,	0));

//	Optional features
//	addIrcMethod("AWAY",	&KIRC::away,		0,	0));
	//Rehash message
	//Die message
	//Restart message
	//Summon message
	//Users
	//Operwall message
	//Userhost message
	//Ison message

//	Numeric Replies:
//	Note: Numeric replies always have the current nick or *(when undefined) as first argmuent.
	addIrcMethod("001",	&KIRC::numericReply_001,	1,	1);

	/* Gives information about the host, close to 004 (See case 004) in the form of:
	 * ":Your host is <servername>, running version <ver>" */
	addIrcMethod("002",	new KIRCMethodFunctor_S_Suffix<KIRC>(this, &KIRC::incomingConnectString,		1,	1));

	/* Gives the date that this server was created (useful for determining the uptime of the server) in the form of:
	 * "This server was created <date>" */
	addIrcMethod("003",	new KIRCMethodFunctor_S_Suffix<KIRC>(this, &KIRC::incomingConnectString,		1,	1));
	addIrcMethod("004",	&KIRC::numericReply_004,	5,	5);

	/* NOT IN RFC1459 NOR RFC2812
	 * Tells connections statistics about the server for the uptime activity:
	 * ":Highest connection count: <integer> (<integer> clients) (<integer> since server was (re)started)"
	 * This is not the only form of this message ignoring it for now.
	 * FIXME: Implement me */
	addIrcMethod("250",	new KIRCMethodFunctor_Ignore());

	/* Tells how many user there are on all the different servers in the form of:
	 * ":There are <integer> users and <integer> services on <integer> servers" */
	addIrcMethod("251",	new KIRCMethodFunctor_Ignore());

	/* Issues a number of operators on the server in the form of:
	 * "<integer> :operator(s) online" */
	// FIXME: send an integer instead of a QString
	addIrcMethod("252",	&KIRC::numericReply_252,	2,	2);

	/* Tells how many unknown connections the server has in the form of:
	 * "<integer> :unknown connection(s)" */
	// FIXME: send an integer instead of a QString
	addIrcMethod("253",	&KIRC::numericReply_253,	2,	2);

	/* Tells how many total channels there are on this network in the form of:
	 * "<integer> :channels formed" */
	// FIXME: send an integer instead of a QString
	addIrcMethod("254",	&KIRC::numericReply_254,	2,	2);

	/* Tells how many clients and servers *this* server handles in the form of:
	 * ":I have <integer> clients and <integer> servers" */
	addIrcMethod("255",	new KIRCMethodFunctor_S_Suffix<KIRC>(this, &KIRC::incomingConnectString, 1, 1));

	/* NOT IN RFC2812
	 * Tells statistics about the current local server state:
	 * ":Current local  users: <integer>  Max: <integer>" */
	addIrcMethod("265",	new KIRCMethodFunctor_Ignore());

	/* Tells statistics about the current global(the whole irc server chain) server state:
	 * ":Current global users: <integer>  Max: <integer>" */
	addIrcMethod("266",	new KIRCMethodFunctor_Ignore());

	/* "<nick> :<away message>"
	 */
	addIrcMethod("301",	new KIRCMethodFunctor_SS_Suffix<KIRC,1>(this, &KIRC::incomingUserIsAway, 2, 2));

	addIrcMethod("303",	&KIRC::numericReply_303,	1,	1);

	/* ":You are no longer marked as being away"
	 */
	addIrcMethod("305",	new KIRCMethodFunctor_Ignore());

	/* ":You have been marked as being away"
	 */
	addIrcMethod("306",	new KIRCMethodFunctor_Ignore());

	/* Show info about a user (part of a /whois) in the form of:
	 * "<nick> <user> <host> * :<real name>" */
	addIrcMethod("311",	&KIRC::numericReply_311,	5,	5);

	/* Show info about a server (part of a /whois) in the form of:
	 * "<nick> <server> :<server info>" */
	addIrcMethod("312",	&KIRC::numericReply_312,	3,	3);

	/* Show info about an operator (part of a /whois) in the form of:
	 * "<nick> :is an IRC operator" */
	addIrcMethod("313",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWhoIsOperator, 2, 2));

	addIrcMethod("317",	&KIRC::numericReply_317,	3,	4);

	/* Receive end of WHOIS in the form of
	 * "<nick> :End of /WHOIS list" */
	addIrcMethod("318",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfWhois, 2, 2));

	addIrcMethod("319",	&KIRC::numericReply_319,	2,	2);

	/* RFC1459: "<channel> :Users  Name" ("Channel :Users  Name")
	 * RFC2812: Obsolete. Not used. */
	addIrcMethod("321",	new KIRCMethodFunctor_Ignore());

	/* Received one channel from the LIST command.
	 * "<channel> <# visible> :<topic>" */
	addIrcMethod("322",	&KIRC::numericReply_322,	3,	3);

	/* End of the LIST command.
	 * ":End of LIST" */
	addIrcMethod("323",	new KIRCMethodFunctor_Empty<KIRC>(this, &KIRC::incomingEndOfList));

	addIrcMethod("324",	&KIRC::numericReply_324,	2,	4);
	addIrcMethod("329",	&KIRC::numericReply_329,	3,	3);
	addIrcMethod("331",	&KIRC::numericReply_331,	2,	2);
	addIrcMethod("332",	&KIRC::numericReply_332,	2,	2);
	addIrcMethod("333",	&KIRC::numericReply_333,	4,	4);
	addIrcMethod("353",	&KIRC::numericReply_353,	3,	3);

	/* Gives a signal to indicate that the NAMES list has ended for a certain channel in the form of:
	 * "<channel> :End of NAMES list" */
	addIrcMethod("366",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfNames, 2, 2));

	/* Part of the MOTD.
	 * ":- <text>" */
	addIrcMethod("372",	&KIRC::numericReply_372,	1,	1);

	/* Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
	 * ":- <server> Message of the day - "
	 */
	addIrcMethod("375",	&KIRC::numericReply_375,	1,	1);
//	addIrcMethod("375",	KIRCMethodFunctor_S_Suffix(this, &KIRC::incomingStartOfMotd,	1,	1));

	/* End of the motd.
	 * ":End of MOTD command" */
	addIrcMethod("376",	&KIRC::numericReply_376,	1,	1);

	/* Gives a signal to indicate that the command issued failed because the person not being on IRC in the for of:
	 * "<nickname> :No such nick/channel"
	 *  - Used to indicate the nickname parameter supplied to a command is currently unused. */
	addIrcMethod("401",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingNoNickChan, 2, 2));

	addIrcMethod("404",	new KIRCMethodFunctor_SS_Suffix<KIRC, 1>(this, &KIRC::incomingCannotSendToChannel, 2, 2));

	/* Like case 401, but when there *was* no such nickname
	 * "<nickname> :There was no such nickname" */
	addIrcMethod("406",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWasNoNick, 2, 2));

	addIrcMethod("433",	&KIRC::numericReply_433,	2,	2);

	addIrcMethod("442",	new KIRCMethodFunctor_SS_Suffix<KIRC, 1>(this, &KIRC::incomingCannotSendToChannel, 2, 2));

	/* Bad server password */
	addIrcMethod("464",	&KIRC::numericReply_464,	1,	1);
	/* Channel is Full */
	addIrcMethod("471",	&KIRC::numericReply_471,	2,	2);
	/* Invite Only */
	addIrcMethod("473",	&KIRC::numericReply_473,	2,	2);
	/* Banned */
	addIrcMethod("474",	&KIRC::numericReply_474,	2,	2);
	/* Wrong Chan-key */
	addIrcMethod("475",	&KIRC::numericReply_475,	2,	2);


//	CTCP Queries
	addCtcpQueryIrcMethod("ACTION",		&KIRC::CtcpQuery_action,	-1,	-1,	"");
	addCtcpQueryIrcMethod("CLIENTINFO",	&KIRC::CtcpQuery_clientInfo,	-1,	1,	"");
	addCtcpQueryIrcMethod("DCC",		&KIRC::CtcpQuery_dcc,		4,	5,	"");
	addCtcpQueryIrcMethod("FINGER",		&KIRC::CtcpQuery_finger,	-1,	0,	"");
	addCtcpQueryIrcMethod("PING",		&KIRC::CtcpQuery_pingPong,	1,	1,	"");
	addCtcpQueryIrcMethod("SOURCE",		&KIRC::CtcpQuery_source,	-1,	0,	"");
	addCtcpQueryIrcMethod("TIME",		&KIRC::CtcpQuery_time,		-1,	0,	"");
	addCtcpQueryIrcMethod("USERINFO",	&KIRC::CtcpQuery_userInfo,	-1,	0,	"");
	addCtcpQueryIrcMethod("VERSION",	&KIRC::CtcpQuery_version,	-1,	0,	"");

//	CTCP Replies
	addCtcpReplyIrcMethod("ERRMSG",		&KIRC::CtcpReply_errorMsg,	1,	-1,	"");
	addCtcpReplyIrcMethod("PING",		&KIRC::CtcpReply_pingPong,	1,	1,	"");
	addCtcpReplyIrcMethod("VERSION",	&KIRC::CtcpReply_version,	-1,	-1,	"");

	QObject::connect(&m_sock, SIGNAL(lookupFinished(int)), this, SLOT(slotHostFound()));
	QObject::connect(&m_sock, SIGNAL(connectionSuccess()), this, SLOT(slotConnected()));
	QObject::connect(&m_sock, SIGNAL(closed(int)), this, SLOT(slotConnectionClosed()));

	QObject::connect(&m_sock, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	QObject::connect(&m_sock, SIGNAL(connectionFailed(int)), this, SLOT(error(int)));

	m_VersionString = QString::fromLatin1("Anonymous client using the KIRC engine.");
	m_UserString = QString::fromLatin1("Response not supplied by user.");
	m_SourceString = QString::fromLatin1("Unknown client, known source.");

	defaultCodec = QTextCodec::codecForMib(4);
	            kdDebug(14120) << k_funcinfo << defaultCodec->name() << endl;
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
}

void KIRC::setStatus(EngineStatus status)
{
	if( status == Disconnected && m_status != Closing )
	{
		emit disconnected();
	}

	m_status = status;
	emit statusChanged(status);
}


void KIRC::connectToServer(const QString &nickname, const QString &host, Q_UINT16 port)
{
	if (!nickname.isNull())
		m_Nickname = nickname;
	if (!host.isNull())
	{
		m_Host = host;
		m_Port = (port==0)?6667:port;
	}

	kdDebug(14120) << "Trying to connect to server " << m_Host << ":" << m_Port << endl;
	kdDebug(14120) << "Sock status: " << m_sock.socketStatus() << endl;
	if (!m_sock.setAddress(m_Host, m_Port))
		kdDebug(14120) << k_funcinfo << "setAddress failed. Status:  " << m_sock.socketStatus() << endl;
	if (m_sock.lookup()) // necessary to avoid QDns
		kdDebug(14120) << k_funcinfo << "lookup() failed. Status: " << m_sock.socketStatus() << endl;
	if(m_sock.startAsyncConnect()==0) {
		kdDebug(14120) << k_funcinfo << "startAsyncConnect() success!. Status: " << m_sock.socketStatus() << endl;
		setStatus(Connecting);
	} else {
		kdDebug(14120) << k_funcinfo << "startAsyncConnect() failed. Status: " << m_sock.socketStatus() << endl;
		setStatus(Disconnected);
	}

}

void KIRC::slotHostFound()
{
	kdDebug(14120) << "Host Found" << endl;
}

void KIRC::slotAuthFailed()
{
 	if( m_status != Connected )
	{
		setStatus(Disconnected);
		m_sock.close();
		m_sock.reset();
	}
}

void KIRC::slotConnected()
{
	kdDebug(14120) << k_funcinfo << "Connected" << endl;
	setStatus(Authentifying);
	m_sock.enableRead(true);
	// If password is given for this server, send it now, and don't expect a reply
	if (!(password()).isEmpty()) {
		writeMessage("PASS", QStringList(password()) , m_Realname, false);
	}

	changeUser(m_Username, 0, QString::fromLatin1("Kopete User"));
	changeNickname(m_Nickname);

	//If we don't get a reply within 15 seconds, give up
	QTimer::singleShot(connectTimeout, this, SLOT(slotAuthFailed()));
}

void KIRC::slotConnectionClosed()
{
	kdDebug(14120) << k_funcinfo << "Connection Closed - local status: " << m_status << " sock status: " << m_sock.socketStatus() << endl;
	if(m_status == Closing)
		emit successfulQuit();

	if(m_status!=Disconnected)
		setStatus(Disconnected);
	m_sock.reset();
}

void KIRC::error(int /*errCode*/)
{
	if (m_sock.socketStatus () != KExtendedSocket::connecting)
	{
		// Connection in progress.. This is a signal fired wrong
		setStatus(Disconnected);
		m_sock.reset();
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
	map.replace(QString::fromLatin1(str).upper(), method);
}

void KIRC::addIrcMethod(QDict<KIRCMethodFunctorCall> &map, const char *str,
			bool (KIRC::*method)(const KIRCMessage &msg),
			int argsSize_min, int argsSize_max, const char *helpMessage)
{
	addIrcMethod(map, str, new KIRCMethodFunctor_Forward<KIRC>(this, method, argsSize_min, argsSize_max, helpMessage));
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
		KIRCMessage ircmsg = KIRCMessage::writeRawMessage(&m_sock, rawMsg, defaultCodec);
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
		KIRCMessage ircmsg = KIRCMessage::writeMessage(&m_sock, msg, defaultCodec);
		emit sentMessage(ircmsg);
		return ircmsg;
	}
	kdDebug(14120) << "Must be connected error:" << msg << endl;
	return KIRCMessage();
}

KIRCMessage KIRC::writeMessage(const QString &command, const QStringList &args, const QString &suffix, bool mustBeConnected)
{
	if(canSend(mustBeConnected))
	{
		KIRCMessage ircmsg = KIRCMessage::writeMessage(&m_sock, command, args, suffix, defaultCodec);
		emit sentMessage(ircmsg);
		return ircmsg;
	}
	kdDebug( 14120 ) << k_funcinfo << "Must be connected error:" << command << args.join( " " ) << suffix << endl;
	return KIRCMessage();
}

KIRCMessage KIRC::writeMessage(const QString &command, const QString &arg, const QString &suffix, bool mustBeConnected)
{
	if(canSend(mustBeConnected))
	{
		KIRCMessage ircmsg = KIRCMessage::writeMessage(&m_sock, command, arg,
			suffix, codecForNick( arg ) );
		emit sentMessage(ircmsg);
		return ircmsg;
	}
	kdDebug(14120) << "Must be connected error:" << command << arg << suffix << endl;
	return KIRCMessage();
}

KIRCMessage KIRC::writeCtcpMessage(const char *command, const QString &to, const QString &suffix,
		const QString &ctcpMessage, bool emitRepliedCtcp)
{
	QString nick =  getNickFromPrefix(to);
	KIRCMessage msg = KIRCMessage::writeCtcpMessage(&m_sock, QString::fromLatin1(command),
		nick, suffix, ctcpMessage, codecForNick( to ) );

	emit sentMessage(msg);
	if(emitRepliedCtcp && msg.isValid() && msg.hasCtcpMessage())
		emit repliedCtcp(msg.ctcpMessage().command(), msg.ctcpMessage().ctcpRaw());

	return msg;
}

KIRCMessage KIRC::writeCtcpMessage(const char *command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QString &ctcpArg, const QString &ctcpSuffix, bool emitRepliedCtcp)
{
	QString nick =  getNickFromPrefix(to);
	KIRCMessage msg = KIRCMessage::writeCtcpMessage(&m_sock, QString::fromLatin1(command),
		nick, suffix, ctcpCommand, ctcpArg, ctcpSuffix, codecForNick( to ) );

	emit sentMessage(msg);
	if(emitRepliedCtcp && msg.isValid() && msg.hasCtcpMessage())
		emit repliedCtcp(msg.ctcpMessage().command(), msg.ctcpMessage().ctcpRaw());

	return msg;
}

KIRCMessage KIRC::writeCtcpMessage(const char *command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix, bool emitRepliedCtcp)
{
	QString nick =  getNickFromPrefix(to);
	KIRCMessage msg = KIRCMessage::writeCtcpMessage(&m_sock, QString::fromLatin1(command),
		nick, suffix, ctcpCommand, ctcpArgs, ctcpSuffix, codecForNick( to ) );

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

//	while(	m_sock.socketStatus()==KExtendedSocket::connected &&
//		m_sock.canReadLine())
	if(	m_sock.socketStatus()==KExtendedSocket::connected &&
		m_sock.canReadLine())
	{
		KIRCMessage msg = KIRCMessage::parse(&m_sock, &parseSuccess, defaultCodec);
		if(parseSuccess)
		{
			KIRCMethodFunctorCall *method = m_IrcMethods[msg.command()];
			if(method && method->isValid())
			{
				if (method->checkMsgValidity(msg))
				{
					// FIXME: should also check for args[0] == m_nickname/"*" for nummeric replies
					// If the m_nickname is given, the nickname change is successful(ie we are in).
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
		QTimer::singleShot(0, this, SLOT(slotReadyRead())); // Event loop.
	}

	if(m_sock.socketStatus()!=KExtendedSocket::connected)
		error();
}
/*
void KIRC::setPassword(const QString &passord)
{
}
*/

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

	writeMessage("USER", QStringList(m_Username) << hostname << m_Host, m_Realname, false);
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

	writeMessage("USER", QStringList(m_Username) << QString::number(mode) << QChar('*'), m_Realname, false);
}

void KIRC::quitIRC(const QString &reason, bool now)
{
	if(!now || canSend(now))
		writeMessage("QUIT", QString::null, reason, false);
	if(now)
	{
//		setStatus(Disconnected); // This segfaults while deleteing the KIRC object.
		m_status = Disconnected;
		m_sock.close();
	}
	else
	{
		if(	m_status != Disconnected &&
			m_status != Closing)
		{
			setStatus(Closing);
		}
		QTimer::singleShot(10000, this, SLOT(quitTimeout()));
	}
}

void KIRC::quitTimeout()
{
	if(	m_sock.socketStatus() > KExtendedSocket::nothing &&
		m_sock.socketStatus() < KExtendedSocket::done &&
		m_status == Closing)
	{
		setStatus(Disconnected);
		m_sock.close();
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
	/* This will join a channel
	 */
	if (!key.isNull()) {
		writeMessage("JOIN", QStringList(name) << key);
	} else {
		writeMessage("JOIN", name);
	}

}

bool KIRC::joinChannel(const KIRCMessage &msg)
{
	/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
	 * suspected: ":<channel> *(" "/"," <channel>)"
	 * assumed ":<channel>"
	 * This is the response of someone joining a channel.
	 * Remember that this will be emitted when *you* /join a room for the first time */
	if (msg.args().size()==1)
		emit userJoinedChannel(msg.prefix(), msg.args()[0]);
	else
		emit userJoinedChannel(msg.prefix(), msg.suffix());
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
	emit incomingPartedChannel(msg.prefix(), msg.args()[0], msg.suffix());
	return true;
}

void KIRC::changeMode(const QString &target, const QString &mode)
{
	writeMessage("MODE", QStringList(target) << mode);
}

bool KIRC::modeChange(const KIRCMessage &msg)
{
	/* Change the mode of a user.
	 * "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
	 */
	QStringList args = msg.args();
	args.pop_front();
	emit incomingModeChange( msg.prefix().section('!',0,0), msg.args()[0], args.join(" "));
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
	emit incomingTopicChange(msg.prefix().section('!',0,0), msg.args()[0], msg.suffix());
	return true;
}
/*
// Show visible users in the channel list
void KIRC::names(const QStringList &channels)
{
	QString arg;
	if( channel.size()>0 )
		while()
			arg += "," + channel(index)
	writeMessage("NAMES", QString::null);
}
*/
void KIRC::list()
{
	writeMessage("LIST", QString::null);
}

void KIRC::motd(const QString &server)
{
	writeMessage("MOTD", server);
}

/*
void KIRC::invite()
{
}

bool KIRC::invite(const KIRCMessage &msg)
{
	emit incomingInvitation(prefix() who, m_arg[0] from, m_arg[1] to_channel);
}
*/
void KIRC::kickUser(const QString &user, const QString &channel, const QString &reason)
{
	writeMessage("KICK", QStringList(channel) << user, reason);
}

bool KIRC::kick(const KIRCMessage &msg)
{
	/* The given user is kicked.
	 * "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
	 */
	emit incomingKick( msg.prefix().section('!',0,0), msg.args()[0], msg.args()[1], msg.suffix());
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
		QString user = m.args()[0];
		m = KIRCMessage::parse( codecForNick(user)->toUnicode( m.raw() ) );

		QString message = m.suffix();
		QChar tmpChar = user[0];

		if (tmpChar == QChar('#') || tmpChar == QChar('!') || tmpChar == QChar('&'))
			emit incomingMessage(getNickFromPrefix(msg.prefix()), msg.args()[0], message );
		else
			emit incomingPrivMessage(getNickFromPrefix(msg.prefix()), msg.args()[0], message );
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
/*
void KIRC::serverMessageOfTheDay(const QString &server) // server = QString::null means current server
{
	writeMessage("MOTD", server);
}

//	FIXME: should follow the optionnal syntax [mask [target]]
void KIRC::usersStatistics( const QString &mask, const QString &target )
{
	writeMessage("LUSERS", QStringList(mask) << target);
}

void KIRC::serverVersion(const QString &server) // server = QString::null means current server
{
	writeMessage("VERSION", server);
}

//	FIXME: should follow the optionnal syntax [query [target]]
void KIRC::serverStatistics( const QString &query, const QString &target )
{
	writeMessage("STATS", QStringList(query) << target);
}

//	FIXME: [ [ <remote server> ] <server mask> ]
void KIRC::serverLinks(const QString &remote, const QString &mask)
{
	writeMessage("LINKS", QStringList(query) << mask);
}

void KIRC::serverTime(const QString &server) // server = QString::null means current server
{
	writeMessage("TIME", server);
}

//	For admin users
void KIRC::serverConnect(const QString &server, Q_UINT16 port)
{
	writeMessage("CONNECT", QStringList(server) << QString::number(port) );
}

void KIRC::serverTrace( const QString &server ) // server = QString::null means current server
{
	writeMessage("TRACE", server);
}

// Get some info about the server administrator
void KIRC::serverAdmin( const QString &server ) // server = QString::null means current server
{
	writeMessage("ADMIN", server);
}

// target = QString::null means current server
// target can also be a nick, in this case return the nick connected to server
void KIRC::serverInfo( const QString &target )
{
	writeMessage("INFO", target);
}

//	FIXME: [ [ <remote server> ] <server mask> ]
void KIRC::serviceList( const QString &mask, const QString &type )
{
	writeMessage("SERVLIST", QStringList(mask) << type);
}

void KIRC::queryService( const QString &serviceName, const QString &query )
{
	writeMessage("SQUERY", serviceName, query);
}

//	FIXME [ <mask> [ "o" ] ]
void KIRC::whoUser(const QString &mask, bool askForOperator)
{
	writeMessage("WHO", mask);
}
*/
void KIRC::whoisUser(const QString &user)
{
	writeMessage("WHOIS", user);
}
/*
//	FIXME: <nickname> *( "," <nickname> ) [ <count> [ <target> ] ]
void KIRC::whowasUser(const QString &nickname)
{
	writeMessage("WHOWAS", nickname);
}

void KIRC::killUser(const QString &nickname, const QString &comment)
{
	writeMessage("KILL", nickname, comment);
}

// ?? Received when killed ??
// bool KIRC::kill(const KIRCMessage &msg)

//	FIXME: <server1> [ <server2> ]
void KIRC::ping(const QString &server1, const QString &server2)
{
	writeMessage("PING", QStringList(server1) << server2);
}
*/
bool KIRC::ping(const KIRCMessage &msg)
{
	writeMessage("PONG", msg.args(), msg.suffix(), false);
	// maybe should emit one signal.
	return true;
}

//	received after a ping command
bool KIRC::pong(const KIRCMessage &/*msg*/)
{
	// maybe should emit one signal.
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
/*
bool KIRC::away(const KIRCMessage &msg)
{
	if(msg.suffix().isNull())
		emit incomingUserPresent( msg.prefix() );
	else
		emit incomingUserAway( msg.prefix(), suffix );
}

// Force the server to re-read and process its configuration file.
void KIRC::serverRehash()
{
	// FIXME: should we check operator rigth to avoid one error msg?
	writeMessage("REHASH", QString::null);
}

void KIRC::serverRestart()
{
	// FIXME: should we check operator rigth to avoid one error msg?
	writeMessage("RESTART", QString::null);
}

void KIRC::summonUser(const QString &user, const QString &server) //server=QString::null
{
	QStringList args(user);
	if(!server.isEmpty())
		args << server;
	writeMessage("SUMMON", args);
}

void KIRC::users(const QString &server) //server=QString::null
{
	writeMessage("USERS", server);
}

bool KIRC::users(const KIRCMessage &msg)
{
	// what to do with this?
	// should be of the form: :user USERS server
}

void KIRC::warnAllOperators(const QString &message)
{
	writeMessage("WALLOPS", QString::null, message);
}

bool KIRC::warnAllOperators(const KIRCMessage &msg)
{
	// what to do with this?
	// should be of the form: :user WALLOPS :message
}

// Make a check of users host up to 5 users.
void KIRC::usersHosts( const QStringList &users )
{
// FIXME: support more than 5 users
	writeMessage("USERHOST", users);
}
*/
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

		//15 second timeout to detect network disconnections
		isonRecieved = false;
		QTimer::singleShot(15000, this, SLOT( slotIsonCheck() ) );
	}
}

void KIRC::slotIsonCheck()
{
	if( !isonRecieved )
	{
		setStatus( Disconnected );
		m_sock.close();
		m_sock.reset();
	}
}

bool KIRC::numericReply_001(const KIRCMessage &msg)
{
	/* Gives a welcome message in the form of:
	 * "Welcome to the Internet Relay Network <nick>!<user>@<host>"
	 */
	if (m_FailedNickOnLogin == true)
	{
		// this is if we had a "Nickname in use" message when connecting and we set another nick.
		// This signal emits that the nick was accepted and we are now logged in
		emit successfullyChangedNick(m_Nickname, m_PendingNick);
		m_Nickname = m_PendingNick;
		m_FailedNickOnLogin = false;
	}

	/* At this point we are connected and the server is ready for us to being taking commands
	 * although the MOTD comes *after* this.
	 */
	emit incomingConnectString(msg.suffix());
	setStatus(Connected);
	emit connectedToServer();

	return true;
}

bool KIRC::numericReply_004(const KIRCMessage &msg)
{
	/* Gives information about the servername, version, available modes, etc in the form of:
	 * "<servername> <version> <available user modes> <available channel modes>"
	 */
	emit incomingHostInfo(msg.args()[1],msg.args()[2],msg.args()[3],msg.args()[4]);
	return true;
}

bool KIRC::numericReply_252(const KIRCMessage &msg)
{
	emit incomingConnectString( msg.toString() );
	return true;
}

bool KIRC::numericReply_253(const KIRCMessage &msg)
{
	emit incomingConnectString( msg.toString() );
	return true;
}

bool KIRC::numericReply_254(const KIRCMessage &msg)
{
	emit incomingConnectString( msg.toString() );
	return true;
}

bool KIRC::numericReply_265(const KIRCMessage &msg)
{
	return true;
}

bool KIRC::numericReply_266(const KIRCMessage &msg)
{
	return true;
}

bool KIRC::numericReply_303(const KIRCMessage &msg)
{
	/* ":*1<nick> *(" " <nick> )"
	 */
	isonRecieved = true;
	QStringList nicks = QStringList::split(QRegExp(QChar(' ')), msg.suffix());
	for(QStringList::Iterator it = nicks.begin(); it != nicks.end(); ++it)
	{
		if (!(*it).stripWhiteSpace().isEmpty())
			emit userOnline(*it);
	}
	return true;
}
/*
bool KIRC::numericReply_305(const KIRCMessage &msg)
{
	return true;
}

bool KIRC::numericReply_306(const KIRCMessage &msg)
{
	return true;
}
*/
bool KIRC::numericReply_311(const KIRCMessage &msg)
{
	emit incomingWhoIsUser(msg.args()[1], msg.args()[2], msg.args()[3], msg.suffix());
	return true;
}

bool KIRC::numericReply_312(const KIRCMessage &msg)
{
	emit incomingWhoIsServer(msg.args()[1], msg.args()[2], msg.suffix());
	return true;
}

bool KIRC::numericReply_317(const KIRCMessage &msg)
{
	/* RFC say: "<nick> <integer> :seconds idle"
	 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
	 * Show info about someone who is idle (part of a /whois) in the form of:
	 */
	emit incomingWhoIsIdle(msg.args()[1], msg.args()[2].toULong());
	if (msg.args().size()==4)
		emit incomingSignOnTime(msg.args()[1],msg.args()[3].toULong());
	return true;
}

bool KIRC::numericReply_319(const KIRCMessage &msg)
{
	/* Show info a channel a user is logged in (part of a /whois) in the form of:
	 * "<nick> :{[@|+]<channel><space>}"
	 */
	emit incomingWhoIsChannels(msg.args()[1], msg.suffix());
	return true;
}

bool KIRC::numericReply_322(const KIRCMessage &msg)
{
	/* "<channel> <# visible> :<topic>"
	 */
	//kdDebug(14120) << k_funcinfo << "Listed " << msg.args()[1] << endl;

	emit incomingListedChan(msg.args()[1], msg.args()[2].toUInt(), msg.suffix());
	return true;
}

bool KIRC::numericReply_324(const KIRCMessage &msg)
{
	/* "<channel> <mode> <mode params>"
	 */
	emit incomingChannelMode(msg.args()[1], msg.args()[2], msg.args()[3]);
	return true;
}

bool KIRC::numericReply_329( const KIRCMessage & /* msg */ )
{
	/* NOT IN RFC1459 NOR RFC2812
	 * "%s %lu"
	 * FIXME: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
	 */
	return true;
}

bool KIRC::numericReply_331( const KIRCMessage & /* msg */ )
{
	/* Gives the existing topic for a channel after a join.
	 * "<channel> :No topic is set"
	 */
//	emit incomingExistingTopic(args[1], suffix);
	return true;
}

bool KIRC::numericReply_332(const KIRCMessage &msg)
{
	/* Gives the existing topic for a channel after a join.
	 * "<channel> :<topic>"
	 */
	emit incomingExistingTopic(msg.args()[1], msg.suffix());
	return true;
}

bool KIRC::numericReply_333( const KIRCMessage & /* msg */ )
{
	/* NOT IN RFC1459 NOR RFC2812
	 * "%s %s %lu"
	 * FIXME: What is the meaning of this arguments. DAL-ircd say it's a RPL_TOPICWHOTIME
	 */
	 return true;
}

bool KIRC::numericReply_353(const KIRCMessage &msg)
{
	/* Gives a listing of all the people in the channel in the form of:
	 * "( "=" / "*" / "@" ) <channel> :[ "@" / "+" ] <nick> *( " " [ "@" / "+" ] <nick> )
	 *  - "@" is used for secret channels, "*" for private
	 *    channels, and "=" for others (public channels).
	 */
	kdDebug(14120) << "Adding name list:" << msg.suffix() << endl;
	emit incomingNamesList(msg.args()[2], QStringList::split(' ', msg.suffix()));
	return true;
}

bool KIRC::numericReply_372(const KIRCMessage &msg)
{
	m_motdBuffer.append( msg.suffix() );
	return true;
}

bool KIRC::numericReply_375(const KIRCMessage &msg)
{
	m_motdBuffer.clear();
	return true;
}

bool KIRC::numericReply_376(const KIRCMessage &msg)
{
	emit incomingMotd(m_motdBuffer);
	return true;
}

bool KIRC::numericReply_433(const KIRCMessage &msg)
{
	/* "<nick> :Nickname is already in use"
	 * Tells us that our nickname is already in use.
	 */
	if(m_status == Authentifying)
	{
		// This tells us that our nickname is, but we aren't logged in.
		// This differs because the server won't send us a response back telling us our nick changed
		// (since we aren't logged in).
		m_FailedNickOnLogin = true;
		emit incomingFailedNickOnLogin(msg.args()[1]);
	}
	else
	{
		// And this is the signal for if someone is trying to use the /nick command or such when already logged in,
		// but it's already in use
		emit incomingNickInUse(msg.args()[1]);
	}


	return true;
}

bool KIRC::numericReply_464(const KIRCMessage &/*msg*/)
{
	/* Server need pass.. Call disconnect */
	emit incomingFailedServerPassword();
	return true;
}

bool KIRC::numericReply_471(const KIRCMessage &msg)
{
	emit incomingFailedChanFull(msg.args()[1]);
	return true;
}

bool KIRC::numericReply_473(const KIRCMessage &msg)
{
	emit incomingFailedChanInvite(msg.args()[1]);
	return true;
}

bool KIRC::numericReply_474(const KIRCMessage &msg)
{
	emit incomingFailedChanBanned(msg.args()[1]);
	return true;
}

bool KIRC::numericReply_475(const KIRCMessage &msg)
{
	emit incomingFailedChankey(msg.args()[1]);
	return true;
}

void KIRC::sendCtcpCommand(const QString &contact, const QString &command)
{
	if(m_status == Connected)
	{
		writeCtcpQueryMessage(contact, QString::null, command);
//		emit ctcpCommandMessage( contact, command );
	}
}

void KIRC::sendCtcpAction(const QString &contact, const QString &message)
{
	if(m_status == Connected)
	{
		writeCtcpQueryMessage(contact, QString::null, "ACTION", QStringList(message));

		if (contact[0] != '#' && contact[0] != '!' && contact[0] != '&')
			emit incomingPrivAction(m_Nickname, contact, message);
		else
			emit incomingAction(m_Nickname, contact, message);
	}
}

bool KIRC::CtcpQuery_action(const KIRCMessage &msg)
{
	QString target = msg.args()[0];
	if (target[0] == '#' || target[0] == '!' || target[0] == '&')
		emit incomingAction(msg.prefix(), target, msg.ctcpMessage().ctcpRaw());
	else
		emit incomingPrivAction(msg.prefix(), target, msg.ctcpMessage().ctcpRaw());
	return true;
}

void KIRC::sendCtcpPing(const QString &target)
{
	timeval time;
	if (gettimeofday(&time, 0) == 0)
	{
		// FIXME: the time code is wrong for usec
		QString timeReply = QString::fromLatin1("%1.%2").arg(time.tv_sec).arg(time.tv_usec);
		writeCtcpQueryMessage(	target, QString::null,
					"PING", timeReply);
	}
}

bool KIRC::CtcpQuery_pingPong(const KIRCMessage &msg)
{
	writeCtcpReplyMessage(	msg.prefix(), QString::null,
				msg.ctcpMessage().command(), msg.ctcpMessage().args()[0]);
	return true;
}

bool KIRC::CtcpReply_pingPong( const KIRCMessage & /* msg */ )
{
/*
	timeval time;
	if (gettimeofday(&time, 0) == 0)
	{
		// FIXME: the time code is wrong for usec
		QString timeReply = QString::fromLatin1("%1.%2").arg(time.tv_sec).arg(time.tv_usec);
		double newTime = timeReply.toDouble();
		double oldTime = ctcpArgs[0].toDouble();
		double difference = newTime - oldTime;
		QString diffString;
		if (difference < 1)
		{
			diffString = QString::number(difference);
			diffString.remove((diffString.find('.') -1), 2);
			diffString.truncate(3);
			diffString.append(i18n("msecs"));
		} else {
			diffString = QString::number(difference);
			QString seconds = diffString.section('.', 0, 0);
			QString millSec = diffString.section('.', 1, 1);
			millSec.remove(millSec.find('.'), 1);
			millSec.truncate(3);
			diffString = QString::fromLatin1("%1 secs %2 msecs").arg(seconds).arg(millSec);
		}
		emit incomingCtcpReply(QString::fromLatin1("PING"), originating.section('!', 0, 0), diffString);
		return true;
	}*/
	return false;
}

//void KIRC::queryCtcpVersion(const QString &target)
void KIRC::sendCtcpVersion(const QString &target)
{
	writeCtcpQueryMessage(target, QString::null, "VERSION");
}

bool KIRC::CtcpQuery_version(const KIRCMessage &msg)
{
	QString response = customCtcpMap[ QString::fromLatin1("version") ];
	kdDebug(14120) << "Version check: " << response << endl;

	if( !response.isNull() )
	{
		writeCtcpReplyMessage(msg.prefix(), QString::null,
			msg.ctcpMessage().command(), QStringList(), response);
	}
	else
	{
		writeCtcpReplyMessage(msg.prefix(), QString::null,
			msg.ctcpMessage().command(), QStringList(), m_VersionString);
	}

	return true;
}

bool KIRC::CtcpReply_version(const KIRCMessage &msg)
{
	emit incomingCtcpReply(msg.ctcpMessage().command(), getNickFromPrefix(msg.prefix()), msg.ctcpMessage().ctcpRaw());
	return true;
}

bool KIRC::CtcpQuery_userInfo(const KIRCMessage &msg)
{
	QString response = customCtcpMap[ QString::fromLatin1("userinfo") ];
	if( !response.isNull() )
	{
		writeCtcpReplyMessage(msg.prefix(), QString::null,
			msg.ctcpMessage().command(), QStringList(), response);
	}
	else
	{
		writeCtcpReplyMessage( msg.prefix(), QString::null,
				msg.ctcpMessage().command(), QStringList(), m_UserString );
	}

	return true;
}

//	FIXME: the API can now answer to help commands.
bool KIRC::CtcpQuery_clientInfo(const KIRCMessage &msg)
{
	QString response = customCtcpMap[ QString::fromLatin1("clientinfo") ];
	if( !response.isNull() )
	{
		writeCtcpReplyMessage(	msg.prefix(), QString::null,
					msg.ctcpMessage().command(), QStringList(), response);
	}
	else
	{
		QString info = QString::fromLatin1("The following commands are supported, but "
			"without sub-command help: VERSION, CLIENTINFO, USERINFO, TIME, SOURCE, PING,"
			"ACTION.");

		writeCtcpReplyMessage(	msg.prefix(), QString::null,
					msg.ctcpMessage().command(), QStringList(), info);
	}
	return true;
}

bool KIRC::CtcpQuery_time(const KIRCMessage &msg)
{
	writeCtcpReplyMessage(	msg.prefix(), QString::null,
				msg.ctcpMessage().command(), QStringList(QDateTime::currentDateTime().toString()), QString::null,
				false);
	return true;
}

bool KIRC::CtcpQuery_source(const KIRCMessage &msg)
{
	writeCtcpReplyMessage(	msg.prefix(), QString::null,
				msg.ctcpMessage().command(), QStringList(m_SourceString));
	return true;
}

bool KIRC::CtcpQuery_finger( const KIRCMessage & /* msg */ )
{
	// To be implemented
	return true;
}

void KIRC::requestDccConnect(const QString &nickname, const QString &filename, uint port, DCCClient::Type type)
{
	if(	m_status != Connected ||
		m_sock.localAddress() == 0 ||
		m_sock.localAddress()->nodeName() == QString::null)
		return;

	if(type == DCCClient::Chat)
	{
		writeCtcpQueryMessage(nickname, QString::null,
			QString("DCC"),
			QStringList(QString::fromLatin1("CHAT")) << QString::fromLatin1("chat") <<
			m_sock.localAddress()->nodeName() << QString::number(port));
	}
	else if(type == DCCClient::File)
	{
		QFileInfo file(filename);
		QString noWhiteSpace = file.fileName();
		if (noWhiteSpace.contains(' ') > 0)
			noWhiteSpace.replace(QRegExp("\\s+"), "_");

		writeCtcpQueryMessage(nickname, QString::null,
			QString("DCC"),
			QStringList( QString::fromLatin1( "SEND" ) ) << noWhiteSpace <<
			    m_sock.localAddress()->nodeName() << QString::number( port ) << QString::number( file.size() ) );
	}
}

bool KIRC::CtcpQuery_dcc(const KIRCMessage &msg)
{
	const KIRCMessage &ctcpMsg = msg.ctcpMessage();
	QString dccCommand = ctcpMsg.args()[0].upper();

	if (dccCommand == QString::fromLatin1("CHAT"))
	{
		if(ctcpMsg.args().size()!=4) return false;

		/* DCC CHAT type longip port
		 *
		 *  type   = Either Chat or Talk, but almost always Chat these days
		 *  longip = 32-bit Internet address of originator's machine
		 *  port   = Port on which the originator is waitng for a DCC chat
		 */
		bool okayHost, okayPort;
		// should ctctArgs[1] be tested?
		QHostAddress address(ctcpMsg.args()[2].toUInt(&okayHost));
		unsigned int port = ctcpMsg.args()[3].toUInt(&okayPort);
		if (okayHost && okayPort)
		{
			kdDebug(14120) << "Starting DCC chat window." << endl;
			DCCClient *chatObject = new DCCClient(address, port, 0, DCCClient::Chat);
			emit incomingDccChatRequest(address, port, getNickFromPrefix(msg.prefix()), *chatObject);
			return true;
		}
	}
	else if (dccCommand == QString::fromLatin1("SEND"))
	{
		if(ctcpMsg.args().size()!=5) return false;

		/* DCC SEND (filename) (longip) (port) (filesize)
		 *
		 *  filename = Name of file being sent
		 *  longip   = 32-bit Internet address of originator's machine
		 *  port     = Port on which the originator is waiitng for a DCC chat
		 *  filesize = Size of file being sent
		 */
		bool okayHost, okayPort, okaySize;
		QFileInfo realfile(msg.args()[1]);
		QHostAddress address(ctcpMsg.args()[2].toUInt(&okayHost));
		unsigned int port = ctcpMsg.args()[3].toUInt(&okayPort);
		unsigned int size = ctcpMsg.args()[4].toUInt(&okaySize);
		if (okayHost && okayPort && okaySize)
		{
			kdDebug(14120) << "Starting DCC send file transfert." << endl;
			DCCClient *chatObject = new DCCClient(address, port, size, DCCClient::File);
			emit incomingDccSendRequest(address, port, getNickFromPrefix(msg.prefix()), realfile.fileName(), size, *chatObject);
			return true;
		}
	}
//	else
//		emit unknown dcc command signal
	return false;
}

/*
 * The ctcp commands seems to follaw the same message behaviours has normal IRC command.
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

bool KIRC::CtcpReply_errorMsg(const KIRCMessage &msg)
{
	// should emit one signal
	return true;
}

#include "kirc.moc"

