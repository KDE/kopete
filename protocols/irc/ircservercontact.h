/***************************************************************************
                          ircservercontact.h  -  description
                             -------------------
    begin                : Wed Mar 6 2002
    copyright            : (C) 2002 by nbetcher
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

#ifndef IRCSERVERCONTACT_H
#define IRCSERVERCONTACT_H

#include <qobject.h>
#include <qstring.h>
#include "kopete.h"
#include <klistview.h>
#include <qtimer.h>
#include <ircchatwindow.h>
#include <qdict.h>
#include <qptrlist.h>
#include "irccontact.h"

class KIRC;
class IMContact;
class QVBox;
class IRCConsoleView;
class KPopupMenu;
class QStringList;
class IRCCmdParser;
class IRCServerManager;
class IRCMessage;
class IRCProtocol;
class DCCChat;

class IRCServerContact : public QObject
{
Q_OBJECT
public:
	IRCServerContact(const QString &, const QString &, bool, IRCProtocol *protocol);
	IRCServerContact(IRCProtocol *protocol, bool connectNow = true);
	KIRC *engine;
	const QString &nickName() { return mNickname; };
	const QString &serverName() { return mServer; };
	void newNickname(const QString &newNick);
	IRCServerManager *mManager;
	IRCChatWindow *mWindow;
	QString mNickname;
	QString mServer;
	bool parentClosing();
	QString mQuitMessage;
	IRCMessage *messenger;
	IRCCmdParser *parser;
	QStringList activeContacts;
	QVBox *mTabView;
	bool tryingQuit;
	bool closing;
	IRCProtocol *mProtocol;
	IRCConsoleView *consoleView() { return mConsoleView; };
	void initiateDcc(const QString &nickname, DCCServer::Type type);
private:
	IRCConsoleView *mConsoleView;
	KPopupMenu *popup;
	void init();
private slots:
	void nickInUseOnLogin(const QString &);
	void slotChangedNick(const QString &, const QString &);
	void slotServerHasQuit();
	void forceDisconnect();
	void updateToolbar();
	void incomingPrivMessage(const QString &, const QString &, const QString &);
	void incomingPrivAction(const QString &, const QString &, const QString &);
	void incomingDccChatRequest(const QHostAddress &, unsigned int port, const QString &nickname, DCCClient &chatObject);
public slots:
	void slotQuitServer();
	void connectNow();
	void promptChannelJoin();
	void disconnectNow();
signals:
	void serverQuit();
	void connecting();
};

#endif
