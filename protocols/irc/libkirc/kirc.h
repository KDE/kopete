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
#include <qstring.h>

/**
  *@author nbetcher
  */

class KIRC : public QSocket {
Q_OBJECT
public:
	KIRC();
	~KIRC();
	void connectToServer(const QString host, Q_UINT16 port, const QString, const QString);
	void joinChannel(const QCString &);
	void messageContact(const QCString &contact, const QCString &message);
	void actionContact(const QString &contact, const QString &message);
	QString &userName() { return mUsername; };
	QString &nickName() { return mNickname; };
	QString &host() { return mHost; };
	bool isLoggedIn() { return loggedIn; };
	void changeNickname(const QString &newNickname);
	void partChannel(const QCString &name, const QString &reason);
	void quitIRC(const QString &reason);
enum UserClass
{
	Normal = 0,
	Operator = 1,
	Voiced = 2
};
private slots:
	void slotHostFound();
	void slotConnected();
	void slotConnectionClosed();
	void slotReadyRead();
	void slotBytesWritten(int);
	void slotError(int);
signals:
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
private:
	bool waitingFinishMotd;
	bool loggedIn;
	QString mUsername;
	QString mNickname;
	QString mHost;
	bool failedNickOnLogin;
	QString pendingNick;
	bool attemptingQuit;
};

#endif
