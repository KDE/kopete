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

#include "dcchandler.h"

#include <qstring.h>
#include <qstringlist.h>



class IRCChatWindow;
class IRCCmdParser;
class IRCConsoleView;
class IRCMessage;
class IRCProtocol;
class IRCServerManager;
class KIRC;

class KPopupMenu;

class QVBox;



class IRCServerContact : public QObject
{
Q_OBJECT
public:
	IRCServerContact(const QString &, const QString &, bool, IRCProtocol *protocol);
	IRCServerContact(IRCProtocol *protocol, bool connectNow = true);
	KIRC *engine() { return m_engine; };
	const QString &nickName() { return m_nickname; };

	const QString &serverName() { return m_serverName; };
    const QString &quitMessage() { return mQuitMessage; };

	void setServerName( const QString &name ) { m_serverName = name; };
	void setQuitMessage( const QString &msg ) { mQuitMessage = msg; };
	
	void newNickname(const QString &newNick);
	IRCServerManager *serverManager() { return m_serverManager; };
	IRCChatWindow *chatWindow() { return m_ircChatWindow; };
	IRCMessage *messenger() { return m_messenger; };
	IRCCmdParser *parser() { return m_parser; };
	QStringList activeContacts() { return m_activeContacts; };
	bool parentClosing();

	bool tryingQuit() { return m_tryingQuit; };
	bool closing() { return m_closing;};
	void setClosing( bool c ) { m_closing = c;};

	IRCProtocol *protocol() { return m_protocol; };
	IRCConsoleView *consoleView() { return m_consoleView; };
	QVBox *tabView() { return mTabView ;};
	void initiateDcc(const QString &nickname, const QString &, DCCServer::Type type);
	QString id() const;

private:
	KIRC *m_engine;
	QString mQuitMessage;
	IRCMessage *m_messenger;
	IRCCmdParser *m_parser;
	QStringList m_activeContacts;
	QVBox *mTabView;
	bool m_tryingQuit;
	bool m_closing;

	QString m_serverName;
	IRCChatWindow *m_ircChatWindow;
	IRCServerManager *m_serverManager;
	IRCConsoleView *m_consoleView;
	QString m_nickname;
	IRCProtocol *m_protocol;
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
	void incomingDccSendRequest(const QHostAddress &, unsigned int port, const QString &nickname, const QString &, unsigned int, DCCClient &chatObject);
public slots:
	void slotQuitServer();
	void slotConnectNow();
	void slotDisconnectNow();
signals:
	void serverQuit();
	void connecting();
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

