/*
    kopetecontactlistview.cpp

    Kopete Contactlist GUI

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Nick Betcher           <nbetcher@usinternet.com>
    Copyright (c) 2002      by Stefan Gehn            <metz AT gehn.net>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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
#if KDE_IS_VERSION( 3, 1, 90 )
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

#include "addcontactwizard.h"
#include "addcontactpage.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontactlist.h"
#include "kopeteevent.h"
#include "kopetegroup.h"
#include "kopetegroupviewitem.h"
#include "kopetemetacontact.h"
#include "kopetemetacontactlvi.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetestatusgroupviewitem.h"
#include "kopetestdaction.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetecontact.h"
#include "kopetemessagemanager.h" //needed to send the URL
#include "kopetemessage.h"       //needed to send the URL

#include "kopetelviprops.h"

class KopeteContactListViewPrivate
{
public:
	QTimer *sortTimer;
};

/*
	Custom QToolTip for the contact list.
	The decision whether or not to show tooltips is taken in
	maybeTip(). See also the QListView sources from Qt itself.
*/
class KopeteContactListViewToolTip : public QToolTip
{
public:
	KopeteContactListViewToolTip( QWidget *parent, KopeteContactListView *lv );
	virtual ~KopeteContactListViewToolTip();

	void maybeTip( const QPoint &pos );

private:
	QString idleTime2String( int seconds );
private:
	KopeteContactListView *m_listView;
};

KopeteContactListViewToolTip::KopeteContactListViewToolTip( QWidget *parent,
	KopeteContactListView *lv )
: QToolTip( parent )
{
	m_listView = lv;
}

KopeteContactListViewToolTip::~KopeteContactListViewToolTip()
{
	QMimeSourceFactory::defaultFactory()->setImage( "kopete:icon", 0 );
}

void KopeteContactListViewToolTip::maybeTip( const QPoint &pos )
{
	if( !parentWidget() || !m_listView )
		return;

	QListViewItem *item = m_listView->itemAt( pos );
	if( !item )
		return;

	KopeteMetaContactLVI *metaLVI = dynamic_cast<KopeteMetaContactLVI *>( item );
	KopeteGroupViewItem *groupLVI = dynamic_cast<KopeteGroupViewItem *>( item );

	// FIXME: add more info to the tooltips! they are sorta bare at the
	// moment! Time last online, Idle time, members of groups, etc... - Martijn
	KopeteContact *contact = 0L;
	QString toolTip;
	QRect itemRect = m_listView->itemRect( item );

	if( metaLVI )
	{
		uint leftMargin = m_listView->treeStepSize() *
				( item->depth() + ( m_listView->rootIsDecorated() ? 1 : 0 ) ) +
				m_listView->itemMargin();

		if( metaLVI->metaContact()->contacts().count() == 1 )
		{
			contact = metaLVI->metaContact()->contacts().first();
		}
		else
		{
			// Check if we are hovering over a protocol icon. If so, use that
			// tooltip in the code below
			uint xAdjust = itemRect.left() + leftMargin;
			uint yAdjust = itemRect.top();
			QPoint relativePos( pos.x() - xAdjust, pos.y() - yAdjust );
			contact = metaLVI->contactForPoint( relativePos );

			if( contact )
			{
				QRect iconRect = metaLVI->contactRect( contact );

				itemRect = QRect(
					iconRect.left() + xAdjust,
					iconRect.top() + yAdjust,
					iconRect.width(),
					iconRect.height() );
			}
		}

		if ( contact )
		{
			if ( !contact->toolTip().isNull() )
				toolTip = contact->toolTip();
		}
		else
		{
			KopeteMetaContact *mc = metaLVI->metaContact();
			toolTip = i18n( "<b>%2</b><br><img src=\"kopete:icon\">&nbsp;%1" ).
#if QT_VERSION < 0x030200
				arg( mc->statusString() ).arg( QStyleSheet::escape( mc->displayName() ) );
#else
				arg( mc->statusString(), QStyleSheet::escape( mc->displayName() ) );
#endif
			QMimeSourceFactory::defaultFactory()->setImage( "kopete:icon",
				SmallIcon( mc->statusIcon() ).convertToImage() );

			if( mc->idleTime() > 0 )
				toolTip += idleTime2String(mc->idleTime());

			// Adjust the item rect on the right
			uint first = metaLVI->firstContactIconX();
			uint last  = metaLVI->lastContactIconX();

			if ( first != last )
			{
				if ( pos.x() > int( itemRect.left() + leftMargin + first ) )
					itemRect.setLeft( itemRect.left() + leftMargin + last );
				else
					itemRect.setWidth( leftMargin + first );
			}
		}
	}
	else if( groupLVI )
	{
		// FIXME: use item->text( 0 ) for now so we get the # online / # total, since there is no
		//        interface to get these from KopeteGroup
		//KopeteGroup *g=groupLVI->group();
		toolTip = QString( "<b>%1</b>" ).arg( item->text( 0 ) );
	}

	//kdDebug( 14000 ) << k_funcinfo << "Adding tooltip: itemRect: " << itemRect << ", tooltip:  " << toolTip << endl;
	tip( itemRect, toolTip );
}

QString KopeteContactListViewToolTip::idleTime2String( int idleSeconds )
{
	unsigned int days, hours, mins, secs, left;
	days = idleSeconds / ( 60*60*24 );
	left = idleSeconds % ( 60*60*24 );
	hours = left / ( 60*60 );
	left = left % ( 60*60 );
	mins = left / 60;
	secs = left % 60;
	if ( days!=0 )
		return i18n( "<br>Idle: %1d %2h %3m %4s" ).arg( days ).arg( hours ).arg( mins ).arg( secs );
	else if ( hours!=0 )
		return i18n( "<br>Idle: %1h %2m %3s" ).arg( hours ).arg( mins ).arg( secs );
	else
		return i18n( "<br>Idle: %1m %2s" ).arg( mins ).arg( secs );
}

