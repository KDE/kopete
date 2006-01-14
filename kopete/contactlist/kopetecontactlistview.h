/*
    kopetecontactlistview.h

    Kopete Contactlist GUI

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Nick Betcher <nbetcher@usinternet.com>
    Copyright (c) 2002      by Stefan Gehn <metz AT gehn.net>
    Copyright (c) 2002-2005 by Olivier Goffart <ogoffart @ kde.org>
    Copyright (c) 2004      by Richard Smith <kde@metafoo.co.uk>

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

#ifndef KOPETE_CONTACTLISTVIEW_H
#define KOPETE_CONTACTLISTVIEW_H

#include "kopetelistview.h"
#include "kopetemetacontact.h"

#include <qpixmap.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qrect.h>
#include <qtimer.h>
#include <qguardedptr.h>

class KopeteMetaContactLVI;
class KopeteGroupViewItem;
class KopeteStatusGroupViewItem;
class KRootPixmap;
class KActionCollection;
class KAction;
class KListAction;
class KActionMenu;

class KopeteContactListViewPrivate;

namespace Kopete
{
class Contact;
class MetaContact;
class Group;
class MessageEvent;
}

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteContactListView : public Kopete::UI::ListView::ListView
{
	Q_OBJECT

public:
	KopeteContactListView( QWidget *parent = 0, const char *name = 0 );
	~KopeteContactListView();

	/**
	 * Init MetaContact related actions
	 */
	void initActions(KActionCollection*);

	/**
	 * Add a given group name and return it
	 */
	void addGroup( const QString &groupName );

	/**
	 * Are we displaying as a tree view (true), or in a flat list (false)?
	 * @todo make this an enum
	 */
	bool showAsTree() { return mShowAsTree; }

public slots:
	/**
	 * Remove all KopeteMetaContactLVI of a metaContact
	 */
	void removeContact( Kopete::MetaContact *contact );

	/**
	 * Prompt the user for the group name (slot)
	 */
	void addGroup();

protected:
	virtual void contentsMousePressEvent( QMouseEvent *e );

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
	void updateActionsForSelection( QPtrList<Kopete::MetaContact> contacts, QPtrList<Kopete::Group> groups );

private slots:
	/**
	 * When an account is added, so we add it to the menu action
	 */
	void slotAddSubContactActionNewAccount(Kopete::Account*);
	/**
	 * When an account is destroyed, the child add subcontact action is deleted
	 * so we remove it from the menu action
	 */
	void slotAddSubContactActionAccountDeleted(const Kopete::Account *);

	void slotViewSelectionChanged();
	void slotListSelectionChanged();
	void slotContextMenu(KListView*,QListViewItem *item, const QPoint &point );
	void slotExpanded( QListViewItem *item );
	void slotCollapsed( QListViewItem *item );

	void slotSettingsChanged( void );
	void slotUpdateAllGroupIcons();
	void slotExecuted( QListViewItem *item, const QPoint &pos, int c );

	void slotAddedToGroup( Kopete::MetaContact *mc, Kopete::Group *to );
	void slotRemovedFromGroup( Kopete::MetaContact *mc, Kopete::Group *from );
	void slotMovedToGroup( Kopete::MetaContact *mc, Kopete::Group *from, Kopete::Group *to );

	/**
	 * A meta contact was added to the contact list - update the view
	 */
	void slotMetaContactAdded( Kopete::MetaContact *mc );
	void slotMetaContactDeleted( Kopete::MetaContact *mc );
	void slotMetaContactSelected( bool sel );

	void slotGroupAdded(Kopete::Group *);

	void slotContactStatusChanged( Kopete::MetaContact *mc );

	void slotDropped(QDropEvent *e, QListViewItem *parent, QListViewItem*);

	void slotShowAddContactDialog();
	void slotNewMessageEvent(Kopete::MessageEvent *);

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
	void slotRemove();
	void slotRename();
	void slotAddContact();
	void slotAddTemporaryContact();
	void slotProperties();
	void slotUndo();
	void slotRedo();

	void slotTimeout();

private:
	bool mShowAsTree;

	// TODO: do we really need to store these?
	QPtrList<KopeteMetaContactLVI> m_selectedContacts;
	QPtrList<KopeteGroupViewItem> m_selectedGroups;

	bool mSortByGroup;
	KRootPixmap *root;

	QRect m_onItem;

	QPoint m_startDragPos;

	/* ACTIONS */
	KAction *actionSendMessage;
	KAction *actionStartChat;
	KAction *actionSendFile;
	KAction *actionSendEmail;
	KListAction *actionMove;
	KListAction *actionCopy;
	KAction *actionRename;
	KAction *actionRemove;
	KAction *actionAddTemporaryContact;
	KAction *actionProperties;
	KAction *actionUndo;
	KAction *actionRedo;

	KopeteContactListViewPrivate *d;

	void moveDraggedContactToGroup( Kopete::MetaContact *contact, Kopete::Group *from, Kopete::Group *to );
	void addDraggedContactToGroup( Kopete::MetaContact *contact, Kopete::Group *group );
	void addDraggedContactToMetaContact( Kopete::Contact *contact, Kopete::MetaContact *parent );
	void addDraggedContactByInfo( const QString &protocolId, const QString &accountId,
		const QString &contactId, QListViewItem *after );

public:
	struct UndoItem;
	UndoItem *m_undo;
	UndoItem *m_redo;
	void insertUndoItem(UndoItem *u);
	QTimer undoTimer;

public:
	// This is public so the chatwinodw can handle sub actions
	// FIXME: do we not believe in accessor functions any more?
	KActionMenu *actionAddContact;
	QMap<const Kopete::Account *, KAction *> m_accountAddContactMap;
};

struct KopeteContactListView::UndoItem
{
	enum Type { MetaContactAdd, MetaContactRemove , MetaContactCopy , MetaContactRename, MetaContactChange, ContactAdd, GroupRename } type;
	QStringList args;
	QGuardedPtr<Kopete::MetaContact> metacontact;
	QGuardedPtr<Kopete::Group> group;
	UndoItem *next;
	bool isStep;
	Kopete::MetaContact::PropertySource nameSource;

	UndoItem() : isStep(true) {}
	UndoItem(Type t, Kopete::MetaContact *m=0L ,Kopete::Group *g=0L)
	{
		isStep=true;
		type=t;
		metacontact=m;
		group=g;
		next=0L;
	}
};


#endif
// vim: set noet ts=4 sts=4 sw=4:
