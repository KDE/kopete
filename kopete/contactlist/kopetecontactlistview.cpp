/*
    kopetecontactlistview.cpp

    Kopete Contactlist GUI

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Nick Betcher           <nbetcher@usinternet.com>
    Copyright (c) 2002      by Stefan Gehn            <metz AT gehn.net>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart @kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactlistview.h"
#include "kopeteuiglobal.h"

#include <qcursor.h>
#include <qdragobject.h>
#include <qheader.h>
#include <qstylesheet.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qguardedptr.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kurldrag.h>
#include <kmultipledrag.h>
#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>

#include <kdeversion.h>
#include <kinputdialog.h>

#include "addcontactwizard.h"
#include "addcontactpage.h"
#include "chatmessagepart.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontactlist.h"
#include "kopetemessageevent.h"
#include "kopetegroup.h"
#include "kopetegroupviewitem.h"
#include "kopetemetacontact.h"
#include "kopetemetacontactlvi.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetestatusgroupviewitem.h"
#include "kopetestdaction.h"
#include "kopetechatsessionmanager.h"
#include "kopetecontact.h"
#include "kopetechatsession.h" //needed to send the URL
#include "kopetemessage.h"       //needed to send the URL
#include "kopeteglobal.h"
#include "kopetelviprops.h"
#include "kopetegrouplistaction.h"

#include <memory>

class ContactListViewStrategy;

class KopeteContactListViewPrivate
{
public:
	std::auto_ptr<ContactListViewStrategy> viewStrategy;

	void updateViewStrategy( KListView *view );
};

class ContactListViewStrategy
{
public:
	ContactListViewStrategy( KListView *view )
	 : _listView( view )
	{
		view->clear();
	}
	virtual ~ContactListViewStrategy() {}
	KListView *listView() { return _listView; }
	void addCurrentItems()
	{
		// Add the already existing groups now
		QPtrList<Kopete::Group> grps = Kopete::ContactList::self()->groups();
		for ( QPtrListIterator<Kopete::Group> groupIt( grps ); groupIt.current(); ++groupIt )
			addGroup( groupIt.current() );

		// Add the already existing meta contacts now
		QPtrList<Kopete::MetaContact> metaContacts = Kopete::ContactList::self()->metaContacts();
		for ( QPtrListIterator<Kopete::MetaContact> it( metaContacts ); it.current(); ++it )
			addMetaContact( it.current() );
	}

	virtual void addMetaContact( Kopete::MetaContact *mc ) = 0;
	virtual void removeMetaContact( Kopete::MetaContact *mc ) = 0;

	virtual void addGroup( Kopete::Group * ) {}

	virtual void addMetaContactToGroup( Kopete::MetaContact *mc, Kopete::Group *gp ) = 0;
	virtual void removeMetaContactFromGroup( Kopete::MetaContact *mc, Kopete::Group *gp ) = 0;
	virtual void moveMetaContactBetweenGroups( Kopete::MetaContact *mc, Kopete::Group *from, Kopete::Group *to ) = 0;

	virtual void metaContactStatusChanged( Kopete::MetaContact *mc ) = 0;

protected:
	// work around QListView design stupidity.
	// GroupViewItem will be QListView-derived, or QListViewItem-derived.
	template<typename GroupViewItem>
	void addMetaContactToGroupInner( Kopete::MetaContact *mc, GroupViewItem *gpi )
	{
		// check if the contact isn't already in the group
		for( QListViewItem *item = gpi->firstChild(); item; item = item->nextSibling() )
			if ( KopeteMetaContactLVI *mci = dynamic_cast<KopeteMetaContactLVI*>(item) )
				if ( mci->metaContact() == mc )
					return;
		(void) new KopeteMetaContactLVI( mc, gpi );
	}

	template<typename GroupViewItem>
	void removeMetaContactFromGroupInner( Kopete::MetaContact *mc, GroupViewItem *gpi )
	{
	    KopeteMetaContactLVI* mci;
        QListViewItem* item = gpi->firstChild();
        while(item) {
            mci = dynamic_cast<KopeteMetaContactLVI*>(item);
	        item = item->nextSibling();

			if ( mci && mci->metaContact() == mc )
			    delete mci;
	    }
	}

private:
	KListView *_listView;
};

class ArrangeByGroupsViewStrategy : public ContactListViewStrategy
{
public:
	ArrangeByGroupsViewStrategy( KListView *view )
	 : ContactListViewStrategy( view )
	{
		addCurrentItems();
	}

	void addMetaContact( Kopete::MetaContact *mc )
	{
		// create group items
		Kopete::GroupList list = mc->groups();
		for ( Kopete::Group *gp = list.first(); gp; gp = list.next() )
			// will check to see if the contact is already in the group.
			// this is inefficient but makes this function idempotent.
			addMetaContactToGroup( mc, gp );
	}
	void removeMetaContact( Kopete::MetaContact *mc )
	{
		// usually, the list item will be deleted when the KMC is. however, we still
		// need to make sure that the item count of the groups is correct.
		// as a bonus, this allows us to remove a MC from the contact list without deleting it.
		Kopete::GroupList list = mc->groups();
		for ( Kopete::Group *gp = list.first(); gp; gp = list.next() )
			removeMetaContactFromGroup( mc, gp );
	}

	void addGroup( Kopete::Group *group )
	{
		(void) findOrCreateGroupItem( group );
	}

	void addMetaContactToGroup( Kopete::MetaContact *mc, Kopete::Group *gp )
	{
		if ( KopeteGroupViewItem *gpi = findOrCreateGroupItem( gp ) )
			addMetaContactToGroupInner( mc, gpi );
		else
			addMetaContactToGroupInner( mc, listView() );
	}
	void removeMetaContactFromGroup( Kopete::MetaContact *mc, Kopete::Group *gp )
	{
		if ( gp->type() == Kopete::Group::TopLevel )
			removeMetaContactFromGroupInner( mc, listView() );
		else if ( KopeteGroupViewItem *gpi = findGroupItem( gp ) )
		{
			removeMetaContactFromGroupInner( mc, gpi );

			// update the group's display of its number of children.
			// TODO: make the KopeteGroupViewItem not need this, by overriding insertItem and takeItem
			gpi->refreshDisplayName();

			// remove the temporary group if it's empty
			if ( gpi->childCount() == 0 )
				if ( gp->type() == Kopete::Group::Temporary )
					delete gpi;
		}
	}
	void moveMetaContactBetweenGroups( Kopete::MetaContact *mc, Kopete::Group *from, Kopete::Group *to )
	{
		// TODO: use takeItem and insertItem, and mci->movedGroup
		addMetaContactToGroup( mc, to );
		removeMetaContactFromGroup( mc, from );
	}
	void metaContactStatusChanged( Kopete::MetaContact * ) {}

private:
	KopeteGroupViewItem *findGroupItem( Kopete::Group *gp )
	{
		if ( gp->type() == Kopete::Group::TopLevel ) return 0;
		for( QListViewItem *item = listView()->firstChild(); item; item = item->nextSibling() )
			if ( KopeteGroupViewItem *gvi = dynamic_cast<KopeteGroupViewItem*>(item) )
				if ( gvi->group() == gp )
					return gvi;
		return 0;
	}
	KopeteGroupViewItem *findOrCreateGroupItem( Kopete::Group *gp )
	{
		if ( gp->type() == Kopete::Group::TopLevel ) return 0;
		if ( KopeteGroupViewItem *item = findGroupItem(gp) )
			return item;
		KopeteGroupViewItem *gpi = new KopeteGroupViewItem( gp, listView() );
		// TODO: store as plugin data the expandedness of a group
		// currently this requires a 'plugin' for the main UI.
		gpi->setOpen( gp->isExpanded() );
		return gpi;
	}
};

class ArrangeByPresenceViewStrategy : public ContactListViewStrategy
{
public:
	ArrangeByPresenceViewStrategy( KListView *view )
	 : ContactListViewStrategy( view )
	 , m_onlineItem( new KopeteStatusGroupViewItem( Kopete::OnlineStatus::Online, listView() ) )
	 , m_offlineItem( new KopeteStatusGroupViewItem( Kopete::OnlineStatus::Offline, listView() ) )
	{
		m_onlineItem->setOpen( true );
		m_offlineItem->setOpen( true );
		addCurrentItems();
	}

	void removeMetaContact( Kopete::MetaContact *mc )
	{
		// there's only three places we put metacontacts: online, offline and temporary.
		removeMetaContactFromGroupInner( mc, m_onlineItem );
		removeMetaContactFromGroupInner( mc, m_offlineItem );
		if ( m_temporaryItem )
			removeMetaContactFromGroupInner( mc, (KopeteGroupViewItem*)m_temporaryItem );
	}

	void addMetaContact( Kopete::MetaContact *mc )
	{
		updateMetaContact( mc );
	}
	void addMetaContactToGroup( Kopete::MetaContact *mc, Kopete::Group * )
	{
		updateMetaContact( mc );
	}
	void removeMetaContactFromGroup( Kopete::MetaContact *mc, Kopete::Group * )
	{
		updateMetaContact( mc );
	}
	void moveMetaContactBetweenGroups( Kopete::MetaContact *mc, Kopete::Group *, Kopete::Group * )
	{
		updateMetaContact( mc );
	}
	void metaContactStatusChanged( Kopete::MetaContact *mc )
	{
		updateMetaContact( mc );
	}
private:
	void updateMetaContact( Kopete::MetaContact *mc )
	{
		// split into a ...Inner function and this one to make the short-circuiting logic easier
		updateMetaContactInner( mc );

		// FIXME: these items should do this for themselves...
		m_onlineItem->setText(0,i18n("Online contacts (%1)").arg(m_onlineItem->childCount()));
		m_offlineItem->setText(0,i18n("Offline contacts (%1)").arg(m_offlineItem->childCount()));
	}
	void updateMetaContactInner( Kopete::MetaContact *mc )
	{
		// this function basically *is* the arrange-by-presence strategy.
		// given a metacontact, it removes it from any existing incorrect place and adds
		// it to the correct place. usually it does this with takeItem and insertItem.

		// if the metacontact is temporary, it should be only in the temporary group
		if ( mc->isTemporary() )
		{
			removeMetaContactFromGroupInner( mc, m_onlineItem );
			removeMetaContactFromGroupInner( mc, m_offlineItem );

			// create temporary item on demand
			if ( !m_temporaryItem )
			{
				m_temporaryItem = new KopeteGroupViewItem( Kopete::Group::temporary(), listView() );
				m_temporaryItem->setOpen( true );
			}

			addMetaContactToGroupInner( mc, (KopeteGroupViewItem*)m_temporaryItem );
			return;
		}

		// if it's not temporary, it should not be in the temporary group
		if ( m_temporaryItem )
		{
			removeMetaContactFromGroupInner( mc, (KopeteGroupViewItem*)m_temporaryItem );

			// remove temporary item if empty
			if ( m_temporaryItem && m_temporaryItem->childCount() == 0 )
			{
				delete (KopeteGroupViewItem*)m_temporaryItem;
			}
		}

		// check if the contact is already in the correct "group"
		QListViewItem *currentGroup = mc->isOnline() ? m_onlineItem : m_offlineItem;
		for( QListViewItem *lvi = currentGroup->firstChild(); lvi; lvi = lvi->nextSibling() )
			if ( KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi ) )
				if ( kc->metaContact() == mc )
					return;

		// item not found in the right group; look for it in the other group
		QListViewItem *oppositeGroup = mc->isOnline() ? m_offlineItem : m_onlineItem;
		for( QListViewItem *lvi = oppositeGroup->firstChild(); lvi; lvi = lvi->nextSibling() )
		{
			if ( KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi ) )
			{
				if ( kc->metaContact() == mc )
				{
					// found: move it over to the right group
					oppositeGroup->takeItem(kc);
					currentGroup->insertItem(kc);
					return;
				}
			}
		}

		// item not found in either online neither offline groups: add it
		(void) new KopeteMetaContactLVI( mc, currentGroup );
	}

	KopeteStatusGroupViewItem *m_onlineItem, *m_offlineItem;
	QGuardedPtr<KopeteGroupViewItem> m_temporaryItem;
};

void KopeteContactListViewPrivate::updateViewStrategy( KListView *view )
{
	// this is a bit nasty, but this function needs changing if we add
	// more view strategies anyway, so it should be fine.
	bool bSortByGroup = (bool)dynamic_cast<ArrangeByGroupsViewStrategy*>(viewStrategy.get());
	if ( !viewStrategy.get() || KopetePrefs::prefs()->sortByGroup() != bSortByGroup )
	{
		// delete old strategy first...
		viewStrategy.reset( 0 );
		// then create and store a new one
		if ( KopetePrefs::prefs()->sortByGroup() )
			viewStrategy.reset( new ArrangeByGroupsViewStrategy(view) );
		else
			viewStrategy.reset( new ArrangeByPresenceViewStrategy(view) );
	}
}

// returns the next item in a depth-first descent of the list view.
// much like QLVI::itemBelow but does not depend on visibility of items, etc.
static QListViewItem *nextItem( QListViewItem *item )
{
	if ( QListViewItem *it = item->firstChild() )
		return it;
	while ( item && !item->nextSibling() )
		item = item->parent();
	if ( !item )
		return 0;
	return item->nextSibling();
}



KopeteContactListView::KopeteContactListView( QWidget *parent, const char *name )
 : Kopete::UI::ListView::ListView( parent, name )
{
	d = new KopeteContactListViewPrivate;
	m_undo=0L;
	m_redo=0L;

	mShowAsTree = KopetePrefs::prefs()->treeView();
	if ( mShowAsTree )
	{
		setRootIsDecorated( true );
	}
	else
	{
		setRootIsDecorated( false );
		setTreeStepSize( 0 );
	}

	d->updateViewStrategy( this );

	setFullWidth( true );

	connect( this, SIGNAL( contextMenu( KListView *, QListViewItem *, const QPoint & ) ),
	         SLOT( slotContextMenu( KListView *, QListViewItem *, const QPoint & ) ) );
	connect( this, SIGNAL( expanded( QListViewItem * ) ),
	         SLOT( slotExpanded( QListViewItem * ) ) );
	connect( this, SIGNAL( collapsed( QListViewItem * ) ),
	         SLOT( slotCollapsed( QListViewItem * ) ) );
	connect( this, SIGNAL( executed( QListViewItem *, const QPoint &, int ) ),
	         SLOT( slotExecuted( QListViewItem *, const QPoint &, int ) ) );
	connect( this, SIGNAL( selectionChanged() ), SLOT( slotViewSelectionChanged() ) );
	connect( this, SIGNAL( itemRenamed( QListViewItem * ) ),
	         SLOT( slotItemRenamed( QListViewItem * ) ) );

	connect( KopetePrefs::prefs(), SIGNAL( saved() ), SLOT( slotSettingsChanged() ) );

	connect( Kopete::ContactList::self(), SIGNAL( selectionChanged() ),
	         SLOT( slotListSelectionChanged() ) );
	connect( Kopete::ContactList::self(),
	         SIGNAL( metaContactAdded( Kopete::MetaContact * ) ),
	         SLOT( slotMetaContactAdded( Kopete::MetaContact * ) ) );
	connect( Kopete::ContactList::self(),
	         SIGNAL( metaContactRemoved( Kopete::MetaContact * ) ),
	         SLOT( slotMetaContactDeleted( Kopete::MetaContact * ) ) );
	connect( Kopete::ContactList::self(), SIGNAL( groupAdded( Kopete::Group * ) ),
	         SLOT( slotGroupAdded( Kopete::Group * ) ) );

	connect( Kopete::ChatSessionManager::self(), SIGNAL( newEvent( Kopete::MessageEvent * ) ),
	         this, SLOT( slotNewMessageEvent( Kopete::MessageEvent * ) ) );

	connect( this, SIGNAL( dropped( QDropEvent *, QListViewItem *, QListViewItem * ) ),
	         this, SLOT( slotDropped( QDropEvent *, QListViewItem *, QListViewItem * ) ) );

	connect( &undoTimer, SIGNAL(timeout()) , this, SLOT (slotTimeout() ) );

	addColumn( i18n( "Contacts" ), 0 );  //add an unique colums to add every contact
	header()->hide(); // and hide the ugly header which show the single word  "Contacts"

	setAutoOpen( true );
	setDragEnabled( true );
	setAcceptDrops( true );
	setItemsMovable( false );
	setDropVisualizer( false );
	setDropHighlighter( true );
	setSelectionMode( QListView::Extended );

	// Load in the user's initial settings
	slotSettingsChanged();
}

void KopeteContactListView::initActions( KActionCollection *ac )
{
	actionUndo = KStdAction::undo( this , SLOT( slotUndo() ) , ac );
	actionRedo = KStdAction::redo( this , SLOT( slotRedo() ) , ac );
	actionUndo->setEnabled(false);
	actionRedo->setEnabled(false);


	new KAction( i18n( "Create New Group..." ), 0, 0, this, SLOT( addGroup() ),
		ac, "AddGroup" );

	actionSendMessage = KopeteStdAction::sendMessage(
		this, SLOT( slotSendMessage() ), ac, "contactSendMessage" );
	actionStartChat = KopeteStdAction::chat( this, SLOT( slotStartChat() ),
		ac, "contactStartChat" );

	actionMove = new KopeteGroupListAction( i18n( "&Move To" ), QString::fromLatin1( "editcut" ),
														  0, this, SLOT( slotMoveToGroup() ), ac, "contactMove" );
	actionCopy = new KopeteGroupListAction( i18n( "&Copy To" ), QString::fromLatin1( "editcopy" ), 0,
														 this, SLOT( slotCopyToGroup() ), ac, "contactCopy" );

	actionRemove = KopeteStdAction::deleteContact( this, SLOT( slotRemove() ),
		ac, "contactRemove" );
	actionSendEmail = new KAction( i18n( "Send Email..." ), QString::fromLatin1( "mail_generic" ),
		0, this, SLOT(  slotSendEmail() ), ac, "contactSendEmail" );
	/* this actionRename is buggy, and useless with properties, removed in kopeteui.rc*/
	actionRename = new KAction( i18n( "Rename" ), "filesaveas", 0,
		this, SLOT( slotRename() ), ac, "contactRename" );
	actionSendFile = KopeteStdAction::sendFile( this, SLOT( slotSendFile() ),
		ac, "contactSendFile" );

	actionAddContact = new KActionMenu( i18n( "&Add Contact" ),
		QString::fromLatin1( "add_user" ), ac , "contactAddContact" );
	actionAddContact->popupMenu()->insertTitle( i18n("Select Account") );

	actionAddTemporaryContact = new KAction( i18n( "Add to Your Contact List" ), "add_user", 0,
		this, SLOT( slotAddTemporaryContact() ), ac, "contactAddTemporaryContact" );

	connect( Kopete::ContactList::self(), SIGNAL( metaContactSelected( bool ) ), this, SLOT( slotMetaContactSelected( bool ) ) );

	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered( Kopete::Account* )), SLOT(slotAddSubContactActionNewAccount(Kopete::Account*)));
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered( const Kopete::Account* )), SLOT(slotAddSubContactActionAccountDeleted(const Kopete::Account *)));

	actionProperties = new KAction( i18n( "&Properties" ), "edit_user", Qt::Key_Alt + Qt::Key_Return,
		this, SLOT( slotProperties() ), ac, "contactProperties" );

	// Update enabled/disabled actions
	slotViewSelectionChanged();
}

