/*
    kopetecontactlistview.h

    Kopete Contactlist GUI

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Nick Betcher <nbetcher@usinternet.com>
    Copyright (c) 2002      by Stefan Gehn <metz AT gehn.net>
    Copyright (c) 2002-2003 by Olivier Goffart <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <qpixmap.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qrect.h>
#include <klistview.h>

class KopeteContact;
class KopeteMetaContact;
class KopeteMetaContactLVI;
class KopeteGroup;
class KopeteGroupViewItem;
class KopeteStatusGroupViewItem;
class KRootPixmap;
class KopeteEvent;
class KActionCollection;
class KAction;
class KListAction;
class KActionMenu;

class KopeteContactListViewToolTip;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteContactListView : public KListView
{
	Q_OBJECT

public:
	KopeteContactListView( QWidget *parent = 0, const char *name = 0 );
	~KopeteContactListView();

	/**
	 * Init MetaContact related actions
	 */
	void initActions(KActionCollection*);


	// FIXME: Make this private again when meta contact is more mature...
	KopeteGroupViewItem *getGroup( KopeteGroup* , bool add=true );


	/**
	 * Add a given group name and return it
	 */
	void addGroup( const QString groupName );

public slots:
	/**
	 * Remove all KopeteMetaContactLVI of a metaContact
	 */
	void removeContact( KopeteMetaContact *contact );

	/**
	 * Prompt the user for the group name (slot)
	 */
	void addGroup();

protected:
	virtual void contentsMousePressEvent( QMouseEvent *e );
	virtual void keyPressEvent( QKeyEvent *e );

	virtual bool acceptDrag(QDropEvent *e) const;
	virtual QDragObject *dragObject();

private slots:
	void slotSelectionChanged();
	void slotContextMenu(KListView*,QListViewItem *item, const QPoint &point );
	void slotExpanded( QListViewItem *item );
	void slotDoubleClicked( QListViewItem *item );
	void slotCollapsed( QListViewItem *item );

	void slotSettingsChanged( void );
	void slotExecuted( QListViewItem *item, const QPoint &pos, int c );

	void slotAddedToGroup( KopeteMetaContact *mc, KopeteGroup *to );
	void slotRemovedFromGroup( KopeteMetaContact *mc, KopeteGroup *from );
	void slotMovedToGroup( KopeteMetaContact *mc, KopeteGroup *from, KopeteGroup *to );

	/**
	 * A meta contact was added to the contact list - update the view
	 */
	void slotMetaContactAdded( KopeteMetaContact *mc );
	void slotMetaContactDeleted( KopeteMetaContact *mc);

	void slotGroupAdded(KopeteGroup *);

	void slotContactStatusChanged( KopeteMetaContact *mc );

	void slotDropped(QDropEvent *e, QListViewItem *parent, QListViewItem*);

	void slotShowAddContactDialog();
	void slotNewMessageEvent(KopeteEvent *);

	void removeGroup();


	/** Actions related slots **/
	void slotSendMessage();
	void slotStartChat();
	void slotSendFile();
	void slotMoveToGroup();
	void slotCopyToGroup();
	void slotRemoveFromGroup();
	void slotRemove();
	void slotRename();
	void slotAddContact();


private:

	bool mShowAsTree;

	QPixmap open;
	QPixmap closed;
	QPixmap classic;

	QPtrList<KopeteGroupViewItem> mGroups;

	QPtrList<KopeteMetaContactLVI> m_metaContacts;

	KopeteGroupViewItem *removeGroupItem;

	KopeteStatusGroupViewItem *m_onlineItem;
	KopeteStatusGroupViewItem *m_offlineItem;
	bool mSortByGroup;
	KRootPixmap *root;

	QRect m_onItem;

	QPoint m_startDragPos;

	KopeteContactListViewToolTip *m_tooltip;

	/* ACTIONS */
	KAction *actionSendFile;
	KListAction *actionMove;
	KListAction *actionCopy;
	KAction *actionRename;
	KAction *actionRemove;
	KAction *actionRemoveFromGroup;
	KAction *actionAddTemporaryContact;
public: KActionMenu *actionAddContact; //it is public so the chatwinodw can handle sub actions

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

