/***************************************************************************
                          kirc.cpp  -  description
                             -------------------
    begin                : Sat Feb 23 2002
    copyright            : (C) 2002 by Nick Betcher
    email                : nbetcher@usinternet.com


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kirc.h"
#include <kdebug.h>
#include <qregexp.h>
#include <sys/time.h>
#include <qdatetime.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <qfileinfo.h>
#include "dcchandler.h"

#include <kglobal.h>
#include <klocale.h>

KIRC::KIRC()
	: QSocket()
{
	attemptingQuit = false;
	waitingFinishMotd = false;
	loggedIn = false;
	failedNickOnLogin = false;
	QObject::connect(this, SIGNAL(hostFound()), this, SLOT(slotHostFound()));
	QObject::connect(this, SIGNAL(connected()), this, SLOT(slotConnected()));
	QObject::connect(this, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	QObject::connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	QObject::connect(this, SIGNAL(bytesWritten(int)), this, SLOT(slotBytesWritten(int)));
	QObject::connect(this, SIGNAL(error(int)), this, SLOT(slotError(int)));

	codec = QTextCodec::codecForLocale();
	if (KGlobal::locale()->country() == "jp") {
		codec = QTextCodec::codecForName("iso-2022-jp");
	}
}

KIRC::~KIRC()
{

}

Q_LONG KIRC::writeString(const QString &str)
{
	QCString s;
	if (codec) {
		s = codec->fromUnicode(str);
	} else {
		s = str.local8Bit();
	}
	return writeBlock(s.data(), s.length());
}

void KIRC::slotError(int error)
{
	kdDebug() << "IRC Plugin: Error, error code == " << error << endl;
	loggedIn = false;
}

void KIRC::slotBytesWritten(int bytes)
{

}

void KIRC::slotReadyRead()
{
	// Please note that the regular expression "[\\r\\n]*$" is used in a QString::replace statement many times. This gets rid of trailing \r\n, \r, \n, and \n\r characters.
	while (canReadLine() == true)
	{
		QString line = codec->toUnicode(readLine());
		line.replace(QRegExp("[\\r\\n]*$"), "");
		QString number = line.mid((line.find(' ', 0, false)+1), 3);
		int commandIndex = line.find(' ', 0, false);
		QString command = line.section(' ', 1, 1);
		if (command == QString("JOIN"))
		{
			/*
			This is the response of someone joining a channel. Remember that this will be emitted when *you* /join a room for the first time
			*/
			kdDebug() << "IRC Plugin: userJoinedChannel emitting" << endl;
			QString channel = line.mid((line.findRev(':')+1), (line.length()-1));
			emit userJoinedChannel(line.mid(1, (commandIndex-1)), channel);
			continue;
		}
		else if (command == QString("PRIVMSG"))
		{
			/*
			This is a signal that indicates there is a new message. This can be either from a channel or from a specific user.
			*/
			kdDebug() << "IRC Plugin: We have a message" << endl;
			QString originating = line.section(' ', 0, 0);
			originating.remove(0, 1);
			QString target = line.section(' ', 2, 2);
			QString message = line.section(' ', 3);
			message.remove(0,1);
			if (QChar(message[0]).unicode() == 1 && (QChar(message[(message.length() -1)])).unicode() == 1)
			{
				// Fun!
				message = message.remove(0, 1);
				message = message.remove((message.length() -1), 1);
				QString special = message.section(' ', 0, 0).lower();
				if (special == "ping")
				{
					QString reply = QString("NOTICE %1 :%2PING %3%4%5").arg(originating.section('!', 0, 0)).arg(QChar(0x01)).arg(message.section(' ', 1, 1)).arg(QChar(0x01)).arg("\r\n");
					writeString(reply);
					emit repliedCtcp("PING", originating.section('!', 0, 0), message.section(' ', 1, 1));
					continue;
				} else if (special == "version")
				{
					// Just remove newlines and line feeds because we don't want to user to get clever ;)
					mVersionString.replace(QRegExp("[\\r\\n]*$"), "");
					if (mVersionString.isEmpty())
					{
						// Don't edit this!
						mVersionString = "Anonymous client using the KIRC engine.";
					}
					QString reply = QString("NOTICE %1 :%2VERSION %3%4%5").arg(originating.section('!', 0, 0)).arg(QChar(0x01)).arg(mVersionString).arg(QChar(0x01)).arg("\r\n");
					writeString(reply);
					emit repliedCtcp("VERSION", originating.section('!', 0, 0), mVersionString);
					continue;
				} else if (special == "userinfo")
				{
					mUserString.replace(QRegExp("[\\r\\n]*$"), "");
					if (mUserString.isEmpty())
					{
						mUserString = "Response not supplied by user.";
					}
					QString reply = QString("NOTICE %1 :%2USERINFO %3%4%5").arg(originating.section('!', 0, 0)).arg(QChar(0x01)).arg(mUserString).arg(QChar(0x01)).arg("\r\n");
					writeBlock(reply.latin1(), reply.length());
					emit repliedCtcp("USERINFO", originating.section('!', 0, 0), mUserString);
					continue;
				} else if (special == "clientinfo")
				{
					QString response = "The following commands are supported, but without sub-command help: VERSION, CLIENTINFO, USERINFO, TIME, SOURCE, PING, ACTION.";
					QString reply = QString("NOTICE %1 :%2CLIENTINFO %3%4%5").arg(originating.section('!', 0, 0)).arg(QChar(0x01)).arg(response).arg(QChar(0x01)).arg("\r\n");
					writeBlock(reply.latin1(), reply.length());
					emit repliedCtcp("CLIENTINFO", originating.section('!', 0, 0), response);
					continue;
				} else if (special == "time")
				{
					QString dateTime = QDateTime::currentDateTime().toString();
					QString reply = QString("NOTICE %1 :%2TIME %3%4%5").arg(originating.section('!', 0, 0)).arg(QChar(0x01)).arg(dateTime).arg(QChar(0x01)).arg("\r\n");
					writeBlock(reply.latin1(), reply.length());
					emit repliedCtcp("TIME", originating.section('!', 0, 0), dateTime);
					continue;
				} else if (special == "source")
				{
					mSourceString.replace(QRegExp("[\\r\\n]*$"), "");
					if (mSourceString.isEmpty())
					{
						mSourceString = "Unknown client, known source.";
					}
					QString reply = QString("NOTICE %1 :%2SOURCE %3%4%5").arg(originating.section('!', 0, 0)).arg(QChar(0x01)).arg(mSourceString).arg(QChar(0x01)).arg("\r\n");
					writeString(reply);
					emit repliedCtcp("TIME", originating.section('!', 0, 0), mSourceString);
					continue;
				} else if (special == "finger")
				{
					// Not implemented yet
				} else if (special == "action")
				{
					message = message.section(' ', 1);
					if (target[0] == '#' || target[0] == '!' || target[0] == '&')
					{
						emit incomingAction(originating, target, message);
					} else {
						emit incomingPrivAction(originating, target, message);
					}
					continue;
				} else if (special == "dcc")
				{
					if (message.section(' ', 1, 1).lower() == "chat")
					{
						// Tells if the conversion went okay to unsigned int
						bool okayHost;
						bool okayPort;
						QHostAddress address(message.section(' ', 3, 3).toUInt(&okayHost));
						unsigned int port = message.section(' ', 4, 4).toUInt(&okayPort);
						if (okayHost && okayPort)
						{
							DCCClient *chatObject = new DCCClient(address, port, 0, DCCClient::Chat);
							emit incomingDccChatRequest(address, port, originating.section('!', 0, 0), *chatObject);
						}
					} else if (message.section(' ', 1, 1).lower() == "send")
					{
						// Tells if the conversion went okay to unsigned int
						bool okayHost;
						bool okayPort;
						QString filename = message.section(' ', 2, 2);
						QFileInfo realfile(filename);
						QHostAddress address(message.section(' ', 3, 3).toUInt(&okayHost));
						unsigned int port = message.section(' ', 4, 4).toUInt(&okayPort);
						unsigned int size = message.section(' ', 5, 5).toUInt();
						if (okayHost && okayPort)
						{
							DCCClient *chatObject = new DCCClient(address, port, message.section(' ', 5, 5).toUInt(), DCCClient::File);
							emit incomingDccSendRequest(address, port, originating.section('!', 0, 0), realfile.fileName(), size, *chatObject);
						}
					}
					continue;
				}
			}
			if (target[0] == '#' || target[0] == '!' || target[0] == '&')
			{
				emit incomingMessage(originating, target, message);
			} else {
				emit incomingPrivMessage(originating, target, message);
			}
			continue;
		}
		else if (command == QString("NOTICE"))
		{
			QString originating = line.section(' ', 0, 0);
			originating.remove(0, 1);
			QString message = line.section(' ', 3);
			message.remove(0,1);
			if (QChar(message[0]).unicode() == 1 && (QChar(message[(message.length() -1)])).unicode() == 1)
			{
				// Fun!
				message = message.remove(0, 1);
				message = message.remove((message.length() -1), 1);
				QString special = message.section(' ', 0, 0);
				if (special.lower() == "ping")
				{
					QString epoch = message.section(' ', 1, 1);
					timeval time;
					if (gettimeofday(&time, 0) == 0)
					{
						QString timeReply = QString("%1.%2").arg(time.tv_sec).arg(time.tv_usec);
						double newTime = timeReply.toDouble();
						if (!epoch.isEmpty())
						{
							double oldTime = epoch.toDouble();
							double difference = newTime - oldTime;
							QString diffString;
							if (difference < 1)
							{
								diffString = QString::number(difference);
								diffString.remove((diffString.find('.') -1), 2);
								diffString.truncate(3);
								diffString.append("msecs");
							} else {
								diffString = QString::number(difference);
								QString seconds = diffString.section('.', 0, 0);
								QString millSec = diffString.section('.', 1, 1);
								millSec.remove(millSec.find('.'), 1);
								millSec.truncate(3);
								diffString = QString("%1 secs %2 msecs").arg(seconds).arg(millSec);
							}
							emit incomingCtcpReply("PING", originating.section('!', 0, 0), diffString);
						}
					}
					continue;
				}
				if (special.lower() == "version")
				{
					emit incomingCtcpReply("VERSION", originating.section('!', 0, 0), message.remove(0, 7));
				}
			}
			kdDebug() << "-" << command << "-" << endl;
			continue;
		} else if (command == QString("PART"))
		{
			/*
			This signal emits when a user parts a channel
			*/
			kdDebug() << "IRC Plugin: User parting" << endl;
			QString originating = line.section(' ', 0, 0);
			originating.remove(0, 1);
			QString target = line.section(' ', 2, 2);
			QString message = line.section(' ', 3);
			message = message.remove(0, 1);
			emit incomingPartedChannel(originating, target, message);
			continue;
		}
		else if (command == QString("QUIT"))
		{
			/*
			This signal emits when a user quits irc
			*/
			kdDebug() << "IRC Plugin: User quiting" << endl;
			QString originating = line.section(' ', 0, 0);
			originating.remove(0, 1);
			QString message = line.section(' ', 2);
			message = message.remove(0, 1);
			emit incomingQuitIRC(originating, message);
			continue;
		}
		else if (command == QString("NICK"))
		{
			/*
			Nick name of a user changed
			*/
			QString oldNick = line.section('!', 0, 0);
			oldNick = oldNick.remove(0,1);
			QString newNick = line.section(' ', 2, 2);
			newNick = newNick.remove(0, 1);
			if (oldNick.lower() == mNickname.lower())
			{
				emit successfullyChangedNick(oldNick, newNick);
				mNickname = newNick;
				continue;
			}
			emit incomingNickChange(oldNick, newNick);
			continue;
		}
		else if (command == QString("TOPIC"))
		{
			/*
			The topic of a channel changed. emit the channel, new topic, and the person who changed it
			*/
			QString channel = line.section(' ', 2, 2);
			QString newTopic = line.section(' ', 3);
			newTopic = newTopic.remove(0, 1);
			QString changer = line.section('!', 0, 0);
			changer = changer.remove(0,1);
			emit incomingTopicChange(channel, changer, newTopic);
			continue;
		}
		else if (command == "MODE")
		{

			//emit incomingGiveOp(nickname, channel);
			//emit incomingTakeOp(nickname, channel);

		}
		else if (command == "KICK")
		{
			QString nickname, channel, reason, by;

			channel = line.section(' ', 2, 2);
			nickname = line.section(' ', 3, 3);
			reason = line.section(':', 2);
			by = line.section('!', 0, 0);
			by.remove(0,1);

			emit incomingKick(nickname, channel, by, reason);
		}
		else if (number.contains(QRegExp("^\\d\\d\\d$")))
		{
			QRegExp getServerText("\\s\\d+\\s+[^\\s]+\\s+:(.*)");
			getServerText.match(line);
			QString message = getServerText.cap(1);
			switch(number.toInt())
			{
				case 375:
				{
					/*
					Beginging the motd. This isn't emitted because the MOTD is sent out line by line
					*/
					emit incomingStartOfMotd();
					waitingFinishMotd = true;
					break;
				}
				case 372:
				{
					/*
					Part of the MOTD.
					*/
					if (waitingFinishMotd == true)
					{
						QString message = line.section(' ', 3);
						message.remove(0, 1);
						emit incomingMotd(message);
					}
					break;
				}
				case 376:
				{
					/*
					End of the motd.
					*/
					emit incomingEndOfMotd();
					waitingFinishMotd = false;
					break;
				}
				case 001:
				{
					if (failedNickOnLogin == true)
					{
						// this is if we had a "Nickname in use" message when connecting and we set another nick. This signal emits that the nick was accepted and we are now logged in
						emit loginNickNameAccepted(pendingNick);
						mNickname = pendingNick;
						failedNickOnLogin = false;
					}
					/*
					Gives a welcome message in the form of:
					"Welcome to the Internet Relay Network
					<nick>!<user>@<host>"
					*/
					emit incomingWelcome(message);
					/*
					At this point we are connected and the server is ready for us to being taking commands although the MOTD comes *after* this.
					*/
					loggedIn = true;
					emit connectedToServer();
					break;
				}
				case 002:
				{
					/*
					Gives information about the host, close to 004 (See case 004) in the form of:
					"Your host is <servername>, running version <ver>"
					*/
					emit incomingYourHost(message);
					break;
				}
				case 003:
				{
					/*
					Gives the date that this server was created (useful for determining the uptime of the server) in the form of:
					"This server was created <date>"
					*/
					emit incomingHostCreated(message);
					break;
				}
				case 004:
				{
					/*
					Gives information about the servername, version, available modes, etc in the form of:
					"<servername> <version> <available user modes>
					<available channel modes>"
					*/
					emit incomingHostInfo(message);
					break;
				}
				case 251:
				{
					/*
					Tells how many user there are on all the different servers in the form of:
					":There are <integer> users and <integer>
					services on <integer> servers"
					*/
					emit incomingUsersInfo(message);
					break;
				}
				case 252:
				{
					/*
					Issues a number of operators on the server in the form of:
					"<integer> :operator(s) online"
					*/
					emit incomingOnlineOps(message);
					break;
				}
				case 253:
				{
					/*
					Tells how many unknown connections the server has in the form of:
					"<integer> :unknown connection(s)"
					*/
					emit incomingUnknownConnections(message);
					break;
				}
				case 254:
				{
					/*
					Tells how many total channels there are on this network in the form of:
					"<integer> :channels formed"
					*/
					emit incomingTotalChannels(message);
					break;
				}
				case 255:
				{
					/*
					Tells how many clients and servers *this* server handles in the form of:
					":I have <integer> clients and <integer>
					servers"
					*/
					emit incomingHostedClients(message);
					break;
				}
				case 311:
				{
					/*
					Show info about a user (part of a /whois) in the form of:
					"<nick> <user> <host> * :<real name>"
					*/
					QString realName = line.section(' ', 7);
					realName.remove(0, 1);
					emit incomingWhoIsUser(line.section(' ', 3, 3), line.section(' ', 4, 4), line.section(' ', 5, 5), realName);
					break;
				}
				case 312:
				{
					/*
					Show info about a server (part of a /whois) in the form of:
					"<nick> <server> :<server info>"
					*/
					QString serverInfo = line.section(' ', 5);
					serverInfo.remove(0, 1);
					emit incomingWhoIsServer(line.section(' ', 3, 3), line.section(' ', 4, 4), serverInfo);
					break;
				}
				case 313:
				{
					/*
					Show info about an operator (part of a /whois) in the form of:
					"<nick> :is an IRC operator"
					*/
					emit incomingWhoIsOperator(line.section(' ', 3, 3));
					break;
				}
				case 317:
				{
					/*
					Show info about someone who is idle (part of a /whois) in the form of:
					"<nick> <integer> :seconds idle"
					*/
					emit incomingWhoIsIdle(line.section(' ', 3, 3), line.section(' ', 4, 4).toULong());
					break;
				}
				case 318:
				{
					/*
					Receive end of WHOIS in the form of
					"<nick> :End of /WHOIS list"
					*/
					emit incomingEndOfWhois(line.section(' ', 3, 3));
					break;
				}
				case 319:
				{
					/*
					Show info a channel a user is logged in (part of a /whois) in the form of:
					"<nick> :{[@|+]<channel><space>}"
					*/
					QString channel = line.section(' ', 4);
					channel.remove(0, 1);
					emit incomingWhoIsChannels(line.section(' ', 3, 3), channel);
					break;
				}
				case 332:
				{
					/*
					Gives the existing topic for a channel after a join
					*/
					QString channel = line.section(' ', 3, 3);
					QString topic = line.section(' ', 4);
					topic = topic.remove(0, 1);
					emit incomingExistingTopic(channel, topic);
					break;
				}
				case 353:
				{
					/*
					Gives a listing of all the people in the channel in the form of:
					"( "=" / "*" / "@" ) <channel>
					:[ "@" / "+" ] <nick> *( " " [ "@" / "+" ] <nick> )
					- "@" is used for secret channels, "*" for private
					channels, and "=" for others (public channels).
					*/
					QString channel = line.section(' ', 4, 4);
					kdDebug() << "IRC Plugin: Case 353: channel == \"" << channel << "\"" << endl;
					QString names = line.section(' ', 5);
					names = names.remove(0, 1);
					kdDebug() << "IRC Plugin: Case 353: names before preprocessing == \"" << names << "\"" << endl;
					QStringList namesList = QStringList::split(' ', names);
					for (QStringList::Iterator it = namesList.begin(); it != namesList.end(); it++)
					{
						if ((*it).lower() == mNickname.lower())
						{
							continue;
						}
						if ((*it)[0] == '@')
						{
							(*it) = (*it).remove(0, 1);
							emit incomingNamesList(channel, (*it), KIRC::Operator);
							continue;
						}
						if ((*it)[0] == '+')
						{
							(*it) = (*it).remove(0,1);
							emit incomingNamesList(channel, (*it), KIRC::Voiced);
							continue;
						}
						emit incomingNamesList(channel, (*it), KIRC::Normal);
						continue;
					}
					break;
				}
				case 366:
				{
					/*
					Gives a signal to indicate that the NAMES list has ended for a certain channel in the form of:
					"<channel> :End of NAMES list"
					*/
					emit incomingEndOfNames(line.section(' ', 3, 3));
					break;
				}
				case 401:
				{
					/*
					Gives a signal to indicate that the command issued failed because the person not being on IRC in the for of:
					"<nickname> :No such nick/channel"
					- Used to indicate the nickname parameter supplied to a
					command is currently unused.
					*/
					emit incomingNoNickChan(line.section(' ', 3, 3));
					break;
				}
				case 406:
					/*
					Like case 401, but when there *was* no such nickname
					*/
					emit incomingWasNoNick(line.section(' ', 3, 3));
					break;
				case 433:
				{
					/*
					Tells us that our nickname is already in use.
					*/
					if (loggedIn == false)
					{
						// This tells us that our nickname is, but we aren't logged in. This differs because the server won't send us a response back telling us our nick changed (since we aren't logged in)
						failedNickOnLogin = true;
						emit incomingFailedNickOnLogin(line.section(' ', 3, 3));
						continue;
					}
					// And this is the signal for if someone is trying to use the /nick command or such when already logged in, but it's already in use
					emit incomingNickInUse(line.section(' ', 3, 3));
					break;
				}
				default:
				{
					/*
					Any other messages the server decides to send us that we don't understand or have implemented (the latter is probably the case)
					*/
					emit incomingUnknown(line);
					break;
				}
			}
		}
		else if ( line.section(' ', 0, 0) == "PING" )
		{
			QString statement = "PONG";

			if ( line.contains(' ') )
				statement.append(" :" + line.section(':', 1, 1));

			statement.append( "\r\n");
			writeString(statement);
			continue;
		}
	}
}