KopeteContactListView::~KopeteContactListView()
{
	delete d;
}

void KopeteContactListView::slotAddSubContactActionNewAccount(Kopete::Account* account)
{
	KAction *action = new KAction( account->accountLabel(), account->accountIcon(), 0 , this, SLOT(slotAddContact()), account);
	m_accountAddContactMap.insert( account, action);
	actionAddContact->insert( action );
}

void KopeteContactListView::slotAddSubContactActionAccountDeleted(const Kopete::Account *account)
{
	kdDebug(14000) << k_funcinfo << endl;
	if ( m_accountAddContactMap.contains( account ) )
	{
		KAction *action = m_accountAddContactMap[account];
		m_accountAddContactMap.remove( account );
		actionAddContact->remove( action );
	}
}

void KopeteContactListView::slotMetaContactAdded( Kopete::MetaContact *mc )
{
	d->viewStrategy->addMetaContact( mc );

	connect( mc, SIGNAL( addedToGroup( Kopete::MetaContact *, Kopete::Group * ) ),
		SLOT( slotAddedToGroup( Kopete::MetaContact *, Kopete::Group * ) ) );
	connect( mc, SIGNAL( removedFromGroup( Kopete::MetaContact *, Kopete::Group * ) ),
		SLOT( slotRemovedFromGroup( Kopete::MetaContact *, Kopete::Group * ) ) );
	connect( mc, SIGNAL( movedToGroup( Kopete::MetaContact *, Kopete::Group *, Kopete::Group * ) ),
		SLOT( slotMovedToGroup( Kopete::MetaContact *, Kopete::Group *, Kopete::Group * ) ) );
	connect( mc, SIGNAL( onlineStatusChanged( Kopete::MetaContact *, Kopete::OnlineStatus::StatusType ) ),
		SLOT( slotContactStatusChanged( Kopete::MetaContact * ) ) );
}

