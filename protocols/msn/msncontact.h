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
#include "msnprotocol.h"

class QListView;
class QListViewItem;
class QPixmap;
class QTimer;

class KAction;
class KActionCollection;
class KListAction;

class KopeteHistoryDialog;

class MSNContact : public KopeteContact
{
	Q_OBJECT

public:
	MSNContact( QString &protocolId, const QString &msnId,
		const QString &displayName, const QString &group,
		KopeteMetaContact *parent );


	virtual QString id() const;
	virtual QString data() const;

	ContactStatus status() const;
	QString statusText() const;
	QString statusIcon() const;
	int importance() const;

	/**
	 * The MSN id of this user, e.g. kopeteuser@kde.org
	 */
	QString msnId() const;
	void setMsnId( const QString &id );

	/**
	 * Indicate whether this contact is blocked
	 */
	bool isBlocked() const;
	void setBlocked( bool b );

	/**
	 * Indicate whether this contact is deleted
	 * FIXME: What does this mean??? For now, just port from KMSNContact
	 */
	bool isDeleted() const;
	void setDeleted( bool d );

	/**
	 * Indicate whether this contact is allowed
	 * FIXME: What does this mean??? For now, just port from KMSNContact
	 */
	bool isAllowed() const;
	void setAllowed( bool d );

	/**
	 * The groups in which the user is located.
	 * Not the whole API supports multi-group membership yet, be careful
	 * relying on this for now!
	 */
	virtual QStringList groups();

	/**
	 * The contact's MSN specific status, this is NOT the ContactStatus!
	 * FIXME: Status handling needs serious redesign, probably!
	 */
	MSNProtocol::Status msnStatus() const;
	void setMsnStatus( MSNProtocol::Status status );

	/**
	 * Add/Remove user to/from a group
	 */
	virtual void addToGroup( const QString &group );
	virtual void removeFromGroup( const QString &group );
	virtual void moveToGroup( const QString &from, const QString &to );

	virtual bool isReachable() { return false; };

	virtual KActionCollection *customContextMenuActions();

public slots:


	virtual void slotUserInfo();
	virtual void slotDeleteContact();
	virtual void execute();
	virtual void slotViewHistory();

signals:
	void chatToUser( QString );

private slots:

	void slotBlockUser();

	void slotHistoryDialogClosing();
	void slotCloseHistoryDialog();


private:
	QString m_msnId;
	QStringList m_groups;
	bool hasLocalGroup;

	bool m_blocked;
	bool m_allowed;
	bool m_deleted;

	MSNProtocol::Status m_status;

	KopeteHistoryDialog *historyDialog;

	KActionCollection* m_actionCollection;
//	KAction* m_actionRemove;
//	KAction* m_actionRemoveFromGroup;
//	KAction* m_actionChat;
//	KAction* m_actionInfo;
	KAction* m_actionBlock;
//	KAction* m_actionHistory;
//	KListAction *m_actionMove;
//	KListAction *m_actionCopy;

	/**
	 * Most ugly hack ever. You're not seeing this. Nothing to see here.
	 * I plead the fifth if questions are asked.
	 * The contact list overhaul planned for Kopete 0.5 will hopefully fix
	 * this. 'nuff said.
	 */
	QString m_movingToGroup;

	/**
	 * Slightly less hacky, but still ugly: the group we're moving FROM. We
	 * need that to signal the completed move to libkopete when done.
	 */
	QString m_movingFromGroup;
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

