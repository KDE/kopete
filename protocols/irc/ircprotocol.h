/***************************************************************************
                          msnprotocol.h  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IRCPROTOCOL_H
#define IRCPROTOCOL_H

#include <qpixmap.h>
#include <qwidget.h>
#include <qstringlist.h>
#include "ircpreferences.h"
#include <statusbaricon.h>
#include <addcontactpage.h>
#include <improtocol.h>
#include "kirc.h"
#include "ircservermanager.h"
#include "ircservercontact.h"

/**
  *@author duncan
  */

class IRCProtocol : public QObject, public IMProtocol
{
Q_OBJECT
public:
	IRCProtocol();
	~IRCProtocol();
	/* Plugin reimplementation */
	void init();
	bool unload();
	/** IMProtocol reimplementation */
	virtual QPixmap getProtocolIcon();
	virtual AddContactPage *getAddContactWidget(QWidget *parent);
	virtual void Connect();
	virtual void Disconnect();
	virtual bool isConnected();
	/** Internal */
	StatusBarIcon *statusBarIcon;
	/** The IRC Engine */
	QPixmap protocolIcon;
	QPixmap onlineIcon;
	QPixmap offlineIcon;
	QPixmap awayIcon;
	void addContact(const QString &, const QString &, bool, bool);
	bool mIsConnected;
	KIRC *engine;
	QPixmap joinIcon;
	QPixmap privmsgIcon;
	IRCServerManager *manager;
private:
	void initIcons();
public slots: // Public slots
  /** No descriptions */
private slots:
	void slotIncomingMotd(const QString &);
	void slotConnectedToHost();
	void slotUserJoinedChannel(const QString &, const QString &);
	void slotNamesList(const QString &, const QString &, int);
};

#endif