void KopeteContactListView::slotMetaContactDeleted( Kopete::MetaContact *mc )
{
	d->viewStrategy->removeMetaContact( mc );
}

void KopeteContactListView::slotMetaContactSelected( bool sel )
{
	bool set = sel;

	if( sel )
	{
		Kopete::MetaContact *kmc = Kopete::ContactList::self()->selectedMetaContacts().first();
		set = sel && kmc->isReachable();
		actionAddTemporaryContact->setEnabled( sel && kmc->isTemporary() );
	}
	else
	{
		actionAddTemporaryContact->setEnabled(false);
	}

	actionSendMessage->setEnabled( set );
	actionStartChat->setEnabled( set );
	actionMove->setEnabled( sel ); // TODO: make available for several contacts
	actionCopy->setEnabled( sel ); // TODO: make available for several contacts
}

void KopeteContactListView::slotAddedToGroup( Kopete::MetaContact *mc, Kopete::Group *to )
{
	d->viewStrategy->addMetaContactToGroup( mc, to );
}

void KopeteContactListView::slotRemovedFromGroup( Kopete::MetaContact *mc, Kopete::Group *from )
{
	d->viewStrategy->removeMetaContactFromGroup( mc, from );
}

void KopeteContactListView::slotMovedToGroup( Kopete::MetaContact *mc,
	Kopete::Group *from, Kopete::Group *to )
{
	d->viewStrategy->moveMetaContactBetweenGroups( mc, from, to );
}

void KopeteContactListView::removeContact( Kopete::MetaContact *c )
{
	d->viewStrategy->removeMetaContact( c );
}

void KopeteContactListView::addGroup()
{
	QString groupName =
		KInputDialog::getText( i18n( "New Group" ),
			i18n( "Please enter the name for the new group:" ) );

	if ( !groupName.isEmpty() )
		addGroup( groupName );
}

void KopeteContactListView::addGroup( const QString &groupName )
{
	d->viewStrategy->addGroup( Kopete::ContactList::self()->findGroup(groupName) );
}

void KopeteContactListView::slotGroupAdded( Kopete::Group *group )
{
	d->viewStrategy->addGroup( group );
}

void KopeteContactListView::slotExpanded( QListViewItem *item )
{
	KopeteGroupViewItem *groupLVI = dynamic_cast<KopeteGroupViewItem *>( item );
	if ( groupLVI )
	{
		groupLVI->group()->setExpanded( true );
		groupLVI->updateIcon();
	}
	
	//workaround a bug in qt which make the items of a closed item not sorted. (qt 3.3.4 here)
	delayedSort();
}

void KopeteContactListView::slotCollapsed( QListViewItem *item )
{
	KopeteGroupViewItem *groupLVI = dynamic_cast<KopeteGroupViewItem*>( item );
	if ( groupLVI )
	{
		groupLVI->group()->setExpanded( false );
		groupLVI->updateIcon();
	}
}

void KopeteContactListView::slotContextMenu( KListView * /*listview*/,
	QListViewItem *item, const QPoint &point )
{
	// FIXME: this code should be moved to the various list view item classes.
	KopeteMetaContactLVI *metaLVI = dynamic_cast<KopeteMetaContactLVI *>( item );
	KopeteGroupViewItem  *groupvi = dynamic_cast<KopeteGroupViewItem *>( item );

	if ( item && !item->isSelected() )
	{
		clearSelection();
		item->setSelected( true );
	}

	if ( !item )
	{
		clearSelection();
		//Clear selection doesn't update lists of selected contact if the item is onlt heilighted (see bug 106090)
		Kopete::ContactList::self()->setSelectedItems( QPtrList<Kopete::MetaContact>() , QPtrList<Kopete::Group>() );
	}

	int nb = Kopete::ContactList::self()->selectedMetaContacts().count() +
		Kopete::ContactList::self()->selectedGroups().count();

	KMainWindow *window = dynamic_cast<KMainWindow *>(topLevelWidget());
	if ( !window )
	{
		kdError( 14000 ) << k_funcinfo << "Main window not found, unable to display context-menu; "
			<< "Kopete::UI::Global::mainWidget() = " << Kopete::UI::Global::mainWidget() << endl;
		return;
	}

	if ( metaLVI && nb == 1 )
	{
		int px = mapFromGlobal( point ).x() -  ( header()->sectionPos( header()->mapToIndex( 0 ) ) +
			treeStepSize() * ( item->depth() + ( rootIsDecorated() ? 1 : 0 ) ) + itemMargin() );
		int py = mapFromGlobal( point ).y() - itemRect( item ).y() - (header()->isVisible() ? header()->height() : 0) ;

		//kdDebug( 14000 ) << k_funcinfo << "x: " << px << ", y: " << py << endl;
		Kopete::Contact *c = metaLVI->contactForPoint( QPoint( px, py ) ) ;
		if ( c )
		{
			KPopupMenu *p = c->popupMenu();
			connect( p, SIGNAL( aboutToHide() ), p, SLOT( deleteLater() ) );
			p->popup( point );
		}
		else
		{
			KPopupMenu *popup = dynamic_cast<KPopupMenu *>(
				window->factory()->container( "contact_popup", window ) );
			if ( popup )
			{
				QString title = i18n( "Translators: format: '<nickname> (<online status>)'", "%1 (%2)" ).
					arg( metaLVI->metaContact()->displayName(), metaLVI->metaContact()->statusString() );

				if ( title.length() > 43 )
					title = title.left( 40 ) + QString::fromLatin1( "..." );

				if ( popup->title( 0 ).isNull() )
					popup->insertTitle ( title, 0, 0 );
				else
					popup->changeTitle ( 0, title );

				// Submenus for separate contact actions
				bool sep = false;  //FIXME: find if there is already a separator in the end - Olivier
				QPtrList<Kopete::Contact> it = metaLVI->metaContact()->contacts();
				for( Kopete::Contact *c = it.first(); c; c = it.next() )
				{
					if( sep )
					{
						popup->insertSeparator();
						sep = false;
					}

					KPopupMenu *contactMenu = it.current()->popupMenu();
					connect( popup, SIGNAL( aboutToHide() ), contactMenu, SLOT( deleteLater() ) );
					QString nick=c->property(Kopete::Global::Properties::self()->nickName()).value().toString();
					QString text= nick.isEmpty() ?  c->contactId() : i18n( "Translators: format: '<displayName> (<id>)'", "%2 <%1>" ). arg( c->contactId(), nick );
					text=text.replace("&","&&"); // cf BUG 115449

					if ( text.length() > 41 )
						text = text.left( 38 ) + QString::fromLatin1( "..." );

					popup->insertItem( c->onlineStatus().iconFor( c, 16 ), text , contactMenu );
				}

				popup->popup( point );
			}
		}
	}
	else if ( groupvi && nb == 1 )
	{
		KPopupMenu *popup = dynamic_cast<KPopupMenu *>(
			window->factory()->container( "group_popup", window ) );
		if ( popup )
		{
			QString title = groupvi->group()->displayName();
			if ( title.length() > 32 )
				title = title.left( 30 ) + QString::fromLatin1( "..." );

			if( popup->title( 0 ).isNull() )
				popup->insertTitle( title, 0, 0 );
			else
				popup->changeTitle( 0, title );

			popup->popup( point );
		}
	}
	else if ( nb >= 1 )
	{
		KPopupMenu *popup = dynamic_cast<KPopupMenu *>(
			window->factory()->container( "contactlistitems_popup", window ) );
		if ( popup )
			popup->popup( point );
	}
	else
	{
		KPopupMenu *popup = dynamic_cast<KPopupMenu *>(
			window->factory()->container( "contactlist_popup", window ) );
		if ( popup )
		{
			if ( popup->title( 0 ).isNull() )
				popup->insertTitle( i18n( "Kopete" ), 0, 0 );

			popup->popup( point );
		}
	}
}