KopeteContactListView::KopeteContactListView( QWidget *parent, const char *name )
: KListView( parent, name )
{
	d = new KopeteContactListViewPrivate;
	d->sortTimer = new QTimer( this, "sortTimer" );
	connect( d->sortTimer, SIGNAL( timeout() ), this, SLOT( slotSort() ) );

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

	if ( KopetePrefs::prefs()->sortByGroup() )
	{
		m_offlineItem = 0L;
		m_onlineItem = 0L;
	}
	else
	{
		m_offlineItem = new KopeteStatusGroupViewItem( KopeteOnlineStatus::Offline, this );
		m_onlineItem = new KopeteStatusGroupViewItem( KopeteOnlineStatus::Online, this );
		m_onlineItem->setOpen( true );
		m_offlineItem->setOpen( true );
	}

	setFullWidth( true );

	// We have our own tooltips, don't use the default QListView ones
	setShowToolTips( false );

	m_tooltip = new KopeteContactListViewToolTip( viewport(), this );

	connect( this, SIGNAL( contextMenu( KListView *, QListViewItem *, const QPoint & ) ),
		SLOT( slotContextMenu( KListView *, QListViewItem *, const QPoint & ) ) );
	connect( this, SIGNAL( expanded( QListViewItem * ) ), SLOT( slotExpanded( QListViewItem * ) ) );
	connect( this, SIGNAL( collapsed( QListViewItem * ) ), SLOT( slotCollapsed( QListViewItem * ) ) );
	connect( this, SIGNAL( executed( QListViewItem *, const QPoint &, int ) ), SLOT( slotExecuted( QListViewItem *, const QPoint &, int ) ) );
	connect( this, SIGNAL( doubleClicked( QListViewItem * ) ), SLOT( slotDoubleClicked( QListViewItem * ) ) );
	connect( this, SIGNAL( selectionChanged() ), SLOT( slotViewSelectionChanged() ) );
	connect( this, SIGNAL( itemRenamed( QListViewItem * ) ), this, SLOT( slotItemRenamed( QListViewItem * ) ) );

	connect( KopetePrefs::prefs(), SIGNAL( saved() ), SLOT( slotSettingsChanged() ) );

	connect( KopeteContactList::contactList(), SIGNAL( selectionChanged() ),
	         SLOT( slotListSelectionChanged() ) );
	connect( KopeteContactList::contactList(), SIGNAL( metaContactAdded( KopeteMetaContact * ) ),
		SLOT( slotMetaContactAdded( KopeteMetaContact * ) ) );
	connect( KopeteContactList::contactList(), SIGNAL( metaContactDeleted( KopeteMetaContact * ) ),
		SLOT( slotMetaContactDeleted( KopeteMetaContact * ) ) );
	connect( KopeteContactList::contactList(), SIGNAL( groupAdded( KopeteGroup * ) ),
		SLOT( slotGroupAdded( KopeteGroup * ) ) );

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( newEvent( KopeteEvent * ) ),
		this, SLOT( slotNewMessageEvent( KopeteEvent * ) ) );

	connect( this, SIGNAL( dropped( QDropEvent *, QListViewItem *, QListViewItem * ) ),
		this, SLOT( slotDropped( QDropEvent *, QListViewItem *, QListViewItem * ) ) );

	//connect( this , SIGNAL( onItem( QListViewItem * ) ),
	//	this, SLOT ( slotOnItem( QListViewItem * ) ) );

	addColumn( i18n( "Contacts" ), 0 );

	closed  = SmallIcon( "folder_green" );
	open    = SmallIcon( "folder_green_open" );
	classic = SmallIcon( "folder_blue" );

	// Add the already existing groups now
	for ( QPtrListIterator<KopeteGroup> groupIt( KopeteContactList::contactList()->groups() ); groupIt.current(); ++groupIt )
		getGroup( groupIt.current(), true );

	// Add the already existing meta contacts now
	QPtrList<KopeteMetaContact> metaContacts = KopeteContactList::contactList()->metaContacts();
	for ( QPtrListIterator<KopeteMetaContact> it( metaContacts ); it.current(); ++it )
		slotMetaContactAdded( it.current() );

	setAutoOpen( true );
	setDragEnabled( true );
	setAcceptDrops( true );
	setItemsMovable( false );
	setDropVisualizer( false );
	setDropHighlighter( true );
	setSelectionMode( QListView::Extended );

	clearWFlags( WStaticContents | WNoAutoErase );

	// clear the appropriate flags from the viewport - qt docs say we have to mask
	// these flags out of the QListView to make weirdly painted list items work, but
	// that doesn't do the job. this does.
	class MyWidget : public QWidget { public: QWidget::clearWFlags; };
	static_cast<MyWidget*>( viewport() )->clearWFlags( WStaticContents | WNoAutoErase );
}

void KopeteContactListView::initActions( KActionCollection *ac )
{
	new KAction( i18n( "Create New Group..." ), 0, 0, this, SLOT( addGroup() ), ac, "AddGroup" );

	actionSendMessage = KopeteStdAction::sendMessage( this, SLOT( slotSendMessage() ), ac, "contactSendMessage" );
	actionStartChat = KopeteStdAction::chat( this, SLOT( slotStartChat() ), ac, "contactStartChat" );

	actionRemoveFromGroup = KopeteStdAction::deleteContact( this, SLOT( slotRemoveFromGroup() ), ac, "contactRemoveFromGroup" );
	actionRemoveFromGroup->setText( i18n("Remove From Group") );

	actionMove = KopeteStdAction::moveContact( this, SLOT( slotMoveToGroup() ), ac, "contactMove" );
	actionCopy = KopeteStdAction::copyContact( this, SLOT( slotCopyToGroup() ), ac, "contactCopy" );
	actionRemove = KopeteStdAction::deleteContact( this, SLOT( slotRemove() ), ac, "contactRemove" );
	actionRename = new KAction( i18n( "Rename" ), "filesaveas", 0, this, SLOT( slotRename() ), ac, "contactRename" );
	actionSendFile = KopeteStdAction::sendFile( this, SLOT( slotSendFile() ), ac, "contactSendFile" );

	actionAddContact = new KActionMenu( i18n( "&Add Contact" ), QString::fromLatin1( "bookmark_add" ), ac , "contactAddContact" );
	actionAddContact->popupMenu()->insertTitle( i18n("Select Account") );

	actionAddTemporaryContact = new KAction( i18n( "Add to Your Contact List" ), "bookmark_add", 0,
		this, SLOT( slotAddTemporaryContact() ), ac, "contactAddTemporaryContact" );

	connect( KopeteContactList::contactList(), SIGNAL( metaContactSelected( bool ) ), this, SLOT( slotMetaContactSelected( bool ) ) );

	QPtrList<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts();
	for( KopeteAccount *acc = accounts.first() ; acc ; acc = accounts.next() )
	{
		KAction *action = new KAction( acc->accountId(), acc->accountIcon(), 0, this, SLOT( slotAddContact() ), acc );
		actionAddContact->insert( action );
	}

	actionProperties = new KAction( i18n( "&Properties" ), "", Qt::Key_Alt + Qt::Key_Return,
		this, SLOT( slotProperties() ), ac, "contactProperties" );

	// Update enabled/disabled actions
	slotViewSelectionChanged();
}

KopeteContactListView::~KopeteContactListView()
{
	QToolTip::remove( viewport() );
	delete m_tooltip;

	delete d;
}