void KIRC::joinChannel(const QString &name)
{
	/*
	This will join a channel
	*/
	if (state() == QSocket::Connected && loggedIn == true)
	{
		QString statement = "JOIN ";
		statement.append(name.data());
		statement.append("\r\n");
		writeString(statement);
	}
}

void KIRC::quitIRC(const QString &reason)
{
	/*
	This will quit IRC
	*/
	if (state() == QSocket::Connected && loggedIn == true && attemptingQuit == false)
	{
		attemptingQuit = true;
		QString statement = "QUIT :";
		statement.append(reason);
		statement.append("\r\n");
		writeString(statement);
	}
}

void KIRC::sendCtcpPing(const QString &target)
{
	timeval time;
	if (gettimeofday(&time, 0) == 0)
	{
		QString timeReply = QString("%1.%2").arg(time.tv_sec).arg(time.tv_usec);
		QString statement = "PRIVMSG ";
		statement.append(target);
		statement.append(" :");
		statement.append(0x01);
		statement.append("PING ");
		statement.append(timeReply);
		statement.append(0x01);
		statement.append("\r\n");
		writeString(statement);
	}
}

void KIRC::sendCtcpVersion(const QString &target)
{
	QString statement = "PRIVMSG ";
	statement.append(target);
	statement.append(" :");
	statement.append(0x01);
	statement.append("VERSION");
	statement.append(0x01);
	statement.append("\r\n");
	writeString(statement);
}