void KopeteContactListView::slotShowAddContactDialog()
{
	( new AddContactWizard( Kopete::UI::Global::mainWidget() ) )->show();
}

void KopeteContactListView::slotSettingsChanged( void )
{
	mShowAsTree = KopetePrefs::prefs()->treeView();
	if ( mShowAsTree )
	{
		setRootIsDecorated( true );
		setTreeStepSize( 20 );
	}
	else
	{
		setRootIsDecorated( false );
		setTreeStepSize( 0 );
	}

	// maybe setEffects should read these from KopetePrefs itself?
	Kopete::UI::ListView::Item::setEffects( KopetePrefs::prefs()->contactListAnimation(),
	                                        KopetePrefs::prefs()->contactListFading(),
	                                        KopetePrefs::prefs()->contactListFolding() );

	d->updateViewStrategy( this );

	slotUpdateAllGroupIcons();
	update();
}

void KopeteContactListView::slotUpdateAllGroupIcons()
{
	// FIXME: groups can (should?) do this for themselves
	// HACK: assume all groups are top-level. works for now, until the fixme above is dealt with
	for ( QListViewItem *it = firstChild(); it; it = it->nextSibling() )
		if ( KopeteGroupViewItem *gpi = dynamic_cast<KopeteGroupViewItem*>( it ) )
			gpi->updateIcon();
}

void KopeteContactListView::slotExecuted( QListViewItem *item, const QPoint &p, int /* col */ )
{
	item->setSelected( false );
	KopeteMetaContactLVI *metaContactLVI = dynamic_cast<KopeteMetaContactLVI *>( item );

	QPoint pos = viewport()->mapFromGlobal( p );
	Kopete::Contact *c = 0L;
	if ( metaContactLVI )
	{
		// Try if we are clicking a protocol icon. If so, open a direct
		// connection for that protocol
		QRect r = itemRect( item );
		QPoint relativePos( pos.x() - r.left() - ( treeStepSize() *
			( item->depth() + ( rootIsDecorated() ? 1 : 0 ) ) +
			itemMargin() ), pos.y() - r.top() );
		c = metaContactLVI->contactForPoint( relativePos );
		if( c )
			c->execute();
		else
			metaContactLVI->execute();
	}
}

void KopeteContactListView::slotContactStatusChanged( Kopete::MetaContact *mc )
{
	d->viewStrategy->metaContactStatusChanged( mc );
}

void KopeteContactListView::slotDropped(QDropEvent *e, QListViewItem *, QListViewItem *after)
{
	if(!acceptDrag(e))
		return;

	KopeteMetaContactLVI *dest_metaLVI=dynamic_cast<KopeteMetaContactLVI*>(after);
	KopeteGroupViewItem *dest_groupLVI=dynamic_cast<KopeteGroupViewItem*>(after);

	if( const_cast<const QWidget *>( e->source() ) == this )
	{
		QPtrListIterator<KopeteMetaContactLVI> it( m_selectedContacts );

		while ( it.current() )
		{
			Kopete::Contact *source_contact=0L;
			KopeteMetaContactLVI *source_metaLVI = it.current();
			++it;

			if(source_metaLVI)
				source_contact = source_metaLVI->contactForPoint( m_startDragPos );

			if(source_metaLVI  && dest_groupLVI)
			{
				if(source_metaLVI->group() == dest_groupLVI->group())
					return;

				if(source_metaLVI->metaContact()->isTemporary())
				{
					addDraggedContactToGroup(source_metaLVI->metaContact(),dest_groupLVI->group());
				}
				else
				{
					moveDraggedContactToGroup( source_metaLVI->metaContact(),
						source_metaLVI->group(), dest_groupLVI->group() );
				}
			}
			else if(source_metaLVI  && !dest_metaLVI && !dest_groupLVI)
			{
				if ( source_metaLVI->group()->type() == Kopete::Group::TopLevel )
					return;

				if(source_metaLVI->metaContact()->isTemporary())
				{
					addDraggedContactToGroup(source_metaLVI->metaContact() , Kopete::Group::topLevel() );
				}
				else
				{
					moveDraggedContactToGroup( source_metaLVI->metaContact(),
						source_metaLVI->group(), Kopete::Group::topLevel() );
				}
			}
			else if(source_contact && dest_metaLVI) //we are moving a contact to another metacontact
			{
				if(source_metaLVI->metaContact()->isTemporary())
				{
					addDraggedContactToMetaContact( source_contact,dest_metaLVI->metaContact() );
				}
				else
				{
					UndoItem *u=new UndoItem;
					u->type=UndoItem::MetaContactChange;
					u->metacontact=source_metaLVI->metaContact();
					u->group=source_metaLVI->group();
					u->args << source_contact->protocol()->pluginId() << source_contact->account()->accountId() << source_contact->contactId();
					u->args << source_metaLVI->metaContact()->displayName();
					insertUndoItem(u);

					source_contact->setMetaContact(dest_metaLVI->metaContact());
				}
			}
		}
	}
	else if( e->provides("kopete/x-contact") )
	{
		QString contactInfo = QString::fromUtf8( e->encodedData("kopete/x-contact") );
		QString protocolId = contactInfo.section( QChar( 0xE000 ), 0, 0 );
		QString accountId = contactInfo.section( QChar( 0xE000 ), 1, 1 );
		QString contactId = contactInfo.section( QChar( 0xE000 ), 2 );

		addDraggedContactByInfo( protocolId, accountId, contactId, after );

	}
	else if( e->provides("text/uri-list") )
	{
		if ( !QUriDrag::canDecode( e ) )
		{
			e->ignore();
			return;
		}

		KURL::List urlList;
		KURLDrag::decode( e, urlList );

		for ( KURL::List::Iterator it = urlList.begin(); it != urlList.end(); ++it )
		{
			KURL url = (*it);
			if( url.protocol() == QString::fromLatin1("kopetemessage") )
			{
				//Add a contact
				addDraggedContactByInfo( url.queryItem("protocolId"),
					url.queryItem("accountId"), url.host(), after );
			}
			else if( dest_metaLVI )
			{
				QPoint p = contentsToViewport(e->pos());
				int px = p.x() - ( header()->sectionPos( header()->mapToIndex( 0 ) ) +
					treeStepSize() * ( dest_metaLVI->depth() + ( rootIsDecorated() ? 1 : 0 ) ) + itemMargin() );
				int py = p.y() - itemRect( dest_metaLVI ).y();

				Kopete::Contact *c = dest_metaLVI->contactForPoint( QPoint( px, py ) );

				if( url.isLocalFile() )
				{
					//send a file
					if(c)
						c->sendFile( url );
					else
						dest_metaLVI->metaContact()->sendFile( url );
				}
				else
				{
					//this is a URL, send the URL in a message
					if(!c)
					{
						// We need to know which contact was chosen as the preferred
						// in order to message it
						c = dest_metaLVI->metaContact()->execute();
					}

					if (!c)
						return;

					Kopete::Message msg(c->account()->myself(), c, url.url(),
						Kopete::Message::Outbound);
					c->manager(Kopete::Contact::CanCreate)->sendMessage(msg);
				}
			}
		}
		e->acceptAction();
	}
}

void KopeteContactListView::moveDraggedContactToGroup( Kopete::MetaContact *contact, Kopete::Group *from, Kopete::Group *to )
{
	contact->moveToGroup( from, to );

	insertUndoItem( new UndoItem( UndoItem::MetaContactCopy , contact, to ) );
	UndoItem *u=new UndoItem( UndoItem::MetaContactRemove, contact, to );
	u->isStep=false;
	insertUndoItem(u);
}