void KopeteContactListView::slotMetaContactAdded( KopeteMetaContact *mc )
{
	if ( KopetePrefs::prefs()->sortByGroup() || mc->isTemporary() )
	{
		if ( mc->isTopLevel() )
			m_metaContacts.append( new KopeteMetaContactLVI( mc, this ) );

		// Now we create group items
		KopeteGroupList list = mc->groups();
		for ( KopeteGroup *it = list.first(); it; it = list.next() )
		{
			KopeteGroupViewItem *groupLVI = getGroup( it );
			if ( groupLVI )
				m_metaContacts.append( new KopeteMetaContactLVI( mc, groupLVI ) );
		}
	}
	else
	{
		if ( mc->status() == KopeteOnlineStatus::Offline )
			m_metaContacts.append( new KopeteMetaContactLVI( mc, m_offlineItem ) );
		else
			m_metaContacts.append( new KopeteMetaContactLVI( mc, m_onlineItem ) );

		slotContactStatusChanged( 0L );
	}

	connect( mc, SIGNAL( addedToGroup( KopeteMetaContact *, KopeteGroup * ) ),
		SLOT( slotAddedToGroup( KopeteMetaContact *, KopeteGroup * ) ) );
	connect( mc, SIGNAL( removedFromGroup( KopeteMetaContact *, KopeteGroup * ) ),
		SLOT( slotRemovedFromGroup( KopeteMetaContact *, KopeteGroup * ) ) );
	connect( mc, SIGNAL( movedToGroup( KopeteMetaContact *, KopeteGroup *, KopeteGroup * ) ),
		SLOT( slotMovedToGroup( KopeteMetaContact *, KopeteGroup *, KopeteGroup * ) ) );
	connect( mc, SIGNAL( onlineStatusChanged( KopeteMetaContact *, KopeteOnlineStatus::OnlineStatus ) ),
		SLOT( slotContactStatusChanged( KopeteMetaContact * ) ) );
}

void KopeteContactListView::slotMetaContactDeleted( KopeteMetaContact *mc )
{
	removeContact( mc );
}

void KopeteContactListView::slotMetaContactSelected( bool sel )
{
	bool set = sel;

	if( sel )
	{
		KopeteMetaContact *kmc = KopeteContactList::contactList()->selectedMetaContacts().first();
		set = sel && kmc->isReachable();
		actionAddTemporaryContact->setEnabled( sel && kmc->isTemporary() );

		// TODO: make available for several contacts
		actionRemoveFromGroup->setEnabled( sel && (kmc->groups().count()>1 )  );
	}
	else
	{
		actionAddTemporaryContact->setEnabled(false);
		actionRemoveFromGroup->setEnabled(false);
	}

	actionSendMessage->setEnabled( set );
	actionStartChat->setEnabled( set );
	actionMove->setEnabled( sel ); // TODO: make available for several contacts
	actionCopy->setEnabled( sel ); // TODO: make available for several contacts
	actionAddContact->setEnabled( sel );
}

void KopeteContactListView::slotAddedToGroup( KopeteMetaContact *mc, KopeteGroup *to )
{
	if ( !KopetePrefs::prefs()->sortByGroup() && to->type() != KopeteGroup::Temporary )
		return;

	//check if the contact isn't already in the group
	KopeteMetaContactLVI *li = m_metaContacts.first();
	for( ; li; li = m_metaContacts.next() )
	{
		if ( li->metaContact()==mc && li->group()==to )
			return;
	}


	KopeteGroupViewItem *groupLVI = getGroup( to );
	if( groupLVI )
	{
		m_metaContacts.append( new KopeteMetaContactLVI( mc, groupLVI ) );
	}
	else
	{
		m_metaContacts.append( new KopeteMetaContactLVI( mc, this ) );
	}

	// If the group count is now 1, we didn't have _any_ group before, in
	// which case this is actually a move instead of an addition. Remove the
	// old toplevel item
	/*if ( ( mc->groups() ).count() == 1 )
		slotRemovedFromGroup( mc, QString::null );*/
}

void KopeteContactListView::slotRemovedFromGroup( KopeteMetaContact *mc, KopeteGroup *from )
{
	if ( !KopetePrefs::prefs()->sortByGroup() && from->type() != KopeteGroup::Temporary )
		return;

	KopeteGroupViewItem *group_item = getGroup( from , false );
	if( !group_item )
	{
		//from toplevel
		KopeteMetaContactLVI *li = m_metaContacts.first();
		for( ; li; li = m_metaContacts.next() )
		{
			if( li->isTopLevel() && li->metaContact()==mc )
			{
				delete li;
				m_metaContacts.remove( li );
				break;
			}
		}
		return;
	}

	if(mc->groups().isEmpty()  && KopetePrefs::prefs()->sortByGroup())
	{
		//check if there are already a top-level contact
		bool yes=false;
		for(KopeteMetaContactLVI *li = m_metaContacts.first() ; li; li = m_metaContacts.next() )
		{
			if( li->isTopLevel() && li->metaContact()==mc )
			{
				yes=true;
			}
		}
		//if no, move to toplevel
		if(!yes)
		{
			mc->isTopLevel(); //put the contact to toplevel
			slotMovedToGroup( mc, from, KopeteGroup::topLevel() );
			return;
		}
	}

	QListViewItem *lvi = group_item->firstChild();
	for( ; lvi; lvi = lvi->nextSibling() )
	{
		KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi );
		if ( !kc )
			continue;

		if ( kc->metaContact() == mc )
		{
			m_metaContacts.remove( kc );
			delete kc;
			break;
		}
	}

	group_item->refreshDisplayName();

	//delete temporary-group if it is empty
	if ( from->type() == KopeteGroup::Temporary )
	{
		KopeteGroupViewItem *m_temporaryGroup = getGroup( KopeteGroup::temporary(), false );
		if ( m_temporaryGroup->childCount() == 0 )
		{
			mGroups.remove( m_temporaryGroup );
			delete m_temporaryGroup;
			m_temporaryGroup = 0L;
		}
	}

	if(!KopetePrefs::prefs()->sortByGroup())
		slotContactStatusChanged(mc);
}

void KopeteContactListView::slotMovedToGroup( KopeteMetaContact *mc, KopeteGroup *from, KopeteGroup *to )
{
	if ( !KopetePrefs::prefs()->sortByGroup() )
	{
		if ( to->type() == KopeteGroup::Temporary )
			slotAddedToGroup( mc, to );
		else if ( from->type() == KopeteGroup::Temporary )
			slotRemovedFromGroup( mc, from );
		return;
	}

	KopeteGroupViewItem *g_to   = getGroup( to );
	KopeteGroupViewItem *g_from = getGroup( from );

	if( !g_from )
	{
		// The contact has no groups
		if( !g_to )
			return;

		KopeteMetaContactLVI *li = m_metaContacts.first();
		for( ; li; li = m_metaContacts.next() )
		{
			if ( li->isTopLevel()  && li->metaContact()==mc )
			{
				takeItem( li );
				g_to->insertItem( li );
				li->movedToGroup( to );
				break;
			}
		}
		return;
	}

	QListViewItem *lvi = g_from->firstChild();
	for( ; lvi; lvi = lvi->nextSibling() )
	{
		KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi );
		if( !kc )
			continue;

		if( kc->metaContact() == mc )
		{
			g_from->takeItem( lvi );
			if( g_to )
				g_to->insertItem( lvi );
			else
			{
				insertItem( lvi );
			}

			kc->movedToGroup( to );
			break;
		}
	}

	if ( from->type() == KopeteGroup::Temporary )
	{
		// Delete temporary group if it is empty
		KopeteGroupViewItem *m_temporaryGroup = getGroup( KopeteGroup::temporary(), false );
		if ( m_temporaryGroup && m_temporaryGroup->childCount() == 0 )
		{
			mGroups.remove( m_temporaryGroup );
			delete m_temporaryGroup;
			m_temporaryGroup = 0L;
		}
	}
}

