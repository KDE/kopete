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

KIRC::KIRC()
	: QSocket()
{
	waitingFinishMotd = false;
	loggedIn = false;
	QObject::connect(this, SIGNAL(hostFound()), this, SLOT(slotHostFound()));
	QObject::connect(this, SIGNAL(connected()), this, SLOT(slotConnected()));
	QObject::connect(this, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	QObject::connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	QObject::connect(this, SIGNAL(bytesWritten(int)), this, SLOT(slotBytesWritten(int)));
	QObject::connect(this, SIGNAL(error(int)), this, SLOT(slotError(int)));
	mNickname = "";
	mUsername = "";
	mHost = "";
}

KIRC::~KIRC()
{

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
		QString line = readLine();
		QString number = line.mid((line.find(' ', 0, false)+1), 3);
		int commandIndex = line.find(' ', 0, false);
		QString command = line.section(' ', 1, 1);
		kdDebug() << "IRC Plugin: command == \"" << command << "\"" << endl;
		if (command == QString("JOIN"))
		{
			/*
			This is the response of someone joining a channel. Remember that this will be emitted when *you* /join a room for the first time
			*/
			kdDebug() << "IRC Plugin: userJoinedChannel emitting" << endl;
			QString channel = line.mid((line.findRev(':')+1), (line.length()-1));
			channel.replace(QRegExp("[\\r\\n]*$"), "");
			emit userJoinedChannel(line.mid(1, (commandIndex-1)), channel);
		}
		if (command == QString("PRIVMSG"))
		{
			/*
			This is a signal that indicates there is a new message. This can be either from a channel or from a specific user.
			*/
			kdDebug() << "IRC Plugin: We have a message" << endl;
			QString originating = line.section(' ', 0, 0);
			originating.remove(0, 1);
			QString target = line.section(' ', 2, 2);
			QString message = line.section(' ', 3);
			message.replace(QRegExp("[\\r\\n]*$"), "");
			message = message.remove(0, 1);
			emit incomingMessage(originating, target, message);
		}
		if (command == QString("PART"))
		{
			/*
			This signal emits when a user parts a channel
			*/
			kdDebug() << "IRC Plugin: User parting" << endl;
			QString originating = line.section(' ', 0, 0);
			originating.remove(0, 1);
			QString target = line.section(' ', 2, 2);
			QString message = line.section(' ', 3);
			message.replace(QRegExp("[\\r\\n]*$"), "");
			message = message.remove(0, 1);
			emit incomingPartedChannel(originating, target, message);
		}
		if (number.contains(QRegExp("^\\d\\d\\d$")))
		{
			QRegExp getServerText("\\s\\d+\\s+[^\\s]+\\s+:(.*)");
			getServerText.match(line);
			QString message = getServerText.cap(1);
			message.replace(QRegExp("[\\r\\n]*$"), "");
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
						waitingFinishMotd = false;
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
					names.replace(QRegExp("[\\r\\n]*$"), "");
					names = names.remove(0, 1);
					kdDebug() << "IRC Plugin: Case 353: names before preprocessing == \"" << names << "\"" << endl;
					QStringList namesList = QStringList::split(' ', names);
					for (QStringList::Iterator it = namesList.begin(); it != namesList.end(); it++)
					{
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
			}
		}
	}
}

void KIRC::joinChannel(const QCString &name)
{
	/*
	This will join a channel
	*/
	if (state() == QSocket::Connected && loggedIn == true)
	{
		QCString statement = "JOIN ";
		statement.append(name.data());
		statement.append("\r\n");
		writeBlock(statement.data(), statement.length());
	}
}

void KIRC::messageContact(const QCString &contact, const QCString &message)
{
	if (state() == QSocket::Connected && loggedIn == true)
	{
		QCString statement = "PRIVMSG ";
		statement.append(contact);
		statement.append(" :");
		statement.append(message);
		statement.append("\r\n");
		writeBlock(statement.data(), statement.length());
		// Ahhhhhhhh, fix this ASAP:
		emit incomingMessage(QString("Error403-"), contact, message);
	}
}

void KIRC::slotConnectionClosed()
{
	kdDebug() << "IRC Plugin: Connection Closed" << endl;
	loggedIn = false;
}

void KIRC::slotHostFound()
{
	kdDebug() << "IRC Plugin: Host Found" << endl;
}

void KIRC::slotConnected()
{
	kdDebug() << "IRC Plugin: Connected" << endl;
	emit connectedToServer();
	// Just a test for now:
	QString ident = "USER ";
	ident.append(mUsername);
	ident.append(" 127.0.0.1 ");
	ident.append(mHost);
	ident.append(" :Using Kopete IRC Plugin 0.1 ");
	ident.append("\r\nNICK ");
	ident.append(mNickname);
	ident.append("\r\n");
	writeBlock(ident.latin1(), ident.length());
}

void KIRC::connectToServer(const QString host, Q_UINT16 port, const QString &username, const QString &nickname)
{
	mUsername = username;
	mNickname = nickname;
	mHost = host;
	connectToHost(host.latin1(), port);
}
