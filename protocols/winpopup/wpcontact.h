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

// Local Includes
#include "wpprotocol.h"

// Kopete Includes
#include <kopetecontact.h>
#include <kopete.h>
#include <contactlist.h>
#include <kopetemessagemanagerfactory.h>
#include <kopetemessagemanager.h>
#include <kopetehistorydialog.h>
#include <kopetemessage.h>

// Qt Includes
#include <qvaluestack.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcursor.h>
#include <qptrlist.h>
#include <qtimer.h>

// KDE Includes

class QTimer;
class QPixmap;
class QListView;
class QListViewItem;
class KPopupMenu;
class KAction;
class KListAction;

class WPProtocol;

class WPContact: public KopeteContact
{
	Q_OBJECT
private:
	WPProtocol *mProtocol;
	QString mUserID, mGroup;
	enum
	{
		STATUS_OFFLINE = 0,
		STATUS_ONLINE,
		STATUS_AWAY
	};
	int mStatus;

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
	WPContact(const QString &userID, const QString &name, const QString &group, WPProtocol *protocol);

	ContactStatus status() const;
	QString statusText() const;
	QString statusIcon() const;

	int importance() const;
	void execute();

	QString group() const { return mGroup; }
	QString userID() const { return mUserID; }

	virtual void showContextMenu(QPoint, QString);

public slots:
	void slotNewMessage(const QString &Body, const QDateTime &Arrival);
	void slotSendMsgKEW(const KopeteMessage&);
	void slotSendMsgKCW(const KopeteMessage&);
	void slotRemoveFromGroup();

private slots:
	void slotUpdateContact(QString, int);
	void slotDeleteMySelf(bool);
	void slotRemoveThisUser();
	void slotRenameContact();
	void slotDoRenameContact();
	void slotMoveThisUser();
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