void KopeteContactListView::removeContact( KopeteMetaContact *c )
{
	KopeteMetaContactLVI *li = m_metaContacts.first();
	while( (li=m_metaContacts.current()) )
	{
		if ( li->metaContact() == c )
		{
			m_metaContacts.setAutoDelete(false);
			m_metaContacts.remove( li );
			KopeteGroupViewItem *groupLVI=li->parentGroup();
			delete li;
			if(groupLVI) groupLVI->refreshDisplayName();
		}
		else
			m_metaContacts.next();
	}

	//delete temporary-group if it is empty
	KopeteGroupViewItem *m_temporaryGroup = getGroup( KopeteGroup::temporary(), false );
	if ( m_temporaryGroup )
	{
		if ( m_temporaryGroup->childCount() == 0 )
		{
			mGroups.remove( m_temporaryGroup );
			delete m_temporaryGroup;
			m_temporaryGroup = 0L;
		}
	}
}

KopeteGroupViewItem *KopeteContactListView::getGroup( KopeteGroup *Kgroup , bool add )
{
	if( !Kgroup )
		return 0L;

	if ( Kgroup->type() == KopeteGroup::TopLevel )
		return 0L;

	for( KopeteGroupViewItem *it = mGroups.first(); it; it = mGroups.next() )
	{
		if( it->group()==Kgroup )
			return it;
	}

	if(!add)
		return 0l;
	// If no group exists, create an item for it
	KopeteGroupViewItem *item = new KopeteGroupViewItem( Kgroup, this );
	mGroups.append( item );

	if ( Kgroup->isExpanded() )
	{
		item->setOpen( true );
		slotExpanded( item );
	}
	else
	{
		item->setOpen( false );
		slotCollapsed( item );
	}

	return item;
}

void KopeteContactListView::addGroup()
{
	QString groupName =
#if KDE_IS_VERSION( 3, 1, 90 )
		KInputDialog::getText( i18n( "New Group" ), i18n( "Please enter the name for the new group:" ) );
#else
		KLineEditDlg::getText( i18n( "New Group" ), i18n( "Please enter the name for the new group:" ) );
#endif

	if ( !groupName.isEmpty() )
		addGroup( groupName );
}

void KopeteContactListView::addGroup( const QString groupName )
{
	getGroup(KopeteContactList::contactList()->getGroup(groupName));
}

void KopeteContactListView::slotGroupAdded( KopeteGroup *group )
{
	if ( KopetePrefs::prefs()->sortByGroup() )
		getGroup( group );
}

void KopeteContactListView::slotExpanded( QListViewItem *item )
{
	KopeteGroupViewItem *groupLVI = dynamic_cast<KopeteGroupViewItem *>( item );
	if ( groupLVI )
	{
		groupLVI->group()->setExpanded( true );
		if ( groupLVI->group()->useCustomIcon() )
			groupLVI->updateCustomIcons( mShowAsTree );
		else
			item->setPixmap( 0, mShowAsTree ? open : classic );
	}
}

void KopeteContactListView::slotCollapsed( QListViewItem *item )
{
	KopeteGroupViewItem *groupLVI = dynamic_cast<KopeteGroupViewItem*>( item );
	if ( groupLVI )
	{
		groupLVI->group()->setExpanded( false );
		if ( groupLVI->group()->useCustomIcon() )
			groupLVI->updateCustomIcons( mShowAsTree );
		else
			item->setPixmap( 0, mShowAsTree ? closed : classic );
	}
}

void KopeteContactListView::slotDoubleClicked( QListViewItem *item )
{
	if ( !item )
		return;

	kdDebug( 14000 ) << k_funcinfo << endl;

	KopeteMetaContactLVI *metaItem = dynamic_cast<KopeteMetaContactLVI *>( item );
	if ( !metaItem || !mShowAsTree )
	{
		kdDebug( 14000 ) << k_funcinfo << "setOpen( item, " << !isOpen( item ) << " )" << endl;
		setOpen( item, !isOpen( item ) );
	}
}

