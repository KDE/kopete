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
#include "kopetecontact.h"

class IRCQueryView;
class QVBox;
class QStringList;
class IRCServerContact;
class KAction;
class KListAction;

class IRCContact : public KopeteContact
{
	Q_OBJECT
public:
	IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact);
	IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, const QStringList pengingMessages);
	IRCContact(const QString &groupName, const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact);
	~IRCContact();
	// KopeteContact virtual functions
	virtual ContactStatus status() const;
	virtual QString statusIcon() const;
	virtual void execute();
	virtual void showContextMenu(QPoint, QString);
	KIRC *engine;
	bool waitingPart;
	bool requestedQuit;
	IRCServerContact *mContact;
	QVBox *tabPage() { return mTabPage; };
	IRCChatView *getChatView() { return chatView; };
	QStringList mPendingMessage;
	QString mServer;
	QString mTarget;
	QString mGroupName;
private:
	void initActions();
	unsigned int mPort;
	QString mUsername;
	QString mNickname;
	bool mJoinOnConnect;
	IRCChatView *chatView;
	KPopupMenu *popup;
	QVBox *mTabPage;
	IRCQueryView *queryView;
	void init();

	KAction* actionAddGroup;
	KListAction *actionContactMove;
	KAction* actionRemove;

private slots:
//	void slotHop();
	void slotPartedChannel(const QString &, const QString &, const QString &);
	void slotRemoveThis();
	void slotOpen();
	void slotOpenConnect();
	void incomingPrivMessage(const QString &, const QString &, const QString &);
	void incomingPrivAction(const QString &, const QString &, const QString &);
	void slotGroupRemoved(const QString &);

public slots:
	void slotMoveThisUser();

	void slotPart();
	void joinNow();
	void unloading();
};

#endif
