/***************************************************************************
                          jabbercontact.h  -  description
                             -------------------
    begin                : Fri Apr 12 2002
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

#ifndef JABBERCONTACT_H
#define JABBERCONTACT_H

#include "kopetecontact.h"
#include "jabberprotocol.h"
#include "jabbermessagedialog.h"
#include "dlgrename.h"

#include <kopete.h>
#include <contactlist.h>

#include <qvaluestack.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcursor.h>


class QTimer;
class QPixmap;
class QListView;
class QListViewItem;
class KPopupMenu;
class KAction;
class KListAction;

class JabberProtocol;

class JabberContact : public KopeteContact
{
	Q_OBJECT
	public:
		JabberContact (QString userid, QString name, QString group, JabberProtocol *protocol);
		
		void initContact (QString userid, QString name);

		QString mUserID;
		QString mName;
		QString mResource;
		bool hasLocalGroup;

		ContactStatus status() const;
		QString statusText() const;
		QString statusIcon() const;
    int importance() const;
    void execute();

		void setResource(QString);
    QString userID();
    QString nickname();

	public slots:
    void slotMWClosing(void);
    void slotNewMessage(QString, QString);
	virtual void showContextMenu (QPoint, QString);

	private slots:
		void slotUpdateContact (QString, QString, QString, QString);
		void slotDeleteMySelf (bool);
		void slotRemoveThisUser();
		void slotRenameContact();
		void slotDoRenameContact();
		void slotMoveThisUser();
    void slotChatThisUser();

	private:
		void initActions();

		QString mGroup;
		QString mStatus;
		QString mReason;
		JabberProtocol *mProtocol;
		KPopupMenu *popup;
		KAction* actionRemove;
		KAction* actionRemoveFromGroup;
		KAction* actionChat;
		KAction* actionInfo;
		KAction* actionHistory;
		KAction* actionRename;
		KListAction *actionContactMove;
		KListAction *actionContactCopy;
		dlgJabberRename *dlgRename;
    JabberMessageDialog *msgDialog;

	signals:
		void statusChanged();
    void msgRecieved(QString, QString, QString, QString, QFont, QColor);
};

#endif
