/*
    msncontact.h - MSN Contact

    Copyright (c) 2002 Duncan Mac-Vicar Prett <duncan@kde.org>
              (c) 2002 Ryan Cumming           <bodnar42@phalynx.dhs.org>
              (c) 2002 Martijn Klingens       <klingens@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNCONTACT_H
#define MSNCONTACT_H

#include "kopetecontact.h"
#include "kopetehistorydialog.h"

class QListView;
class QListViewItem;
class QPixmap;
class QTimer;

class KAction;
class KListAction;
class KPopupMenu;

class KopeteHistoryDialog;
class MSNProtocol;

class MSNContact : public KopeteContact
{
	Q_OBJECT

public:
	MSNContact( const QString &msnId, const QString &nickname,
				const QString &group, MSNProtocol *protocol );

	void initContact( const QString &msnId, const QString &nickname,
						const QString &group, const MSNProtocol *protocol );
	virtual void showContextMenu(QPoint);
	virtual void execute();

	ContactStatus status() const;
	QString statusText() const;
	QString statusIcon() const;
	int importance() const;

	/**
	 * The MSN id of this user, e.g. kopeteuser@kde.org
	 */
	QString msnId() const;

	/**
	 * The nickname of this user as known to MSN. By default this is also
	 * the name as displayed in the contact list, but the user may want to
	 * rename contacts locally, in which case there is an obvious difference.
	 */
	QString nickname() const;

	/**
	 * The groups in which the user is located.
	 * Not the whole API supports multi-group membership yet, be careful
	 * relying on this for now!
	 */
	QStringList groups() const;

public slots:
	void slotContactRemoved( QString, QString );
	void slotChatThisUser();

signals:
	void chatToUser( QString );

private slots:
	void slotRemoveThisUser();
	void slotCopyThisUser();
	void slotMoveThisUser();
	void slotRemoveFromGroup();

	void slotUpdateContact ( QString, uint );
	// We have to delete the contact if MSN disconenct
	// We will use the engine signal
	void slotDeleteMySelf ( bool );

	void slotHistoryDialogClosing();
	void slotCloseHistoryDialog();
	void slotViewHistory();

private:
	QString m_msnId;
	QString m_nickname;
	QStringList m_groups;
	bool hasLocalGroup;

	uint mStatus;
	const MSNProtocol *m_protocol;
	KopeteHistoryDialog *historyDialog;
	KPopupMenu *popup;

	KAction* m_actionRemove;
	KAction* m_actionRemoveFromGroup;
	KAction* m_actionChat;
	KAction* m_actionInfo;
	KAction* m_actionHistory;
	KListAction *m_actionMove;
	KListAction *m_actionCopy;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

