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
#include <imcontact.h>
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

class IRCServerContact : public IMContact
{
Q_OBJECT
public:
	IRCServerContact(const QString &, const QString &, bool, IRCServerManager *);
	IRCServerContact(IRCServerManager *manager);
	KIRC *engine;
	const QString &nickName() { return mNickname; };
	const QString &serverName() { return mServer; };
	void newNickname(const QString &newNick);
	IRCServerManager *mManager;
	IRCChatWindow *mWindow;
	QString mNickname;
	QString mServer;
	bool parentClosing();
	virtual void rightButtonPressed(const QPoint &);
	virtual void leftButtonDoubleClicked();
	QString mQuitMessage;
	QStringList activeQueries;
	IRCMessage *messenger;
	IRCCmdParser *parser;
	QPtrList<IRCContact> activeContacts;
	void setmWindow(IRCChatWindow *parent);
	QVBox *mTabView;
	bool tryingQuit;
	bool closing;
private:
	IRCConsoleView *mConsoleView;
	KPopupMenu *popup;
private slots:
	void nickInUseOnLogin(const QString &);
	void slotChangedNick(const QString &, const QString &);
	void slotServerHasQuit();
	void forceDisconnect();
	void updateToolbar();
	void incomingMessage(const QString &, const QString &, const QString &);
	void incomingAction(const QString &, const QString &, const QString &);
public slots:
	void slotQuitServer();
	void connectNow();
	void promptChannelJoin();
	void disconnectNow();
signals:
	void quittingServer();
	void serverQuit();
	void connecting();
};

#endif