void KopeteContactListView::addDraggedContactToGroup( Kopete::MetaContact *contact, Kopete::Group *group )
{
	int r=KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
					i18n( "<qt>Would you like to add <b>%1</b> to your contact list as a member of <b>%2</b>?</qt>" )
					.arg( contact->displayName(), group->displayName() ),
					i18n( "Kopete" ), i18n("Add"), i18n("Do Not Add"),
					"addTemporaryWhenMoving" );

	if( r == KMessageBox::Yes )
	{
		contact->setTemporary( false, group );
		Kopete::ContactList::self()->addMetaContact( contact );
		insertUndoItem( new UndoItem( UndoItem::MetaContactAdd, contact, group ) );
	}
}

void KopeteContactListView::addDraggedContactToMetaContact( Kopete::Contact *contact, Kopete::MetaContact *parent )
{
	int r = KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
					i18n( "<qt>Would you like to add <b>%1</b> to your contact list as a child contact of <b>%2</b>?</qt>" )
					.arg( contact->contactId(), parent->displayName() ),
					i18n( "Kopete" ), i18n("Add"), i18n("Do Not Add"),
					"addTemporaryWhenMoving" );

	if( r == KMessageBox::Yes )
	{
		contact->setMetaContact(parent);

		UndoItem *u=new UndoItem;
		u->type=UndoItem::ContactAdd;
		u->args << contact->protocol()->pluginId() << contact->account()->accountId() << contact->contactId();
		insertUndoItem(u);
	}
}

void KopeteContactListView::addDraggedContactByInfo( const QString &protocolId, const QString &accountId,
			      const QString &contactId, QListViewItem *after )
{
	kdDebug(14000) << k_funcinfo << "protocolId=" << protocolId <<
		", accountId=" << accountId << ", contactId=" << contactId << endl;

	Kopete::Account *account = Kopete::AccountManager::self()->findAccount( protocolId,accountId );
	if( account )
	{
		QDict<Kopete::Contact> contacts = account->contacts();
		Kopete::Contact *source_contact = contacts[ contactId ];

		if( source_contact )
		{
			if( source_contact->metaContact()->isTemporary() )
			{
				KopeteMetaContactLVI *dest_metaLVI=dynamic_cast<KopeteMetaContactLVI*>(after);
				KopeteGroupViewItem *dest_groupLVI=dynamic_cast<KopeteGroupViewItem*>(after);

				if( dest_metaLVI )
				{
					addDraggedContactToMetaContact( source_contact, dest_metaLVI->metaContact() );
				}
				else if( dest_groupLVI )
				{
					addDraggedContactToGroup( source_contact->metaContact(),dest_groupLVI->group() );
				}
				else
				{
					addDraggedContactToGroup( source_contact->metaContact(), Kopete::Group::topLevel() );
				}
			}
			else
			{
				KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
					i18n("<qt>This contact is already on your contact list. It is a child contact of <b>%1</b></qt>")
					.arg( source_contact->metaContact()->displayName() )
				);
			}
		}
	}
}

bool KopeteContactListView::acceptDrag(QDropEvent *e) const
{
	QListViewItem *source=currentItem();
	QListViewItem *parent;
	QListViewItem *afterme;
	// Due to a little design problem in KListView::findDrop() we can't
	// call it directly from a const method until KDE 4.0, but as the
	// method is in fact const we can of course get away with a
	// const_cast...
	const_cast<KopeteContactListView *>( this )->findDrop( e->pos(), parent, afterme );

	KopeteMetaContactLVI *dest_metaLVI=dynamic_cast<KopeteMetaContactLVI*>(afterme);

	if( const_cast<const QWidget *>( e->source() ) == this )
	{
		KopeteMetaContactLVI *source_metaLVI=dynamic_cast<KopeteMetaContactLVI*>(source);
		KopeteGroupViewItem *dest_groupLVI=dynamic_cast<KopeteGroupViewItem*>(afterme);
		Kopete::Contact *source_contact=0L;

		if(source_metaLVI)
			source_contact = source_metaLVI->contactForPoint( m_startDragPos );

		if( source_metaLVI && dest_groupLVI && !source_contact)
		{	//we are moving a metacontact to another group
			if(source_metaLVI->group() == dest_groupLVI->group())
				return false;
			if ( dest_groupLVI->group()->type() == Kopete::Group::Temporary )
				return false;
	//		if(source_metaLVI->metaContact()->isTemporary())
	//			return false;
			return true;
		}
		else if(source_metaLVI  && !dest_metaLVI && !dest_groupLVI && !source_contact)
		{	//we are moving a metacontact to toplevel
			if ( source_metaLVI->group()->type() == Kopete::Group::TopLevel )
				return false;
	//		if(source_metaLVI->metaContact()->isTemporary())
	//			return false;

			return true;
		}
		else if(source_contact && dest_metaLVI) //we are moving a contact to another metacontact
		{
			if(source_contact->metaContact() == dest_metaLVI->metaContact() )
				return false;
			if(dest_metaLVI->metaContact()->isTemporary())
				return false;
			return true;
		}
/*		else if(source_groupLVI && dest_groupLVI) //we are moving a group to another group
		{
			if(dest_groupLVI->group() == Kopete::Group::temporary)
				return false;
			if(source_groupLVI->group() == Kopete::Group::temporary)
				return false;
			if(source_groupLVI->group()->parentGroup()  == dest_groupLVI->group() )
				return false;
			Kopete::Group *g=dest_groupLVI->group()->parentGroup();
			while(g && g != Kopete::Group::toplevel)
			{
				if(g==source_groupLVI->group())
					return false;
				g=g->parentGroup();
			}
			return true;
		}
		else if(source_groupLVI && !dest_groupLVI && dest_metaLVI) //we are moving a group to toplevel
		{
			if(source_groupLVI->group() == Kopete::Group::temporary)
				return false;
			if(source_groupLVI->group()->parentGroup() == Kopete::Group::toplevel)
				return false;
			return true;
		}*/
	}
	else
	{
		if( e->provides( "text/uri-list" ) )
		{
			//we are sending a file (or dragging from the chat view)
			if( dest_metaLVI )
			{
				QPoint p=contentsToViewport(e->pos());
				int px = p.x() - ( header()->sectionPos( header()->mapToIndex( 0 ) ) +
					treeStepSize() * ( dest_metaLVI->depth() + ( rootIsDecorated() ? 1 : 0 ) ) + itemMargin() );
				int py = p.y() - itemRect( dest_metaLVI ).y();
				Kopete::Contact *c = dest_metaLVI->contactForPoint( QPoint( px, py ) ) ;

				if( c ? !c->isReachable() : !dest_metaLVI->metaContact()->isReachable() )
					return false; //If the pointed contact is not reachable, abort

				if( c ? c->canAcceptFiles() : dest_metaLVI->metaContact()->canAcceptFiles() )
				{
					// If the pointed contact, or the metacontact if no contact are
					// pointed can accept file, return true in everycase
					return true;
				}
			}

			if ( !QUriDrag::canDecode(e) )
				return false;

			KURL::List urlList;
			KURLDrag::decode( e, urlList );

			for ( KURL::List::Iterator it = urlList.begin(); it != urlList.end(); ++it )
			{
				if( (*it).protocol() != QString::fromLatin1("kopetemessage") && (*it).isLocalFile() )
					return false; //we can't send links if a locale file is in link
			}

			return true; //we will send a link
		}
		else if( e->provides( "application/x-qlistviewitem" ) )
		{
			//Coming from chat members
			return true;
		}
		else
		{
			QString text;
			QTextDrag::decode(e, text);
			kdDebug(14000) << k_funcinfo << "drop with mimetype:" << e->format() << " data as text:" << text << endl;
		}

	}

	return false;
}

void KopeteContactListView::findDrop(const QPoint &pos, QListViewItem *&parent,
	QListViewItem *&after)
{
	//Since KDE 3.1.1 ,  the original find Drop return 0L for afterme if the group is open.
	//This woraround allow us to keep the highlight of the item, and give always a correct position
	parent=0L;
	QPoint p (contentsToViewport(pos));
	after=itemAt(p);
}


void KopeteContactListView::contentsMousePressEvent( QMouseEvent *e )
{
	KListView::contentsMousePressEvent( e );
	if (e->button() == LeftButton )
	{
		QPoint p=contentsToViewport(e->pos());
		QListViewItem *i=itemAt( p );
		if( i )
		{
			//Maybe we are starting a drag?
			//memorize the position to know later if the user move a small contacticon

			int px = p.x() - ( header()->sectionPos( header()->mapToIndex( 0 ) ) +
				treeStepSize() * ( i->depth() + ( rootIsDecorated() ? 1 : 0 ) ) + itemMargin() );
			int py = p.y() - itemRect( i ).y();

			m_startDragPos = QPoint( px , py );
		}
	}
}

void KopeteContactListView::slotNewMessageEvent(Kopete::MessageEvent *event)
{
	Kopete::Message msg=event->message();
	//only for single chat
	if(msg.from() && msg.to().count()==1)
	{
		Kopete::MetaContact *m=msg.from()->metaContact();
		if(!m)
			return;

		for ( QListViewItem *item = firstChild(); item; item = nextItem(item) )
			if ( KopeteMetaContactLVI *li = dynamic_cast<KopeteMetaContactLVI*>(item) )
				if ( li->metaContact() == m )
					li->catchEvent(event);
	}
}