void KopeteContactListView::slotContextMenu( KListView * /* listview */, QListViewItem *item, const QPoint &point )
{
	KopeteMetaContactLVI *metaLVI = dynamic_cast<KopeteMetaContactLVI *>( item );
	KopeteGroupViewItem  *groupvi = dynamic_cast<KopeteGroupViewItem *>( item );

	if ( item && !item->isSelected() )
	{
		clearSelection();
		item->setSelected( true );
	}

	if ( !item )
		clearSelection();

	int nb = KopeteContactList::contactList()->selectedMetaContacts().count() +
		KopeteContactList::contactList()->selectedGroups().count();

	KMainWindow *window = dynamic_cast<KMainWindow *>(topLevelWidget());
	if ( !window )
	{
		kdError( 14000 ) << k_funcinfo << "Main window not found, unable to display context-menu; "
			<< "Kopete::UI::Global::mainWidget() = " << Kopete::UI::Global::mainWidget() << endl;
		return;
	}

	if ( metaLVI && nb == 1 )
	{
		int px = mapFromGlobal( point ).x() - ( header()->sectionPos( header()->mapToIndex( 0 ) ) +
			treeStepSize() * ( item->depth() + ( rootIsDecorated() ? 1 : 0 ) ) + itemMargin() );
		int py = mapFromGlobal( point ).y() - itemRect( item ).y() - header()->height();

		//kdDebug( 14000 ) << k_funcinfo << "x: " << px << ", y: " << py << endl;
		KopeteContact *c = metaLVI->contactForPoint( QPoint( px, py ) ) ;
		if ( c )
		{
			KPopupMenu *p = c->popupMenu();
			connect( p, SIGNAL( aboutToHide() ), p, SLOT( deleteLater() ) );
			p->popup( point );
		}
		else
		{
			KPopupMenu *popup = dynamic_cast<KPopupMenu *>( window->factory()->container( "contact_popup", window ) );
			if ( popup )
			{
				QString title = i18n( "Translators: format: '<nickname> (<online status>)'", "%1 (%2)" ).
#if QT_VERSION < 0x030200
					arg( metaLVI->metaContact()->displayName() ).arg( metaLVI->metaContact()->statusString() );
#else
					arg( metaLVI->metaContact()->displayName() , metaLVI->metaContact()->statusString() );
#endif
				if ( title.length() > 43 )
					title = title.left( 40 ) + QString::fromLatin1( "..." );

				if ( popup->title( 0 ).isNull() )
					popup->insertTitle ( title, 0, 0 );
				else
					popup->changeTitle ( 0, title );

				// Submenus for separate contact actions
				bool sep = false;  //FIXME: find if there is already a separator in the end - Olivier
				QPtrList<KopeteContact> it = metaLVI->metaContact()->contacts();
				for( KopeteContact *c = it.first(); c; c = it.next() )
				{
					if( sep )
					{
						popup->insertSeparator();
						sep = false;
					}

					KPopupMenu *contactMenu = it.current()->popupMenu();
					connect( popup, SIGNAL( aboutToHide() ), contactMenu, SLOT( deleteLater() ) );
					QString text= i18n( "Translators: format: '<displayName> (<id>)'", "%2 <%1>" ).
#if QT_VERSION < 0x030200
						arg( c->contactId() ).arg( c->displayName() );
#else
						arg( c->contactId(), c->displayName() );
#endif
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
		KPopupMenu *popup = dynamic_cast<KPopupMenu *>( window->factory()->container( "group_popup", window ) );
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
		KPopupMenu *popup = dynamic_cast<KPopupMenu *>( window->factory()->container( "contactlistitems_popup", window ) );
		if ( popup )
			popup->popup( point );
	}
	else
	{
		KPopupMenu *popup = dynamic_cast<KPopupMenu *>( window->factory()->container( "contactlist_popup", window ) );
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
	KopeteGroupViewItem *gi;
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

	if ( KopetePrefs::prefs()->sortByGroup() )
	{
		bool cont;
		do
		{
			// slotAddedToGroup changes m_metaContacts.current(), hence the nested loop
			// FIXME: Instead of relying on current() and first/next we should use an Iterator instead - Martijn
			cont = false;
			for ( KopeteMetaContactLVI *li = m_metaContacts.first(); li; li = m_metaContacts.next() )
			{
				if ( !li->isGrouped() )
				{
					KopeteMetaContact *mc = li->metaContact();
					m_metaContacts.setAutoDelete( false );
					m_metaContacts.remove( li );
					delete li;

					if ( mc->isTopLevel() )
						slotAddedToGroup( mc, KopeteGroup::topLevel() );
					KopeteGroupList list = mc->groups();
					for ( KopeteGroup *it = list.first(); it; it = list.next() )
						slotAddedToGroup( mc, it );

					cont = true;
					break;
				}
			}
		} while ( cont );

		delete m_onlineItem;
		m_onlineItem = 0l;
		delete m_offlineItem;
		m_offlineItem = 0l;
	}
	else
	{
		m_metaContacts.setAutoDelete( false );
		mGroups.setAutoDelete( false );
		bool cont;
		do
		{
			// slotContactStatusChanged changes m_metaContacts.current()
			cont = false;
			for ( KopeteMetaContactLVI *li = m_metaContacts.first(); li ; li = m_metaContacts.next() )
			{
				if ( li->isGrouped() && !li->metaContact()->isTemporary())
				{
					KopeteMetaContact *mc = li->metaContact();
					m_metaContacts.remove( li );
					delete li;
					slotContactStatusChanged( mc );
					cont = true;
					break;
				}
			}
		} while ( cont );

		gi = mGroups.first();
		while ( ( gi = mGroups.current() ) != 0L )
		{
			if ( gi->group()->type() != KopeteGroup::Temporary )
			{
				mGroups.remove( gi );
				delete gi;
			}
			else
			{
				mGroups.next();
			}
		}
	}

	for ( gi = mGroups.first(); gi; gi = mGroups.next() )
	{
		if ( gi->group()->useCustomIcon() )
		{
			gi->updateCustomIcons( mShowAsTree );
		}
		else
		{
			if ( mShowAsTree )
				gi->setPixmap( 0, isOpen( gi ) ? open : closed );
			else
				gi->setPixmap( 0, classic );
		}
	}
	delete gi;
	update();
}

void KopeteContactListView::slotExecuted( QListViewItem *item, const QPoint &p, int /* col */ )
{
	KopeteMetaContactLVI *metaContactLVI = dynamic_cast<KopeteMetaContactLVI *>( item );

	QPoint pos = viewport()->mapFromGlobal( p );
	KopeteContact *c = 0L;
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

void KopeteContactListView::slotContactStatusChanged( KopeteMetaContact *mc )
{
	if(KopetePrefs::prefs()->sortByGroup())
		return;

	if(!m_offlineItem)
	{
		m_offlineItem=new KopeteStatusGroupViewItem( KopeteOnlineStatus::Offline, this );
		m_offlineItem->setOpen(true);
	}
	if(!m_onlineItem)
	{
		m_onlineItem = new KopeteStatusGroupViewItem( KopeteOnlineStatus::Online, this );
		m_onlineItem->setOpen(true);
	}
	if(mc)
	{
		//check if the contact is already in the correct "group"
		QListViewItem *g = mc->isOnline()? m_onlineItem : m_offlineItem;
		bool yes=false;
		QListViewItem *lvi = g->firstChild();
		for( ; lvi; lvi = lvi->nextSibling() )
		{
			KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi );
			if ( !kc )
				continue;
			if ( kc->metaContact() == mc )
			{
				yes=true;
				break;
			}
		}
		if(!yes)
		{
			m_metaContacts.append( new KopeteMetaContactLVI( mc, g ) );
		}

		//check if the contact is in the oposite "group"
		g= mc->isOnline()? m_offlineItem : m_onlineItem;
		lvi = g->firstChild();
		for( ; lvi; lvi = lvi->nextSibling() )
		{
			KopeteMetaContactLVI *kc = dynamic_cast<KopeteMetaContactLVI*>( lvi );
			if ( !kc )
				continue;
			if ( kc->metaContact() == mc )
			{
				m_metaContacts.remove(kc);
				delete kc;
				break;
			}
		}
	}

	m_onlineItem->setText(0,i18n("Online contacts (%1)").arg(m_onlineItem->childCount()));
	m_offlineItem->setText(0,i18n("Offline contacts (%1)").arg(m_offlineItem->childCount()));
}

void KopeteContactListView::slotDropped(QDropEvent *e, QListViewItem *, QListViewItem *after)
{
	if(!acceptDrag(e))
		return;

	QListViewItem *source=currentItem();
	KopeteMetaContactLVI *source_metaLVI=dynamic_cast<KopeteMetaContactLVI*>(source);
	KopeteMetaContactLVI *dest_metaLVI=dynamic_cast<KopeteMetaContactLVI*>(after);
	KopeteGroupViewItem *dest_groupLVI=dynamic_cast<KopeteGroupViewItem*>(after);
//	KopeteGroupViewItem *source_groupLVI=dynamic_cast<KopeteGroupViewItem*>(source);
	KopeteContact *source_contact=0L;

	if(source_metaLVI)
		source_contact = source_metaLVI->contactForPoint( m_startDragPos );

	if(source_metaLVI  && dest_groupLVI)
	{
		if(source_metaLVI->group() == dest_groupLVI->group())
			return;
		if(source_metaLVI->metaContact()->isTemporary())
		{
			int r=KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(), i18n( "<qt>Would you like to add this contact to your contact list?</qt>" ),
				i18n( "Kopete" ), KStdGuiItem::yes(),KStdGuiItem::no(),"addTemporaryWhenMoving" );

			if(r==KMessageBox::Yes)
				source_metaLVI->metaContact()->setTemporary( false, dest_groupLVI->group() );
		}
		else
		{
			source_metaLVI->metaContact()->moveToGroup(source_metaLVI->group() , dest_groupLVI->group() );
		}
	}
	else if(source_metaLVI  && !dest_metaLVI && !dest_groupLVI)
	{
		if ( source_metaLVI->group()->type() == KopeteGroup::TopLevel )
			return;

		if(source_metaLVI->metaContact()->isTemporary())
		{
			int r=KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(), i18n( "<qt>Would you like to add this contact to your contact list?</qt>" ),
				i18n( "Kopete" ), KStdGuiItem::yes(),KStdGuiItem::no(),"addTemporaryWhenMoving" );

			if ( r == KMessageBox::Yes )
				source_metaLVI->metaContact()->setTemporary( false, KopeteGroup::topLevel() );
		}
		else
		{
//			kdDebug(14000) << "KopeteContactListView::slotDropped : moving the meta contact " << source_metaLVI->metaContact()->displayName()			<< " to top-level " <<	endl;
			source_metaLVI->metaContact()->moveToGroup( source_metaLVI->group(), KopeteGroup::topLevel() );
		}
	}
	else if(source_contact && dest_metaLVI) //we are moving a contact to another metacontact
	{
		if(source_metaLVI->metaContact()->isTemporary())
		{
			/*int r=KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(), i18n( "<qt>Would you like to add this contact to your contact list</qt>" ),
				i18n( "Kopete" ), KStdGuiItem::yes(),KStdGuiItem::no(),"addTemporaryWhenMoving" );
			if(r==KMessageBox::Yes)
				TODO*/
		}
		else
		{
//			kdDebug(14000) << "KopeteContactListView::slotDropped : moving the contact " << source_contact->contactId()	<< " to metacontact " << dest_metaLVI->metaContact()->displayName() <<	endl;
			source_contact->setMetaContact(dest_metaLVI->metaContact());
		}
	}

/*	else if(source_groupLVI && dest_groupLVI)
	{
		if(source_groupLVI->group()->parentGroup()  == dest_groupLVI->group() )
			return;
		source_groupLVI->group()->setParentGroup( dest_metaLVI->group() );
	}
 	else if(source_groupLVI && !dest_groupLVI && dest_metaLVI)
	{
		if(source_groupLVI->group()->parentGroup() == KopeteGroup::toplevel)
			return;
		source_groupLVI->group()->setParentGroup( KopeteGroup::toplevel );
	}*/
	else if( e->provides( "text/uri-list" ) )
	{
		if ( !QUriDrag::canDecode( e ) )
		{
			e->ignore();
			return;
		}

		KURL::List urlList;
		KURLDrag::decode( e, urlList );

		QPoint p=contentsToViewport(e->pos());
		int px = p.x() - ( header()->sectionPos( header()->mapToIndex( 0 ) ) +
			treeStepSize() * ( dest_metaLVI->depth() + ( rootIsDecorated() ? 1 : 0 ) ) + itemMargin() );
		int py = p.y() - itemRect( dest_metaLVI ).y();
		KopeteContact *c = dest_metaLVI->contactForPoint( QPoint( px, py ) ) ;

		for ( KURL::List::Iterator it = urlList.begin(); it != urlList.end(); ++it )
		{
			if( (*it).isLocalFile() )
			{ //send a file
				if(c)
					c->sendFile( *it );
				else
					dest_metaLVI->metaContact()->sendFile( *it );
			}
			else
			{ //this is a URL, send the URL in a message
				if(!c)
					c = dest_metaLVI->metaContact()->execute(); //We need to know which contact was chosen as the preferred in order to message it
				if (!c) return;

				KopeteMessage msg(c->account()->myself(), c, (*it).url() , KopeteMessage::Outbound);
				c->manager(true)->sendMessage(msg);
			}
		}
		e->acceptAction();
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
		KopeteContact *source_contact=0L;

		if(source_metaLVI)
			source_contact = source_metaLVI->contactForPoint( m_startDragPos );

		if( source_metaLVI && dest_groupLVI && !source_contact) //we are moving a metacontact to another group
		{
			if(source_metaLVI->group() == dest_groupLVI->group())
				return false;
			if ( dest_groupLVI->group()->type() == KopeteGroup::Temporary )
				return false;
	//		if(source_metaLVI->metaContact()->isTemporary())
	//			return false;
			return true;
		}
		else if(source_metaLVI  && !dest_metaLVI && !dest_groupLVI && !source_contact) //we are moving a metacontact to toplevel
		{
			if ( source_metaLVI->group()->type() == KopeteGroup::TopLevel )
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
			if(source_metaLVI->metaContact()->isTemporary())
				return false;	//TODO: allow to move a temporary contact dirrecvtly in a metacontact
			return true;
		}
/*		else if(source_groupLVI && dest_groupLVI) //we are moving a group to another group
		{
			if(dest_groupLVI->group() == KopeteGroup::temporary)
				return false;
			if(source_groupLVI->group() == KopeteGroup::temporary)
				return false;
			if(source_groupLVI->group()->parentGroup()  == dest_groupLVI->group() )
				return false;
			KopeteGroup *g=dest_groupLVI->group()->parentGroup();
			while(g && g != KopeteGroup::toplevel)
			{
				if(g==source_groupLVI->group())
					return false;
				g=g->parentGroup();
			}
			return true;
		}
		else if(source_groupLVI && !dest_groupLVI && dest_metaLVI) //we are moving a group to toplevel
		{
			if(source_groupLVI->group() == KopeteGroup::temporary)
				return false;
			if(source_groupLVI->group()->parentGroup() == KopeteGroup::toplevel)
				return false;
			return true;
		}*/
	}
	else
	{
		if ( e->provides( "text/uri-list" )  && dest_metaLVI && dest_metaLVI->metaContact()->canAcceptFiles() )
		{ //we are sending a file
			QPoint p=contentsToViewport(e->pos());
			int px = p.x() - ( header()->sectionPos( header()->mapToIndex( 0 ) ) +
				treeStepSize() * ( dest_metaLVI->depth() + ( rootIsDecorated() ? 1 : 0 ) ) + itemMargin() );
			int py = p.y() - itemRect( dest_metaLVI ).y();
			KopeteContact *c = dest_metaLVI->contactForPoint( QPoint( px, py ) ) ;

			if( c ? !c->isReachable() : !dest_metaLVI->metaContact()->isReachable() )
				return false; //If the pointed contact is not reachable, abort

			if( c ? c->canAcceptFiles() : dest_metaLVI->metaContact()->canAcceptFiles()  )
			{ //If the pointed contact, or the metacontact if no contact are pointed can accept file, return true in everycase
				return true;
			}
			else
			{
				if ( !QUriDrag::canDecode( e ) )
					return false;
				KURL::List urlList;
				KURLDrag::decode( e, urlList );

				for ( KURL::List::Iterator it = urlList.begin(); it != urlList.end(); ++it )
				{
					if( (*it).isLocalFile() )
						return false; //we can't send links if a locale file is in link
				}
				return true; //we will send a link
			}
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

void KopeteContactListView::findDrop(const QPoint &pos, QListViewItem *&parent, QListViewItem *&after)
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

/* This is a small hack ensuring that only F2 triggers inline
 * renameing. Won't win a beauty award, but I think relying on
 * the fact that QListView intercepts and processes the F2 event
 * through this event filter is sorta safe.
 *
 * Also use enter to start a chat since executed is not called
 * when enter
 */
void KopeteContactListView::keyPressEvent( QKeyEvent *e )
{
	if ( (e->key() == Qt::Key_F2) && currentItem() )
	{
		slotRename();
	}
	else if ( (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) && currentItem())
	{
		slotExecuted(currentItem() , QPoint() , 0);
	}
	else
		KListView::keyPressEvent(e);
}

void KopeteContactListView::slotNewMessageEvent(KopeteEvent *event)
{
	KopeteMessage msg=event->message();
	//only for signle chat
	if(msg.from() && msg.to().count()==1)
	{
		KopeteMetaContact *m=msg.from()->metaContact();
		if(!m)
			return;

		for(KopeteMetaContactLVI *li = m_metaContacts.first(); li; li = m_metaContacts.next() )
		{
			if ( li->metaContact() == m )
				li->catchEvent(event);
		}
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
	KopeteContact *c = metaLVI->contactForPoint( m_startDragPos );
        KMultipleDrag *drag = new KMultipleDrag( this );
	drag->addDragObject( new QStoredDrag("application/x-qlistviewitem", 0L ) );

	if ( c ) 	// dragging a contact
		pm = c->onlineStatus().iconFor( c, 12 ); // FIXME: fixed icon scaling
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
		drag->addDragObject( new QTextDrag( address.fullEmail(), this ) );
		KABC::VCardConverter converter;
		QString vcard;

		if( converter.addresseeToVCard( address, vcard ) )
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
	QPtrList<KopeteMetaContact> contacts;
	QPtrList<KopeteGroup> groups;

	QListViewItemIterator it( this );
	while ( it.current() )
	{
		QListViewItem *item = it.current();
		++it;

		if ( item->isSelected() )
		{
			KopeteMetaContactLVI *metaLVI=dynamic_cast<KopeteMetaContactLVI*>(item);
			if(metaLVI)
				contacts.append( metaLVI->metaContact());
			KopeteGroupViewItem *groupLVI=dynamic_cast<KopeteGroupViewItem*>(item);
			if(groupLVI)
				groups.append( groupLVI->group());
		}
	}

	// will cause slotListSelectionChanged to be called to update our actions.
	KopeteContactList::contactList()->setSelectedItems(contacts , groups);
}

void KopeteContactListView::slotListSelectionChanged()
{
	QPtrList<KopeteMetaContact> contacts = KopeteContactList::contactList()->selectedMetaContacts();
	QPtrList<KopeteGroup> groups = KopeteContactList::contactList()->selectedGroups();

	//TODO: update the list to select the items that should be selected.
	// make sure slotViewSelectionChanged is *not* called.

	updateActionsForSelection( contacts, groups );
}

void KopeteContactListView::updateActionsForSelection( QPtrList<KopeteMetaContact> contacts, QPtrList<KopeteGroup> groups )
{
	actionSendFile->setEnabled( groups.isEmpty() && contacts.count()==1 && contacts.first()->canAcceptFiles());
	actionAddContact->setEnabled( groups.isEmpty() && contacts.count()==1 && !contacts.first()->isTemporary());

	if(contacts.count() == 1 && groups.isEmpty())
	{
		actionRename->setText(i18n("Rename Contact"));
		actionRemove->setText(i18n("Remove Contact"));
		actionRename->setEnabled(true);
		actionRemove->setEnabled(true);
	}
	else if(groups.count() == 1 && contacts.isEmpty())
	{
		actionRename->setText(i18n("Rename Group"));
		actionRemove->setText(i18n("Remove Group"));
		actionRename->setEnabled(true);
		actionRemove->setEnabled(true);
	}
	else
	{
		actionRename->setText(i18n("Rename"));
		actionRemove->setText(i18n("Remove"));
		actionRename->setEnabled(false);
		actionRemove->setEnabled(contacts.count()+groups.count());
	}

	actionMove->setCurrentItem( -1 );
	actionCopy->setCurrentItem( -1 );

	actionProperties->setEnabled( ( groups.count() + contacts.count() ) == 1 );
}

void KopeteContactListView::slotSendMessage()
{
	KopeteMetaContact *m=KopeteContactList::contactList()->selectedMetaContacts().first();
	if(m)
		m->sendMessage();
}

void KopeteContactListView::slotStartChat()
{
	KopeteMetaContact *m=KopeteContactList::contactList()->selectedMetaContacts().first();
	if(m)
		m->startChat();
}

void KopeteContactListView::slotSendFile()
{
	KopeteMetaContact *m=KopeteContactList::contactList()->selectedMetaContacts().first();
	if(m)
		m->sendFile(KURL());
}

void KopeteContactListView::slotMoveToGroup()
{
	KopeteMetaContactLVI *metaLVI=dynamic_cast<KopeteMetaContactLVI*>(currentItem());
	if(!metaLVI)
		return;
	KopeteMetaContact *m=metaLVI->metaContact();
	KopeteGroup *g=metaLVI->group();

	//FIXME! what ios two groups have the same name?
	KopeteGroup *to = actionMove->currentItem() ? KopeteContactList::contactList()->getGroup( actionMove->currentText() ) : KopeteGroup::topLevel();

	if( !to || to->type() == KopeteGroup::Temporary )
		return;


	if(m->isTemporary())
	{
		if( KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(), i18n( "<qt>Would you like to add this contact to your contact list?</qt>" ),
				i18n( "Kopete" ), KStdGuiItem::yes(),KStdGuiItem::no(),"addTemporaryWhenMoving" ) == KMessageBox::Yes )
		{
			m->setTemporary(false,g);
			m->moveToGroup( KopeteGroup::topLevel(), to );
		}
	}
	else if( !m->groups().contains( to ) )
		m->moveToGroup( g, to );

	actionMove->setCurrentItem( -1 );
}

void KopeteContactListView::slotCopyToGroup()
{
	KopeteMetaContact *m =
		KopeteContactList::contactList()->selectedMetaContacts().first();

	if(!m)
		return;

	//FIXME! what if two groups have the same name?
	KopeteGroup *to = actionCopy->currentItem() ?
		KopeteContactList::contactList()->getGroup( actionCopy->currentText() ) :
		KopeteGroup::topLevel();

	if( !to || to->type() == KopeteGroup::Temporary )
		return;

	if( m->isTemporary() )
		return;

	if( !m->groups().contains( to ) )
		m->addToGroup( to );

	actionCopy->setCurrentItem( -1 );
}

void KopeteContactListView::slotRemoveFromGroup()
{
	KopeteMetaContactLVI *metaLVI=dynamic_cast<KopeteMetaContactLVI*>(currentItem());
	if(!metaLVI)
		return;
	KopeteMetaContact *m=metaLVI->metaContact();

	if(m->isTemporary())
		return;

	m->removeFromGroup( metaLVI->group() );
}

void KopeteContactListView::slotRemove()
{
	QPtrList<KopeteMetaContact> contacts=KopeteContactList::contactList()->selectedMetaContacts();
	QPtrList<KopeteGroup> groups=KopeteContactList::contactList()->selectedGroups();

	if(groups.count() + contacts.count() <= 0)
		return;

	QStringList items;
	for( KopeteGroup *it = groups.first(); it; it = groups.next() )
	{
		if(!it->displayName().isEmpty())
			items.append( it->displayName() );
	}
	for( KopeteMetaContact *it = contacts.first(); it; it = contacts.next() )
	{
		if(!it->displayName().isEmpty())
			items.append( it->displayName() );
	}


	if( items.count() <= 1 )
	{  //we are deleting an empty contact
		QString msg;
		if( !contacts.isEmpty() )
		{
			msg = i18n( "<qt>Are you sure you want to remove the contact <b>%1</b> from your contact list?</qt>" ).arg( contacts.first()->displayName() ) ;
		}
		else if( !groups.isEmpty() )
		{
			msg = i18n( "<qt>Are you sure you want to remove the group <b>%1</b> and all contacts that are contained within it?</qt>" ).
				arg( groups.first()->displayName() );
		}
		else
			return; // this should never happen

		if( KMessageBox::warningYesNo( this, msg, i18n( "Remove" ) ) !=
			KMessageBox::Yes )
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

		if( KMessageBox::questionYesNoList( this, msg, items, i18n("Remove"),
			KStdGuiItem::yes(), KStdGuiItem::no(), "askRemovingContactOrGroup" )
			!= KMessageBox::Yes )
		{
			 	return;
		}
	}

	for( KopeteMetaContact *it = contacts.first(); it; it = contacts.next() )
	{
		KopeteContactList::contactList()->removeMetaContact( it );
	}

	for( KopeteGroup *it = groups.first(); it; it = groups.next() )
	{
		KopeteGroupViewItem* removeGroupItem = getGroup( it, false );
		QListViewItem *lvi, *lvi2;
		for( lvi = removeGroupItem->firstChild(); lvi; lvi = lvi2 )
		{
			lvi2 = lvi->nextSibling();
			KopeteMetaContactLVI *kc =
				dynamic_cast<KopeteMetaContactLVI*>( lvi );
			if(kc)
			{
				KopeteMetaContact *mc= kc->metaContact() ;
				if(mc)
				{
					if(mc->groups().count()==1 && !mc->isTopLevel())
						KopeteContactList::contactList()->removeMetaContact(mc);
					else
						mc->removeFromGroup(it);
				}
			}
		}

		if( removeGroupItem->childCount() >= 1 )
		{
			kdDebug(14000) << "KopeteContactListView::slotRemove(): "
				<< "all subMetaContacts are not removed... Aborting" << endl;
			continue;
		}

		mGroups.remove( removeGroupItem );
		KopeteContactList::contactList()->removeGroup(
			removeGroupItem->group() );
	}
}

void KopeteContactListView::slotRename()
{
	if ( KopeteMetaContactLVI *metaLVI = dynamic_cast<KopeteMetaContactLVI *>( currentItem() ) )
	{
		metaLVI->slotRename();
	}
	else if ( KopeteGroupViewItem *groupLVI = dynamic_cast<KopeteGroupViewItem *>( currentItem() ) )
	{
		if ( !KopetePrefs::prefs()->sortByGroup() )
			return;

		groupLVI->startRename( 0 );
	}
}

void KopeteContactListView::slotAddContact()
{
	if( !sender() )
		return;

	KopeteMetaContact *metacontact =
		KopeteContactList::contactList()->selectedMetaContacts().first();
	KopeteAccount *account = dynamic_cast<KopeteAccount*>( sender()->parent() );

	if( account && metacontact )
	{
		if ( metacontact->isTemporary() )
			return;

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
					addContactPage->apply( account, metacontact );
			}
		}
		addDialog->deleteLater();
	}
}

void KopeteContactListView::slotAddTemporaryContact()
{
	KopeteMetaContact *metacontact =
		KopeteContactList::contactList()->selectedMetaContacts().first();
	if( metacontact )
	{
/*		int r=KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
			i18n( "<qt>Would you like to add this contact to your contact list?</qt>" ),
			i18n( "Kopete" ), KStdGuiItem::yes(), KStdGuiItem::no(),
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

		if( groupLVI->group()->useCustomIcon() )
		{
			groupLVI->updateCustomIcons( mShowAsTree );
		}
		else
		{
			if (mShowAsTree)
				groupLVI->setPixmap( 0, isOpen(groupLVI) ? open : closed );
			else
				groupLVI->setPixmap( 0, classic );
		}
	}
}

void KopeteContactListView::delayedSort()
{
	if ( !d->sortTimer->isActive() )
		d->sortTimer->start( 500, true );
}

void KopeteContactListView::slotSort()
{
	//kdDebug( 14000 ) << k_funcinfo << endl;
	sort();
}

void KopeteContactListView::slotItemRenamed( QListViewItem *item )
{
	KopeteMetaContactLVI *metaLVI = dynamic_cast<KopeteMetaContactLVI *>( item );
	if ( metaLVI )
		metaLVI->metaContact()->setDisplayName( metaLVI->text( 0 ) );
	else
		kdWarning( 14000 ) << k_funcinfo << "Unknown list view item '" << item << "' renamed, ignoring item" << endl;
}

#include "kopetecontactlistview.moc"

// vim: set noet ts=4 sts=4 sw=4:

