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
#include "kopeteprotocol.h"
#include "kirc.h"

class IRCServerContact;
class IRCServerManager;
class KPopupMenu;
class KSimpleConfig;

/**
  *@author duncan
  */

class IRCProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	IRCProtocol( QObject *parent, const char *name, const QStringList &args );
	~IRCProtocol();
	/* Plugin reimplementation */
	void init();
	bool unload();
	/** KopeteProtocol reimplementation */
	virtual QString protocolIcon() const;
	virtual AddContactPage *createAddContactWidget(QWidget *parent);
	virtual void Connect();
	virtual void Disconnect();
	virtual bool isConnected() const;
	virtual void setAway(void);
	virtual void setAvailable(void);
	virtual bool isAway(void) const;
	/** Internal */
	StatusBarIcon *statusBarIcon;

	/** The IRC Engine */
	QPixmap protocolSmallIcon;
	QPixmap onlineIcon;
	QPixmap offlineIcon;
	QPixmap awayIcon;
	void addContact(const QString &, const QString &, const QString &, bool, bool);
	bool mIsConnected;
	KIRC *engine;
	QPixmap joinIcon;
	QPixmap privmsgIcon;
	IRCServerManager *manager;
	KSimpleConfig *mConfig;
private:
	void initIcons();
	KPopupMenu *popup;
public slots: // Public slots
  /** No descriptions */
private slots:
	void slotIconRightClicked(const QPoint);
	void slotNewConsole();
};

#endif
