/***************************************************************************
                          irccontact.h  -  description
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

#ifndef IRCCONTACT_H
#define IRCCONTACT_H

#include <qlistview.h>
#include "imcontact.h"
#include <qobject.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qtimer.h>
#include "kopete.h"
#include <kpopupmenu.h>

#include "ircprotocol.h"
#include "ircchatview.h"
#include "kirc.h"

class IRCQueryView;
class QVBox;
class QStringList;

class IRCContact : public IMContact
{
	Q_OBJECT
public:
	IRCContact(QListViewItem *parent, const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact);
	IRCContact(QListViewItem *parent, const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, const QStringList pengingMessages);
	virtual void rightButtonPressed(const QPoint &);
	virtual void leftButtonDoubleClicked();
	KIRC *engine;
	bool waitingPart;
	bool requestedQuit;
	void unloading();
	IRCServerContact *mContact;
	QVBox *tabPage() { return mTabPage; };
	IRCChatView *getChatView() { return chatView; };
	QStringList mPendingMessage;
private:
	QString mServer;
	QString mTarget;
	unsigned int mPort;
	QString mUsername;
	QString mNickname;
	bool mJoinOnConnect;
	IRCChatView *chatView;
	KPopupMenu *popup;
	QVBox *mTabPage;
	IRCQueryView *queryView;
	void init();
private slots:
	void slotIncomingMotd(const QString &);
	void slotConnectedToHost();
	void slotUserJoinedChannel(const QString &, const QString &);
	void slotNamesList(const QString &, const QString &, int);
	void joinNow();
	void slotQuitServer();
	void slotHop();
	void slotPartedChannel(const QString &, const QString &, const QString &);
	void slotServerIsQuitting();
	void slotServerHasQuit();
	void slotRemoveThis();
	void slotServerQuit();
public slots:
	void slotPart();
};

#endif
