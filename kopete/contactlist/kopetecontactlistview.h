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

class KopeteContactListViewPrivate;

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

	/**
	 * Schedule a delayed sort operation. Sorts will be withheld for at most
	 * half a second, after which they will be performed. This way multiple
	 * sort calls can be safely bundled without writing complex code to avoid
	 * the sorts entirely.
	 */
	void delayedSort();

	/**
	 * Are we displaying as a tree view (true), or in a flat list (false)?
	 * @todo make this an enum
	 */
	bool showAsTree() { return mShowAsTree; }

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
	
	/**
	 * Start a drag operation
	 * @return a KMultipleDrag containing: 1) A QStoredDrag of type "application/x-qlistviewitem", 2) If the MC is associated with a KABC entry, i) a QTextDrag containing their email address, and ii) their vCard representation.
	 */
	virtual QDragObject *dragObject();

	/**
	 * Since KDE 3.1.1 ,  the original find Drop return 0L for afterme if the group is open.
	 * This woraround allow us to keep the highlight of the item, and give always a correct position
	 */
	virtual void findDrop(const QPoint &pos, QListViewItem *&parent, QListViewItem *&after);

	/**
	 * The selected items have changed; update our actions to show what's possible.
	 */
	void updateActionsForSelection( QPtrList<KopeteMetaContact> contacts, QPtrList<KopeteGroup> groups );


private slots:
	void slotViewSelectionChanged();
	void slotListSelectionChanged();
	void slotContextMenu(KListView*,QListViewItem *item, const QPoint &point );
	void slotExpanded( QListViewItem *item );
	void slotCollapsed( QListViewItem *item );
	void slotDoubleClicked( QListViewItem *item );

	void slotSettingsChanged( void );
	void slotUpdateAllGroupIcons();
	void slotExecuted( QListViewItem *item, const QPoint &pos, int c );

	void slotAddedToGroup( KopeteMetaContact *mc, KopeteGroup *to );
	void slotRemovedFromGroup( KopeteMetaContact *mc, KopeteGroup *from );
	void slotMovedToGroup( KopeteMetaContact *mc, KopeteGroup *from, KopeteGroup *to );

	/**
	 * A meta contact was added to the contact list - update the view
	 */
	void slotMetaContactAdded( KopeteMetaContact *mc );
	void slotMetaContactDeleted( KopeteMetaContact *mc );
	void slotMetaContactSelected( bool sel );

	void slotGroupAdded(KopeteGroup *);

	void slotContactStatusChanged( KopeteMetaContact *mc );

	void slotDropped(QDropEvent *e, QListViewItem *parent, QListViewItem*);

	void slotShowAddContactDialog();
	void slotNewMessageEvent(KopeteEvent *);

	/**
	 * Handle renamed items by renaming the meta contact
	 */
	void slotItemRenamed( QListViewItem *item );

	/** Actions related slots **/
	void slotSendMessage();
	void slotStartChat();
	void slotSendFile();
	void slotSendEmail();
	void slotMoveToGroup();
	void slotCopyToGroup();
	void slotRemoveFromGroup();
	void slotRemove();
	void slotRename();
	void slotAddContact();
	void slotAddTemporaryContact();
	void slotProperties();

	/**
	 * Sort the view when the timer expires.
	 * Too bad QListView::sort() is not a slot itself...
	 */
	void slotSort();

private:
	bool mShowAsTree;

	QPtrList<KopeteGroupViewItem> mGroups;

	QPtrList<KopeteMetaContactLVI> m_metaContacts;

	QPtrList<KopeteMetaContactLVI> m_selectedContacts;
	QPtrList<KopeteGroupViewItem> m_selectedGroups;

	KopeteStatusGroupViewItem *m_onlineItem;
	KopeteStatusGroupViewItem *m_offlineItem;
	bool mSortByGroup;
	KRootPixmap *root;

	QRect m_onItem;

	QPoint m_startDragPos;

	KopeteContactListViewToolTip *m_tooltip;

	/* ACTIONS */
	KAction *actionSendMessage;
	KAction *actionStartChat;
	KAction *actionSendFile;
	KAction *actionSendEmail;
	KListAction *actionMove;
	KListAction *actionCopy;
	KAction *actionRename;
	KAction *actionRemove;
	KAction *actionRemoveFromGroup;
	KAction *actionAddTemporaryContact;
	KAction *actionProperties;

	KopeteContactListViewPrivate *d;

	public:
		// This is public so the chatwinodw can handle sub actions
		KActionMenu *actionAddContact;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
