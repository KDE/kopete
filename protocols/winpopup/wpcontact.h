/***************************************************************************
                          wpcontact.h  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __WPCONTACT_H
#define __WPCONTACT_H

// KDE Includes
#include <kaction.h>

// Qt Includes
#include <qvaluestack.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcursor.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qstringlist.h>

// Kopete Includes
#include "kopetecontact.h"
#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemessagemanager.h"
#include "kopetehistorydialog.h"
#include "kopetemessage.h"

// Local Includes
#include "wpprotocol.h"

class QTimer;
class QPixmap;
class QListView;
class QListViewItem;
class KPopupMenu;
class KAction;
class KListAction;
class KopeteMetaContact;

class WPProtocol;

class WPContact: public KopeteContact
{
	Q_OBJECT
private:
	WPProtocol *mProtocol;
	QString mUserID;
	enum
	{
		STATUS_OFFLINE = 0,
		STATUS_ONLINE,
		STATUS_AWAY
	};
	int mStatus;

	KopeteMetaContact *myMetaContact;

	QTimer checkStatus;
	KPopupMenu *popup;
	KopeteMessageManager *mMsgManagerKCW, *mMsgManagerKEW;
	KopeteMessageManager *msgManagerKCW(), *msgManagerKEW();

	KAction *actionRemove, *actionRemoveFromGroup, *actionChat, *actionMessage, *actionInfo, *actionHistory, *actionRename;
	KListAction *actionContactMove;

//	dlgWPRename *renameDialog;
	KopeteHistoryDialog *historyDialog;

	void initActions();

public:
	WPContact(const QString &userID, WPProtocol *protocol, KopeteMetaContact *parent );

	ContactStatus status() const;
	QString statusText() const;
	QString statusIcon() const;

	int importance() const;
	void execute();

	QString userID() const { return mUserID; }

//	virtual void showContextMenu(const QPoint&, const QString&);
	KActionCollection *myActionCollection;
	KActionCollection *customContextMenuActions() { if(myActionCollection) delete myActionCollection; return myActionCollection = new KActionCollection(this); }

	/**
	 * Return whether or not this contact is REACHABLE.
	 * Useful in determining if the contact is able to
	 * recieve messages even if offline, etc.
	 */
	bool isReachable() { return mStatus != STATUS_OFFLINE; }

	virtual QString id() const;
	virtual QString data() const;

public slots:
	void slotNewMessage(const QString &Body, const QDateTime &Arrival);
	void slotSendMsgKEW(const KopeteMessage&);
	void slotSendMsgKCW(const KopeteMessage&);

	void moveToGroup(const QString &from, const QString &to);

	/**
	 * Method to delete a contact from the contact list,
	 * should be implemented by protocol plugin to handle
	 * protocol-specific actions required to delete a contact
	 * (ie. messages to the server, etc)
	 */
	void slotDeleteContact();

	/**
	 * Method to retrieve user information.  Should be implemented by
	 * the protocols, and popup some sort of dialog box
	 */
	void slotUserInfo();

private slots:
	void slotUpdateContact(QString, int);
	void slotChatThisUser();
	void slotEmailUser();

	void slotCheckStatus();

	void slotCloseHistoryDialog();
	void slotViewHistory();

signals:
	void statusChanged();
	void msgRecieved(QString, QString, QString, QString, QFont, QColor);
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