QDragObject *KopeteContactListView::dragObject()
{
	// Discover what the drag started on.
	// If it's a MetaContactLVI, it was either on the MCLVI itself
	// or on one of the child contacts
	// Once we know this,
	// we set the pixmap for the drag to the MC's pixmap
	// or the child contact's small icon

	QListViewItem *currentLVI = currentItem();
	if( !currentLVI )
		return 0L;

	KopeteMetaContactLVI *metaLVI = dynamic_cast<KopeteMetaContactLVI*>( currentLVI );
	if( !metaLVI )
		return 0L;

	QPixmap pm;
	Kopete::Contact *c = metaLVI->contactForPoint( m_startDragPos );
        KMultipleDrag *drag = new KMultipleDrag( this );
	drag->addDragObject( new QStoredDrag("application/x-qlistviewitem", 0L ) );

	QStoredDrag *d = new QStoredDrag("kopete/x-metacontact", 0L );
	d->setEncodedData( metaLVI->metaContact()->metaContactId().utf8() );
	drag->addDragObject( d );

	if ( c ) 	// dragging a contact
	{
		QStoredDrag *d = new QStoredDrag("kopete/x-contact", 0L );
		d->setEncodedData( QString( c->protocol()->pluginId() +QChar( 0xE000 )+ c->account()->accountId() +QChar( 0xE000 )+ c->contactId() ).utf8() );
		drag->addDragObject( d );

		pm = c->onlineStatus().iconFor( c, 12 ); // FIXME: fixed icon scaling
	}
	else		// dragging a metacontact
	{
		// FIXME: first start at rendering the whole MC incl small icons
		// into a pixmap to drag - anyone know how to complete this?
		//QPainter p( pm );
		//source_metaLVI->paintCell( p, cg?, width(), 0, 0 );
		pm = SmallIcon( metaLVI->metaContact()->statusIcon() );
	}

	KABC::Addressee address = KABC::StdAddressBook::self()->findByUid(
		metaLVI->metaContact()->metaContactId()
	);

	if( !address.isEmpty() )
	{
		drag->addDragObject( new QTextDrag( address.fullEmail(), 0L ) );
		KABC::VCardConverter converter;
		QString vcard = converter.createVCard( address );
		if( !vcard.isNull() )
		{
			QStoredDrag *vcardDrag = new QStoredDrag("text/x-vcard", 0L );
			vcardDrag->setEncodedData( vcard.utf8() );
			drag->addDragObject( vcardDrag );
		}
	}

	//QSize s = pm.size();
	drag->setPixmap( pm /*, QPoint( s.width() , s.height() )*/ );

	return drag;
}

void KopeteContactListView::slotViewSelectionChanged()
{
	QPtrList<Kopete::MetaContact> contacts;
	QPtrList<Kopete::Group> groups;

	m_selectedContacts.clear();
	m_selectedGroups.clear();

	QListViewItemIterator it( this );
	while ( it.current() )
	{
		QListViewItem *item = it.current();
		++it;

		if ( item->isSelected() )
		{
			KopeteMetaContactLVI *metaLVI=dynamic_cast<KopeteMetaContactLVI*>(item);
			if(metaLVI)
			{
				m_selectedContacts.append( metaLVI );
				if(!contacts.contains(metaLVI->metaContact()))
					contacts.append( metaLVI->metaContact() );
			}
			KopeteGroupViewItem *groupLVI=dynamic_cast<KopeteGroupViewItem*>(item);
			if(groupLVI)
			{
				m_selectedGroups.append( groupLVI );
				if(!groups.contains(groupLVI->group()))
					groups.append( groupLVI->group() );
				
			}
		}
	}

	// will cause slotListSelectionChanged to be called to update our actions.
	Kopete::ContactList::self()->setSelectedItems(contacts , groups);
}

void KopeteContactListView::slotListSelectionChanged()
{
	QPtrList<Kopete::MetaContact> contacts = Kopete::ContactList::self()->selectedMetaContacts();
	QPtrList<Kopete::Group> groups = Kopete::ContactList::self()->selectedGroups();

	//TODO: update the list to select the items that should be selected.
	// make sure slotViewSelectionChanged is *not* called.
	updateActionsForSelection( contacts, groups );
}

void KopeteContactListView::updateActionsForSelection(
	QPtrList<Kopete::MetaContact> contacts, QPtrList<Kopete::Group> groups )
{
	bool singleContactSelected = groups.isEmpty() && contacts.count() == 1;
	bool inkabc=false;
	if(singleContactSelected)
	{
		QString kabcid=contacts.first()->metaContactId();
		inkabc= !kabcid.isEmpty() && !kabcid.contains(":");
	}

	actionSendFile->setEnabled( singleContactSelected && contacts.first()->canAcceptFiles());
	actionAddContact->setEnabled( singleContactSelected && !contacts.first()->isTemporary());
	actionSendEmail->setEnabled( inkabc );

	if( singleContactSelected )
	{
		actionRename->setText(i18n("Rename Contact"));
		actionRemove->setText(i18n("Remove Contact"));
		actionSendMessage->setText(i18n("Send Single Message..."));
		actionRename->setEnabled(true);
		actionRemove->setEnabled(true);
		actionAddContact->setText(i18n("&Add Subcontact"));
		actionAddContact->setEnabled(!contacts.first()->isTemporary());
	}
	else if( groups.count() == 1 && contacts.isEmpty() )
	{
		actionRename->setText(i18n("Rename Group"));
		actionRemove->setText(i18n("Remove Group"));
		actionSendMessage->setText(i18n("Send Message to Group"));
		actionRename->setEnabled(true);
		actionRemove->setEnabled(true);
		actionSendMessage->setEnabled(true);
		actionAddContact->setText(i18n("&Add Contact to Group"));
		actionAddContact->setEnabled(groups.first()->type()==Kopete::Group::Normal);
	}
	else
	{
		actionRename->setText(i18n("Rename"));
		actionRemove->setText(i18n("Remove"));
		actionRename->setEnabled(false);
		actionRemove->setEnabled(contacts.count()+groups.count());
		actionAddContact->setEnabled(false);
	}

	actionMove->setCurrentItem( -1 );
	actionCopy->setCurrentItem( -1 );

	actionProperties->setEnabled( ( groups.count() + contacts.count() ) == 1 );
}

void KopeteContactListView::slotSendMessage()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	Kopete::Group *group = Kopete::ContactList::self()->selectedGroups().first();
	if(m)
		m->sendMessage();
	else
		if(group)
			group->sendMessage();
}

void KopeteContactListView::slotStartChat()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	if(m)
		m->startChat();
}

void KopeteContactListView::slotSendFile()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	if(m)
		m->sendFile(KURL());
}

 void KopeteContactListView::slotSendEmail()
{
	//I borrowed this from slotSendMessage
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	if ( !m->metaContactId().isEmpty( ) ) // check if in kabc
	{
		KABC::Addressee addressee = KABC::StdAddressBook::self()->findByUid( m->metaContactId() );
		if ( !addressee.isEmpty() )
		{
			QString emailAddr = addressee.fullEmail();

			kdDebug( 14000 ) << "Email: " << emailAddr << "!" << endl;
			if ( !emailAddr.isEmpty() )
				kapp->invokeMailer( emailAddr, QString::null );
			else
				KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "There is no email address set for this contact in the KDE address book." ), i18n( "No Email Address in Address Book" ) );
		}
		else
			KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "This contact was not found in the KDE address book. Check that a contact is selected in the properties dialog." ), i18n( "Not Found in Address Book" ) );
	}
	else
		KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "This contact is not associated with a KDE address book entry, where the email address is stored. Check that a contact is selected in the properties dialog." ), i18n( "Not Found in Address Book" ) );
}

void KopeteContactListView::slotMoveToGroup()
{
	KopeteMetaContactLVI *metaLVI=dynamic_cast<KopeteMetaContactLVI*>(currentItem());
	if(!metaLVI)
		return;
	Kopete::MetaContact *m=metaLVI->metaContact();
	Kopete::Group *g=metaLVI->group();

	//FIXME What if two groups have the same name?
	Kopete::Group *to = actionMove->currentItem() ?
		Kopete::ContactList::self()->findGroup( actionMove->currentText() ) :
		Kopete::Group::topLevel();

	if( !to || to->type() == Kopete::Group::Temporary )
		return;

	if(m->isTemporary())
	{
		if( KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
			i18n( "<qt>Would you like to add this contact to your contact list?</qt>" ),
			i18n( "Kopete" ), i18n("Add"), i18n("Do Not Add"),
			"addTemporaryWhenMoving" ) == KMessageBox::Yes )
		{
			m->setTemporary(false,to);

			insertUndoItem( new UndoItem( UndoItem::MetaContactAdd , m  ) );
		}
	}
	else if( !m->groups().contains( to ) )
	{
		m->moveToGroup( g, to );

		insertUndoItem( new UndoItem( UndoItem::MetaContactCopy , m , to ) );

		UndoItem *u=new UndoItem( UndoItem::MetaContactRemove, m, g );
		u->isStep=false;
		insertUndoItem(u);
	}

	actionMove->setCurrentItem( -1 );
}