void KIRC::partChannel(const QString &name, const QString &reason)
{
	/*
	This will part a channel with 'reason' as the reason for parting
	*/
	if (state() == QSocket::Connected && loggedIn == true)
	{
		QString statement = "PART ";
		statement.append(name);
		statement.append(" :");
		statement.append(reason);
		statement.append("\r\n");
		writeString(statement);
	}
}

void KIRC::actionContact(const QString &contact, const QString &message)
{
	if (state() == QSocket::Connected && loggedIn == true)
	{
		QString statement = "PRIVMSG ";
		statement.append(contact);
		statement.append(" :");
		statement.append(0x01);
		statement.append("ACTION ");
		statement.append(message);
		statement.append(0x01);
		statement.append("\r\n");
		writeString(statement);
                if (contact[0] != '#' && contact[0] != '!' && contact[0] != '&')
                {
                        emit incomingPrivAction(mNickname, contact, message);
                } else {
			emit incomingAction(mNickname, contact, message);
		}
	}
}

void KIRC::requestDccConnect(const QString &nickname, const QString &filename, unsigned int port, DCCClient::Type type)
{
	if (state() == QSocket::Connected && loggedIn == true)
	{
		/* Please do not pick on me about this if it isn't portable. Submit a patch and I'll glady apply it. This is *needed* for
		sending the connecting client the proper IP address that they need to connect to. */
		struct sockaddr_in name;
		int sockfd = socket();
		socklen_t len = sizeof(name);
		if (getsockname(sockfd, (struct sockaddr *)&name,&len) == 0)
		{
			// refer to the ntohl man page for more info, but what it basicly does is flips the numbers around (e.g. from 1.0.0.10 to 10.0.0.1) since it's in "network order" right now
			QHostAddress host(ntohl(name.sin_addr.s_addr));
			if (type == DCCClient::Chat)
			{
				QString message = QString("PRIVMSG %1 :%2DCC CHAT chat %3 %4%5\r\n").arg(nickname).arg(QChar(0x01)).arg(host.ip4Addr()).arg(port).arg(QChar(0x01));
				writeString(message);
			} else if (type == DCCClient::File)
			{
				QFileInfo file(filename);
				QString noWhiteSpace = file.fileName();
				if (noWhiteSpace.contains(' ') > 0)
				{
					noWhiteSpace.replace(QRegExp("\\s+"), "-");
				}
				QString message = QString("PRIVMSG %1 :%2DCC SEND %3 %4 %5 %6 %7\r\n").arg(nickname).arg(QChar(0x01)).arg(noWhiteSpace).arg(host.ip4Addr()).arg(port).arg(file.size()).arg(QChar(0x01));
				writeString(message);
			}
		}
	}
}

