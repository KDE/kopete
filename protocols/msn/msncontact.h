/***************************************************************************
                          msncontact.h  -  MSN Contact
                             -------------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MSNCONTACT_H
#define MSNCONTACT_H

#include "kopetecontact.h"
#include "msnprotocol.h"
#include "historydialog.h"
#include <kmsnchatservice.h>

#include <kopete.h>
#include <contactlist.h>

#include <qvaluestack.h>


class QTimer;
class QPixmap;
class QListView;
class QListViewItem;
class KPopupMenu;
class KAction;
class KListAction;

/* Not usefull yet */
struct MSNMessageStruct
{
	QString userid;
	QString message;
};

class MSNProtocol;

class MSNContact : public KopeteContact
{
	Q_OBJECT
	public:
		MSNContact(QString userid, const QString name, QString group, MSNProtocol *protocol);
		
		void initContact(QString userid, const QString name, MSNProtocol *protocol);
		virtual void showContextMenu(QPoint);
		virtual void execute();

		QString mUserID;
		QString mName;
		bool hasLocalGroup;

		ContactStatus status() const;
		QString statusText() const;
		QString statusIcon() const;
		int importance() const;

	public slots:
		void slotContactRemoved(QString, QString);
		void slotChatThisUser();

	private slots:
		void slotRemoveThisUser();
		void slotCopyThisUser();
		void slotMoveThisUser();
		void slotRemoveFromGroup();
		
		void slotUpdateContact (QString, uint);
		// We have to delete the contact if MSN disconenct
		// We will use the engine signal
		void slotDeleteMySelf ( bool );

		void slotHistoryDialogClosing();
		void slotCloseHistoryDialog();
		void slotViewHistory();


	private:
		void initActions();

		QString mGroup;
		uint mStatus;
		MSNProtocol *mProtocol;
		KopeteHistoryDialog *historyDialog;
		KPopupMenu *popup;
		KAction* actionRemove;
		KAction* actionRemoveFromGroup;
		KAction* actionChat;
		KAction* actionInfo;
		KAction* actionHistory;
		KListAction *actionContactMove;
		KListAction *actionContactCopy;

	signals:
		void chatToUser( QString );
};

#endif
