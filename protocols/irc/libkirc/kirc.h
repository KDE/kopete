/***************************************************************************
                          kirc.cpp  -  description
                             -------------------
    begin                : Wed Dec 26 2001
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

#ifndef KIRC_H
#define KIRC_H

#include <qsocket.h>
#include "dcchandler.h"
#include <qstring.h>

class QHostAddress;

/**
  *@author nbetcher
  */

class KIRC : public QSocket {
Q_OBJECT
public:
	KIRC();
	~KIRC();
	void connectToServer(const QString host, Q_UINT16 port, const QString, const QString);
	void joinChannel(const QString &);
	void messageContact(const QString &contact, const QString &message);
	void actionContact(const QString &contact, const QString &message);
	QString &userName() { return mUsername; };
	QString &nickName() { return mNickname; };
	QString &host() { return mHost; };
	bool isLoggedIn() { return loggedIn; };
	void changeNickname(const QString &newNickname);
	void partChannel(const QString &name, const QString &reason);
	void quitIRC(const QString &reason);
	void setVersionString(const QString &newString) { mVersionString = newString; };
	void setUserString(const QString &userString) { mUserString = userString; };
	void setSourceString(const QString &sourceString) { mSourceString = sourceString; };
	void sendCtcpPing(const QString &target);
	void sendCtcpVersion(const QString &target);
	void setTopic(const QString &channel, const QString &topic);
	void kickUser(const QString &user, const QString &channel, const QString &reason);
	void whoisUser(const QString &user);
	void requestDccConnect(const QString &, const QString &, unsigned int port, DCCClient::Type type);
	void sendNotice(const QString &target, const QString &message);
enum UserClass
{
	Normal = 0,
	Operator = 1,
	Voiced = 2
};
private:
	Q_LONG writeString(const QString &str);

private slots:
	void slotHostFound();
	void slotConnected();
	void slotConnectionClosed();
	void slotReadyRead();
	void slotBytesWritten(int);
	void slotError(int);
signals:
	void connecting();
	void incomingMotd(const QString &motd);
	void incomingWelcome(const QString &welcome);
	void incomingYourHost(const QString &hostInfo);
	void connectedToServer(); // This is only on successful login. connected() is QSocket's signal if you want to tell when the TCP connection is ACK'ed
	void incomingHostCreated(const QString &info);
	void incomingHostInfo(const QString &hostInfo);
	void incomingUsersInfo(const QString &users);
	void incomingOnlineOps(const QString &ops);
	void incomingUnknownConnections(const QString &unknown);
	void incomingTotalChannels(const QString &amount);
	void incomingHostedClients(const QString &);
	void userJoinedChannel(const QString &user, const QString &channel);
	void incomingNamesList(const QString &channel, const QString &nickname, const int);
	void incomingMessage(const QString &originating, const QString &target, const QString &message);
	void incomingEndOfNames(const QString &channel);
	void incomingEndOfMotd();
	void incomingStartOfMotd();
	void incomingPartedChannel(const QString &user, const QString &channel, const QString &reason);
	void incomingQuitIRC(const QString &user, const QString &reason);
	void incomingAction(const QString &originating, const QString &target, const QString &message);
	void incomingNickInUse(const QString &usingNick);
	void successfullyChangedNick(const QString &, const QString &);
	void incomingNickChange(const QString &, const QString &);
	void loginNickNameAccepted(const QString &);
	void incomingFailedNickOnLogin(const QString &);
	void incomingTopicChange(const QString &, const QString &, const QString &);
	void incomingExistingTopic(const QString &, const QString &);
	void successfulQuit();
	void incomingNoNickChan(const QString &);
	void incomingWasNoNick(const QString &);
	void incomingWhoIsUser(const QString &nickname, const QString &username, const QString &hostname, const QString &realname);
	void incomingWhoIsServer(const QString &nickname, const QString &server, const QString &serverInfo);
	void incomingWhoIsOperator(const QString &nickname);
	void incomingWhoIsIdle(const QString &nickname, unsigned long seconds);
	void incomingWhoIsChannels(const QString &nickname, const QString &channel);
	void incomingUnknown(const QString &);
	void incomingPrivAction(const QString &, const QString &, const QString &);
	void incomingPrivMessage(const QString &, const QString &, const QString &);
	void repliedCtcp(const QString &type, const QString &target, const QString &messageSent);
	void incomingCtcpReply(const QString &type, const QString &target, const QString &messageReceived);
	void incomingKick(const QString &, const QString &, const QString &, const QString &);
	void incomingDccChatRequest(const QHostAddress &, unsigned int port, const QString &nickname, DCCClient &chatObject);
	void incomingDccSendRequest(const QHostAddress &, unsigned int port, const QString &nickname, const QString &, unsigned int, DCCClient &chatObject);
	void incomingEndOfWhois(const QString &nickname);
	void incomingNotice(const QString &originating, const QString &message);
private:
	bool waitingFinishMotd;
	bool loggedIn;
	QString mUsername;
	QString mNickname;
	QString mHost;
	bool failedNickOnLogin;
	QString pendingNick;
	bool attemptingQuit;
	QString mVersionString;
	QString mUserString;
	QString mSourceString;
};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