void KIRC::messageContact(const QString &contact, const QString &message)
{
	if (state() == QSocket::Connected && loggedIn == true)
	{
		QString statement = "PRIVMSG ";
		statement.append(contact);
		statement.append(" :");
		statement.append(message);
		statement.append("\r\n");
		writeString(statement);
		if (contact[0] != '#' && contact[0] != '!' && contact[0] != '&')
		{
			emit incomingPrivMessage(mNickname, contact, message);
		} else {
			emit incomingMessage(mNickname, contact, message);
		}
	}
}

void KIRC::slotConnectionClosed()
{
	kdDebug() << "IRC Plugin: Connection Closed" << endl;
	loggedIn = false;
	if (attemptingQuit == true)
	{
		emit successfulQuit();
	}
	attemptingQuit = false;
}

void KIRC::changeNickname(const QString &newNickname)
{
	if (loggedIn == false)
	{
		failedNickOnLogin = true;
	}
	pendingNick = newNickname;
	QString newString = "NICK ";
	newString.append(newNickname);
	newString.append("\r\n");
	writeString(newString);
}

void KIRC::slotHostFound()
{
	kdDebug() << "IRC Plugin: Host Found" << endl;
}

void KIRC::slotConnected()
{
	kdDebug() << "IRC Plugin: Connected" << endl;
	// Just a test for now:
	QString ident = "USER ";
	ident.append(mUsername);
	ident.append(" 127.0.0.1 ");
	ident.append(mHost);
	ident.append(" :Using Kopete IRC Plugin 0.1 ");
	ident.append("\r\nNICK ");
	ident.append(mNickname);
	ident.append("\r\n");
	writeString(ident);
}

void KIRC::connectToServer(const QString host, Q_UINT16 port, const QString username, const QString nickname)
{
	mUsername = username;
	mNickname = nickname;
	mHost = host;
	connectToHost(host.latin1(), port);
	emit connecting();
}


void KIRC::setTopic(const QString &channel, const QString &topic)
{
	QString command;
	command = "TOPIC " + channel + " :" + topic + "\r\n";

	writeString(command);
}

void KIRC::kickUser(const QString &user, const QString &channel, const QString &reason)
{
	QString command = "KICK " + channel + " " + user + " :" + reason + "\r\n";

	writeString(command);
}

void KIRC::whoisUser(const QString &user)
{
	QString command = "WHOIS " + user + "\r\n";

	writeString(command);
}


#include "kirc.moc"