void KopeteContactListView::slotCopyToGroup()
{
	Kopete::MetaContact *m =
		Kopete::ContactList::self()->selectedMetaContacts().first();

	if(!m)
		return;

	//FIXME! what if two groups have the same name?
	Kopete::Group *to = actionCopy->currentItem() ?
		Kopete::ContactList::self()->findGroup( actionCopy->currentText() ) :
		Kopete::Group::topLevel();

	if( !to || to->type() == Kopete::Group::Temporary )
		return;

	if( m->isTemporary() )
		return;

	if( !m->groups().contains( to ) )
	{
		m->addToGroup( to );

		insertUndoItem( new UndoItem( UndoItem::MetaContactCopy , m , to ) );
	}

	actionCopy->setCurrentItem( -1 );
}



void KopeteContactListView::slotRemove()
{
	QPtrList<Kopete::MetaContact> contacts = Kopete::ContactList::self()->selectedMetaContacts();
	QPtrList<Kopete::Group> groups = Kopete::ContactList::self()->selectedGroups();

	if(groups.count() + contacts.count() == 0)
		return;

	QStringList items;
	for( Kopete::Group *it = groups.first(); it; it = groups.next() )
	{
		if(!it->displayName().isEmpty())
			items.append( it->displayName() );
	}
	for( Kopete::MetaContact *it = contacts.first(); it; it = contacts.next() )
	{
		if(!it->displayName().isEmpty() )
			items.append( it->displayName() );
	}

	if( items.count() <= 1 )
	{
		// we are deleting an empty contact
		QString msg;
		if( !contacts.isEmpty() )
		{
			msg = i18n( "<qt>Are you sure you want to remove the contact <b>%1</b>" \
			            " from your contact list?</qt>" )
			      .arg( contacts.first()->displayName() ) ;
		}
		else if( !groups.isEmpty() )
		{
			msg = i18n( "<qt>Are you sure you want to remove the group <b>%1</b> " \
			            "and all contacts that are contained within it?</qt>" )
			      .arg( groups.first()->displayName() );
		}
		else
			return; // this should never happen

		if( KMessageBox::warningContinueCancel( this, msg, i18n( "Remove" ), KGuiItem(i18n("Remove"),"editdelete") ,
		 "askRemovingContactOrGroup" , KMessageBox::Notify | KMessageBox::Dangerous ) !=
			KMessageBox::Continue )
		{
			return;
		}
	}
	else
	{
		QString msg = groups.isEmpty() ?
		   i18n( "Are you sure you want to remove these contacts " \
		         "from your contact list?" ) :
		   i18n( "Are you sure you want to remove these groups and " \
		         "contacts from your contact list?" );

		if( KMessageBox::warningContinueCancelList( this, msg, items, i18n("Remove"),
			KGuiItem(i18n("Remove"),"editdelete"), "askRemovingContactOrGroup",
			KMessageBox::Notify | KMessageBox::Dangerous ) != KMessageBox::Continue )
		{
			return;
		}
	}

	bool undo_step=true; //only  the first undo item we will add will be a step

	for( Kopete::MetaContact *mc = contacts.first(); mc; mc = contacts.next() )
	{
		if(mc->groups().count()==1 || mc->isTemporary() )
			Kopete::ContactList::self()->removeMetaContact( mc );
		else
		{
			//try to guess from what group we are removing that contact.
			QListViewItemIterator lvi_it( this );
			while ( lvi_it.current() )
			{
				QListViewItem *item = lvi_it.current();
				++lvi_it;

				if ( item->isSelected() )
				{
					KopeteMetaContactLVI *metaLVI=dynamic_cast<KopeteMetaContactLVI*>(item);
					if(metaLVI && metaLVI->metaContact() == mc )
					{
						if(mc->groups().count()==1)
						{
							Kopete::ContactList::self()->removeMetaContact( mc );
							break;
						}
						else
						{
							mc->removeFromGroup(metaLVI->group());
							insertUndoItem( new UndoItem( UndoItem::MetaContactRemove , mc , metaLVI->group() ) );
							m_undo->isStep=undo_step; //if there is several selected contacts.
							undo_step=false;
						}
						//let's continue, it's possible this contact is selected several times
					}
				}
			}
		}
	}

	for( Kopete::Group *it = groups.first(); it; it = groups.next() )
	{
		QPtrList<Kopete::MetaContact> list = it->members();
		for( list.first(); list.current(); list.next() )
		{
			Kopete::MetaContact *mc = list.current();
			if(mc->groups().count()==1)
				Kopete::ContactList::self()->removeMetaContact(mc);
			else
				mc->removeFromGroup(it);
		}

		if( !it->members().isEmpty() )
		{
			kdDebug(14000) << "KopeteContactListView::slotRemove(): "
				<< "all subMetaContacts are not removed... Aborting" << endl;
			continue;
		}

		Kopete::ContactList::self()->removeGroup( it );
	}
}

void KopeteContactListView::slotRename()
{
	if ( KopeteMetaContactLVI *metaLVI = dynamic_cast<KopeteMetaContactLVI *>( currentItem() ) )
	{
		metaLVI->setRenameEnabled( 0, true);
		metaLVI->startRename( 0 );
	}
	else if ( KopeteGroupViewItem *groupLVI = dynamic_cast<KopeteGroupViewItem *>( currentItem() ) )
	{
		if ( !KopetePrefs::prefs()->sortByGroup() )
			return;
		groupLVI->setRenameEnabled( 0, true);
		groupLVI->startRename( 0 );
	}
}

void KopeteContactListView::slotAddContact()
{
	if( !sender() )
		return;

	Kopete::MetaContact *metacontact =
			Kopete::ContactList::self()->selectedMetaContacts().first();
	Kopete::Group *group =
			Kopete::ContactList::self()->selectedGroups().first();
	Kopete::Account *account = dynamic_cast<Kopete::Account*>( sender()->parent() );

	if ( ( metacontact && metacontact->isTemporary() ) ||
			  (group && group->type()!=Kopete::Group::Normal ) )
		return;


	if( account && ( metacontact || group) )
	{
		KDialogBase *addDialog = new KDialogBase( this, "addDialog", true,
			i18n( "Add Contact" ), KDialogBase::Ok|KDialogBase::Cancel,
			KDialogBase::Ok, true );

		AddContactPage *addContactPage =
			account->protocol()->createAddContactWidget( addDialog, account );

		if (!addContactPage)
		{
			kdDebug(14000) << k_funcinfo <<
				"Error while creating addcontactpage" << endl;
		}
		else
		{
			addDialog->setMainWidget( addContactPage );
			if( addDialog->exec() == QDialog::Accepted )
			{
				if( addContactPage->validateData() )
				{
					if(!metacontact)
					{
						metacontact = new Kopete::MetaContact();
						metacontact->addToGroup( group );
						if (addContactPage->apply( account, metacontact ))
						{
							Kopete::ContactList::self()->addMetaContact( metacontact );
						}
						else
						{
							delete metacontact;
						}
					}
					else
					{
						addContactPage->apply( account, metacontact );
					}
				}
			}
		}
		addDialog->deleteLater();
	}
}

void KopeteContactListView::slotAddTemporaryContact()
{
	Kopete::MetaContact *metacontact =
		Kopete::ContactList::self()->selectedMetaContacts().first();
	if( metacontact )
	{
/*		int r=KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
			i18n( "<qt>Would you like to add this contact to your contact list?</qt>" ),
			i18n( "Kopete" ), i18n("Add"), i18n("Do Not Add"),
			"addTemporaryWhenMoving" );

		if(r==KMessageBox::Yes)*/
		if(metacontact->isTemporary() )
			metacontact->setTemporary( false );
	}
}

void KopeteContactListView::slotProperties()
{
//	kdDebug(14000) << k_funcinfo << "Called" << endl;

	KopeteMetaContactLVI *metaLVI =
		dynamic_cast<KopeteMetaContactLVI *>( currentItem() );
	KopeteGroupViewItem *groupLVI =
		dynamic_cast<KopeteGroupViewItem *>( currentItem() );

	if(metaLVI)
	{

		KopeteMetaLVIProps *propsDialog =
			new KopeteMetaLVIProps( metaLVI, 0L, "propsDialog" );

		propsDialog->exec(); // modal
		delete propsDialog;

		/*
		if( metaLVI->group()->useCustomIcon() )
		{
			metaLVI->updateCustomIcons( mShowAsTree );
		}
		else
		{
		}
		*/
		metaLVI->repaint();
	}
	else if(groupLVI)
	{
		KopeteGVIProps *propsDialog =
			new KopeteGVIProps( groupLVI, 0L, "propsDialog");

		propsDialog->exec(); // modal
		delete propsDialog;

		groupLVI->updateIcon();
	}
}

void KopeteContactListView::slotItemRenamed( QListViewItem */*item*/ )
{
	//everithing is now done in  KopeteMetaContactLVI::rename

/*	KopeteMetaContactLVI *metaLVI = dynamic_cast<KopeteMetaContactLVI *>( item );
	Kopete::MetaContact *m= metaLVI ?  metaLVI->metaContact() : 0L ;
	if ( m )
	{
		m->setDisplayName( metaLVI->text( 0 ) );
	}
	else
	{
		//group are handled differently in KopeteGroupViewItem
	//	kdWarning( 14000 ) << k_funcinfo << "Unknown list view item '" << item
	//	                   << "' renamed, ignoring item" << endl;
	}
	*/
}

void KopeteContactListView::insertUndoItem( KopeteContactListView::UndoItem *u)
{
	u->next=m_undo;
	m_undo=u;
	actionUndo->setEnabled(true);
	while(m_redo)
	{
		UndoItem *i=m_redo->next;
		delete m_redo;
		m_redo=i;
	}
	actionRedo->setEnabled(false);
	undoTimer.start(10*60*1000);
}


void KopeteContactListView::slotUndo()
{
	bool step = false;
	while(m_undo && !step)
	{
		bool success=false;
		switch (m_undo->type)
		{
		 case UndoItem::MetaContactAdd:
		 {
		 	Kopete::MetaContact *m=m_undo->metacontact;
			if(m)
			{
				m->setTemporary(true);
				success=true;
			}
			break;
		 }
		 case UndoItem::MetaContactCopy:
		 {
		 	Kopete::MetaContact *m=m_undo->metacontact;
			Kopete::Group *to=m_undo->group;
			if( m && to )
			{
				m->removeFromGroup( to );
				success=true;
			}
			break;
		 }
		 case UndoItem::MetaContactRemove:
		 {
		 	Kopete::MetaContact *m=m_undo->metacontact;
			Kopete::Group *g=m_undo->group;
			if( m && g )
			{
				m->addToGroup( g );
				success=true;
			}
			break;
		 }
		 case UndoItem::MetaContactRename:
		 {
			Kopete::MetaContact *m=m_undo->metacontact;
			if( m )
			{
				// make a copy
				QStringList undoArgs = m_undo->args;
				Kopete::MetaContact::PropertySource undoSource = m_undo->nameSource;
				// set undo undo
				// set the source first
				m_undo->nameSource = m->displayNameSource();
				if ( m->displayNameSource() == Kopete::MetaContact::SourceCustom )
				{
					m_undo->args[0] = m->customDisplayName();
				}
				else if ( m->displayNameSource() == Kopete::MetaContact::SourceContact )
				{
					Kopete::Contact* c = m->displayNameSourceContact();
					m_undo->args[0] = c->contactId();
					m_undo->args[1] = c->protocol()->pluginId();
					m_undo->args[2] = c->account()->accountId();
				}
				// source kabc requires no arguments
			
				// do the undo
				if ( undoSource == Kopete::MetaContact::SourceContact )
				{ // do undo
					Kopete::Contact *c = Kopete::ContactList::self()->findContact( undoArgs[1], undoArgs[2], undoArgs[0]);
					if (!c)
					{
						success=false;
						break;
					}
					// do undo
					m->setDisplayNameSourceContact(c);
					m->setDisplayNameSource(Kopete::MetaContact::SourceContact);
				}
				else if ( undoSource == Kopete::MetaContact::SourceCustom )
				{
					m->setDisplayName(undoArgs[0]);
					m->setDisplayNameSource(Kopete::MetaContact::SourceCustom);
				}
				else if ( undoSource == Kopete::MetaContact::SourceKABC )
				{
					m->setDisplayNameSource(Kopete::MetaContact::SourceKABC);
				}
				success=true;
			}
			break;
		 }
		 case UndoItem::GroupRename:
		 {
			if( m_undo->group  )
			{
				const QString old=m_undo->group->displayName();
				m_undo->group->setDisplayName( m_undo->args[0] );
				m_undo->args[0]=old;
				success=true;
			}
			break;
		 }
		 case UndoItem::MetaContactChange:
		 {
		 	Kopete::Contact *c=Kopete::ContactList::self()->findContact(m_undo->args[0] , m_undo->args[1], m_undo->args[2] ) ;
			if(c)
			{
				success=true;
				if(m_undo->metacontact)
					c->setMetaContact(m_undo->metacontact);
				else
				{
					Kopete::MetaContact *m=new Kopete::MetaContact;
					m->addToGroup(m_undo->group);
					m->setDisplayName(m_undo->args[3]);
					c->setMetaContact(m);
					Kopete::ContactList::self()->addMetaContact(m);
				}
				m_undo->metacontact=c->metaContact(); //for the redo
			}
			break;
		 }
		 case UndoItem::ContactAdd:
		 {
			Kopete::Contact *c=Kopete::ContactList::self()->findContact(m_undo->args[0] , m_undo->args[1], m_undo->args[2] ) ;
			if(c)
			{
				success=true;
				Kopete::MetaContact *m=new Kopete::MetaContact;
				m->setTemporary(true);
				c->setMetaContact(m);
				Kopete::ContactList::self()->addMetaContact(m);
				m_undo->metacontact=c->metaContact();
			}
			break;
		 }
		}

		if(success) //the undo item has been correctly performed
		{
			step=m_undo->isStep;
			UndoItem *u=m_undo->next;
			m_undo->next=m_redo;
			m_redo=m_undo;
			m_undo=u;
		}
		else //something has been corrupted, clear all undo items
		{
			while(m_undo)
			{
				UndoItem *u=m_undo->next;
				delete m_undo;
				m_undo=u;
			}
		}
	}
	actionUndo->setEnabled(m_undo);
	actionRedo->setEnabled(m_redo);
	undoTimer.start(10*60*1000);
}

void KopeteContactListView::slotRedo()
{
	bool step = false;
	while(m_redo && (!step || !m_redo->isStep ))
	{
		bool success=false;
		switch (m_redo->type)
		{
		 case UndoItem::MetaContactAdd:
		 {
		 	Kopete::MetaContact *m=m_redo->metacontact;
			if(m && m_redo->group)
			{
				m->setTemporary(false,m_redo->group);
				success=true;
			}
			break;
		 }
		 case UndoItem::MetaContactCopy:
		 {
		 	Kopete::MetaContact *m=m_redo->metacontact;
			Kopete::Group *to=m_redo->group;
			if( m && to )
			{
				m->addToGroup( to );
				success=true;
			}
			break;
		 }
		 case UndoItem::MetaContactRemove:
		 {
		 	Kopete::MetaContact *m=m_redo->metacontact;
			Kopete::Group *g=m_redo->group;
			if( m && g )
			{
				m->removeFromGroup( g );
				success=true;
			}
			break;
		 }
		 case UndoItem::MetaContactRename:
		 {
			/*
			Kopete::MetaContact *m=m_redo->metacontact;
			if( m )
			{
				const QString old=m->displayName();
				if( m_redo->args[1].isEmpty() )
				{
					const QString name = m_redo->args[0];
					m_redo->args[0] = m->displayNameSource()->contactId();
					m_redo->args[1] = m->displayNameSource()->protocol()->pluginId();
					m_redo->args[2] = m->displayNameSource()->account()->accountId();
					m->setDisplayName( name );
				}
				else
				{
					const QString oldName = m->displayName();
					QPtrList< Kopete::Contact > cList = m->contacts();
					QPtrListIterator< Kopete::Contact > it (cList);
					Kopete::Contact *c = Kopete::ContactList::self()->findContact( args[0], args[2], args[1]);
					if ( !c)
						return;
					m->setNameSourceContact(c);
					break;

					m_redo->args[0] = oldName;
					m_redo->args[1] = "";
					m_redo->args[2] = "";
				}
				success=true;
			}
			*/ //Why is this code commented ?   - Olivier 2006-04
			break;
		 }
		 case UndoItem::GroupRename:
		 {
			if( m_redo->group  )
			{
				const QString old=m_redo->group->displayName();
				m_redo->group->setDisplayName( m_redo->args[0] );
				m_redo->args[0]=old;
				success=true;
			}
			break;
		 }
		 case UndoItem::MetaContactChange:
		 case UndoItem::ContactAdd:
		 {
		 	Kopete::Contact *c=Kopete::ContactList::self()->findContact(m_redo->args[0] , m_redo->args[1], m_redo->args[2] ) ;
			if(c && m_redo->metacontact)
			{
				success=true;
				c->setMetaContact(m_redo->metacontact);
				m_redo->metacontact=c->metaContact();
			}
			break;
		 }
		}

		if(success) //the undo item has been correctly performed
		{
			step=true;
			UndoItem *u=m_redo->next;
			m_redo->next=m_undo;
			m_undo=m_redo;
			m_redo=u;
		}
		else //something has been corrupted, clear all undo items
		{
			while(m_redo)
			{
				UndoItem *u=m_redo->next;
				delete m_redo;
				m_redo=u;
			}
		}
	}
	actionUndo->setEnabled(m_undo);
	actionRedo->setEnabled(m_redo);
	undoTimer.start(10*60*1000);
}

void KopeteContactListView::slotTimeout()
{
	undoTimer.stop();

	//we will keep one (complete) undo action
	UndoItem *Sdel=m_undo;
	while(Sdel && !Sdel->isStep)
		Sdel=Sdel->next;

	if(Sdel) while( Sdel->next )
	{
		UndoItem *u=Sdel->next->next;
		delete Sdel->next;
		Sdel->next=u;
	}
	actionUndo->setEnabled(m_undo);
	while(m_redo)
	{
		UndoItem *i=m_redo->next;
		delete m_redo;
		m_redo=i;
	}
	actionRedo->setEnabled(false);
}

#include "kopetecontactlistview.moc"

// vim: set noet ts=4 sts=4 sw=4:
