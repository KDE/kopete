/*
    kopetecontactlistview.cpp

    Kopete Contactlist GUI

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Nick Betcher           <nbetcher@usinternet.com>
    Copyright (c) 2002      by Stefan Gehn            <sgehn@gmx.net>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

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
#include "kopetecontactlist.h"

#include <qcursor.h>
#include <qheader.h>
#include <qstylesheet.h>
#include <qdragobject.h>
#include <kurldrag.h>

#include <kapplication.h>
#include <kmainwindow.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kaction.h>

#include "addcontactwizard.h"
#include "addcontactpage.h"
#include "kopeteevent.h"
#include "kopetegroupviewitem.h"
#include "kopetemetacontactlvi.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopeteprefs.h"
#include "kopetestatusgroupviewitem.h"
#include "kopeteviewmanager.h"
#include "kopetestdaction.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopeteprotocol.h"

#if QT_VERSION < 0x030100
#include <qtooltip.h>
#endif

KopeteContactListView::KopeteContactListView( QWidget *parent,
	const char *name )
: KListView( parent, name )
{
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

	if(KopetePrefs::prefs()->sortByGroup())
	{
		m_offlineItem=0L;
		m_onlineItem=0L;
	}
	else
	{
		m_offlineItem = new KopeteStatusGroupViewItem( KopeteOnlineStatus::Offline, this );
		m_onlineItem = new KopeteStatusGroupViewItem( KopeteOnlineStatus::Online, this );
		m_onlineItem->setOpen(true);
		m_offlineItem->setOpen(true);
	}

	setAlternateBackground( QColor() ); // no alternate color, looks ugly
	setFullWidth( true );

	connect( this,
		SIGNAL( contextMenu( KListView*, QListViewItem *, const QPoint &) ),
		SLOT( slotContextMenu(KListView*, QListViewItem *, const QPoint & ) ) );
	connect( this, SIGNAL( expanded( QListViewItem * ) ),
		SLOT( slotExpanded( QListViewItem * ) ) );
	connect( this, SIGNAL( collapsed( QListViewItem * ) ),
		SLOT( slotCollapsed( QListViewItem * ) ) );
	connect( KopetePrefs::prefs(), SIGNAL( saved() ),
		SLOT( slotSettingsChanged() ) );
	connect( this, SIGNAL( executed( QListViewItem *, const QPoint &, int ) ),
		SLOT( slotExecuted( QListViewItem *, const QPoint &, int ) ) );
	connect( this, SIGNAL( doubleClicked( QListViewItem * ) ),
		SLOT( slotDoubleClicked( QListViewItem * ) ) );
	connect( this, SIGNAL( selectionChanged() ),
		SLOT( slotSelectionChanged() ) );

	connect( KopeteContactList::contactList(),
		SIGNAL( metaContactAdded( KopeteMetaContact * ) ),
		SLOT( slotMetaContactAdded( KopeteMetaContact * ) ) );

	connect( KopeteContactList::contactList(),
		SIGNAL( metaContactDeleted( KopeteMetaContact * ) ),
		SLOT( slotMetaContactDeleted( KopeteMetaContact * ) ) );

	connect( KopeteContactList::contactList(),
		SIGNAL( groupAdded( KopeteGroup * ) ),
		SLOT( slotGroupAdded( KopeteGroup * ) ) );

	connect( KopeteViewManager::viewManager() , SIGNAL( newMessageEvent(KopeteEvent *) ),
		this, SLOT( slotNewMessageEvent(KopeteEvent *) ) );



	connect( this , SIGNAL ( dropped(QDropEvent *, QListViewItem *, QListViewItem*) ) ,
		this, SLOT ( slotDropped(QDropEvent *, QListViewItem *, QListViewItem*) ));

//	connect( this , SIGNAL ( onItem(QListViewItem *) ) ,
//		this, SLOT (slotOnItem(QListViewItem *) ));

	addColumn( i18n("Contacts") , 0 );

	closed  = SmallIcon( "folder_green" );
	open    = SmallIcon( "folder_green_open" );
	classic = SmallIcon( "folder_blue" );

	// Add the already existing groups now
	QPtrListIterator<KopeteGroup> groupIt( KopeteContactList::contactList()->groups() );
	KopeteGroup *group;
	while( ( group = groupIt.current() ) != 0 )
	{
		getGroup( group, true );
		++groupIt;
	}

	// Add the already existing meta contacts now
	QPtrList<KopeteMetaContact> metaContacts =
		KopeteContactList::contactList()->metaContacts();
	QPtrListIterator<KopeteMetaContact> it( metaContacts );
	KopeteMetaContact *mc;
	while( ( mc = it.current() ) != 0 )
	{
		slotMetaContactAdded( mc );
		++it;
	}

	setDragEnabled(true);
	setItemsMovable(false);
	setAcceptDrops(true);
	setDropVisualizer(false);
	setDropHighlighter(true);
	setAutoOpen(true);
	setSelectionMode(QListView::Extended);


//	setTooltipColumn(0);
}

void KopeteContactListView::initActions(KActionCollection* ac)
{
//	KAction *actionNewGroup=
	new KAction( i18n("Create New Group") , 0,0,this, SLOT( addGroup() ) , ac , "AddGroup"  );

	KAction *actionSendMessage = KopeteStdAction::sendMessage( this, SLOT( slotSendMessage() ), ac, "contactSendMessage" );
	KAction *actionStartChat =  KopeteStdAction::chat( this, SLOT( slotStartChat() ), ac, "contactStartChat" );

	actionRemoveFromGroup = KopeteStdAction::deleteContact( this, SLOT( slotRemoveFromGroup() ), ac, "contactRemoveFromGroup" );
	actionRemoveFromGroup->setText( i18n("Remove From Group") );

	actionMove = KopeteStdAction::moveContact( this, SLOT( slotMoveToGroup() ), ac, "contactMove" );
	actionCopy = KopeteStdAction::copyContact( this, SLOT( slotCopyToGroup() ), ac, "contactCopy" );
	actionRemove = KopeteStdAction::deleteContact( this, SLOT( slotRemove() ), ac, "contactRemove" );
	actionRename = new KAction( i18n( "Rename" ), "filesaveas", 0, this, SLOT( slotRename() ), ac, "contactRename" );
	actionSendFile = KopeteStdAction::sendFile( this, SLOT( slotSendFile() ), ac, "contactSendFile" );

	actionAddContact= new KActionMenu( i18n( "&Add Contact" ), QString::fromLatin1( "bookmark_add" ), ac , "contactAddContact" );
	actionAddContact->popupMenu()->insertTitle(i18n("Select Account"));


	connect(KopeteContactList::contactList() , SIGNAL(metaContactSelected(bool)) , actionSendMessage , SLOT(setEnabled(bool)));
	connect(KopeteContactList::contactList() , SIGNAL(metaContactSelected(bool)) , actionStartChat   , SLOT(setEnabled(bool)));
	connect(KopeteContactList::contactList() , SIGNAL(metaContactSelected(bool)) , actionMove        , SLOT(setEnabled(bool))); //TODO: make avaliable for several contacts
	connect(KopeteContactList::contactList() , SIGNAL(metaContactSelected(bool)) , actionCopy        , SLOT(setEnabled(bool))); //TODO: make avaliable for several contacts
	connect(KopeteContactList::contactList() , SIGNAL(metaContactSelected(bool)),actionRemoveFromGroup,SLOT(setEnabled(bool))); //TODO: make avaliable for several contacts, and unavaliable when the contact is only in one group
	connect(KopeteContactList::contactList() , SIGNAL(metaContactSelected(bool)) , actionAddContact  , SLOT(setEnabled(bool)));



	QPtrList<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts();
	for( KopeteAccount *a = accounts.first() ; a ; a = accounts.next() )
	{
		KAction *aa=new KAction( a->accountId() , a->protocol()->pluginIcon() , 0 , this, SLOT( slotAddContact() ) , a);
		actionAddContact->insert(aa);
		//m_addContactActions.insert(aa,a);
	}


	//TODO
	actionAddTemporaryContact = new KAction( i18n( "Add to Your Contact List" ), "bookmark_add", 0, this, SLOT( slotAddTemoraryContact() ), ac, "actionAddTemporaryContact" );

	//update enabled/disabled actions
	slotSelectionChanged();
}



KopeteContactListView::~KopeteContactListView()
{
}

void KopeteContactListView::slotMetaContactAdded( KopeteMetaContact *mc )
{
	if(KopetePrefs::prefs()->sortByGroup() || mc->isTemporary())
	{
	   // if the contact is a toplevel
		if( mc->isTopLevel() )
		{
			m_metaContacts.append( new KopeteMetaContactLVI( mc, this ) );
		}
		// now we create group items
		KopeteGroupList list=mc->groups();
		for(KopeteGroup *it=list.first() ; it; it=list.next() )
		{
			KopeteGroupViewItem *groupLVI = getGroup( it );
			if( groupLVI )
			{
				m_metaContacts.append( new KopeteMetaContactLVI( mc, groupLVI ) );
			}
		}
	}
	else
	{
		if(mc->status() == KopeteOnlineStatus::Offline )
			m_metaContacts.append( new KopeteMetaContactLVI( mc, m_offlineItem ) );
		else
			m_metaContacts.append( new KopeteMetaContactLVI( mc, m_onlineItem ) );
		slotContactStatusChanged(0L);
	}

	connect( mc,
		SIGNAL( addedToGroup( KopeteMetaContact *, KopeteGroup * ) ),
		SLOT( slotAddedToGroup( KopeteMetaContact *, KopeteGroup * ) ) );
	connect( mc,
		SIGNAL( removedFromGroup( KopeteMetaContact *, KopeteGroup * ) ),
		SLOT( slotRemovedFromGroup( KopeteMetaContact *, KopeteGroup * ) ) );
	connect( mc,
		SIGNAL( movedToGroup( KopeteMetaContact *, KopeteGroup *, KopeteGroup * ) ),
		SLOT( slotMovedToGroup( KopeteMetaContact *, KopeteGroup *, KopeteGroup * ) ) );

	connect( mc,
		SIGNAL( onlineStatusChanged( KopeteMetaContact *, KopeteOnlineStatus::OnlineStatus ) ),
		SLOT( slotContactStatusChanged( KopeteMetaContact * ) ) );
}

void KopeteContactListView::slotMetaContactDeleted( KopeteMetaContact *mc )
{
	removeContact( mc );
}

void KopeteContactListView::slotAddedToGroup( KopeteMetaContact *mc, KopeteGroup *to )
{
	if(!KopetePrefs::prefs()->sortByGroup() && to!=KopeteGroup::temporary)
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
	if(!KopetePrefs::prefs()->sortByGroup() && from!=KopeteGroup::temporary)
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
			slotMovedToGroup( mc, from, KopeteGroup::toplevel );
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
	if(from == KopeteGroup::temporary)
	{
		KopeteGroupViewItem *m_temporaryGroup=getGroup(KopeteGroup::temporary, false);
		if(m_temporaryGroup->childCount() ==0)
		{
			mGroups.remove(m_temporaryGroup);
			delete m_temporaryGroup;
			m_temporaryGroup=0l;
		}
	}

	if(!KopetePrefs::prefs()->sortByGroup())
		slotContactStatusChanged(mc);
}

void KopeteContactListView::slotMovedToGroup( KopeteMetaContact *mc, KopeteGroup *from, KopeteGroup *to )
{
	if(!KopetePrefs::prefs()->sortByGroup())
	{
		if(to==KopeteGroup::temporary)
			slotAddedToGroup( mc, to );
		else if(from==KopeteGroup::temporary)
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

	if(from==KopeteGroup::temporary)
	{	//delete temporary-group if it is empty
		KopeteGroupViewItem *m_temporaryGroup=getGroup(KopeteGroup::temporary, false);
		if(m_temporaryGroup && m_temporaryGroup->childCount() ==0)
		{
			mGroups.remove(m_temporaryGroup);
			delete m_temporaryGroup;
			m_temporaryGroup=0l;
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
	KopeteGroupViewItem *m_temporaryGroup=getGroup(KopeteGroup::temporary, false);
	if(m_temporaryGroup)
	{
		if(m_temporaryGroup->childCount() ==0)
		{
			mGroups.remove(m_temporaryGroup);
			delete m_temporaryGroup;
			m_temporaryGroup=0l;
		}
	}
}

KopeteGroupViewItem *KopeteContactListView::getGroup( KopeteGroup *Kgroup , bool add )
{
	if( !Kgroup )
		return 0L;

	if( Kgroup == KopeteGroup::toplevel)
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

	if( Kgroup->expanded() )
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
	bool ok;
	QString groupName = KLineEditDlg::getText(
		i18n( "New Group - Kopete" ),
		i18n( "Please enter the name for the new group:" ),
		QString::null, &ok );
	if( ok )
		addGroup( groupName );
}

void KopeteContactListView::addGroup( const QString groupName )
{
	getGroup(KopeteContactList::contactList()->getGroup(groupName));
}

void KopeteContactListView::slotGroupAdded(KopeteGroup *g)
{
	if(KopetePrefs::prefs()->sortByGroup())
		getGroup(g);
}

void KopeteContactListView::slotExpanded( QListViewItem *item )
{
	if( dynamic_cast<KopeteMetaContactLVI*>( item ) )
		return;

	if ( mShowAsTree )
		item->setPixmap( 0, open );
	else
		item->setPixmap( 0, classic );

	KopeteGroupViewItem *groupLVI = dynamic_cast<KopeteGroupViewItem*>( item );
	if( groupLVI )
		groupLVI->group()->setExpanded( true );
}

void KopeteContactListView::slotDoubleClicked( QListViewItem *item )
{
	if ( item == NULL )
		return;

	KopeteMetaContactLVI *metaItem = dynamic_cast<KopeteMetaContactLVI*>(item);
	if ( !metaItem || !mShowAsTree )
	{
		setOpen( item, !isOpen( item ) );
	}
}

void KopeteContactListView::slotCollapsed(QListViewItem *item)
{
	if( dynamic_cast<KopeteMetaContactLVI*>( item ) )
		return;

	if ( mShowAsTree )
		item->setPixmap( 0, closed );
	else
		item->setPixmap( 0, classic );

	KopeteGroupViewItem *groupLVI = dynamic_cast<KopeteGroupViewItem*>( item );
	if( groupLVI )
		groupLVI->group()->setExpanded( false );
}

void KopeteContactListView::slotContextMenu( KListView*, QListViewItem *item,
	const QPoint &point )
{
	KopeteMetaContactLVI *metaLVI =
		dynamic_cast<KopeteMetaContactLVI*>( item );
	KopeteGroupViewItem *groupvi =
		dynamic_cast<KopeteGroupViewItem*>( item );

	if(item && !item->isSelected())
	{
		clearSelection();
		item->setSelected(true);
	}
	if(!item)
	{
		clearSelection();
	}

	int nb=KopeteContactList::contactList()->selectedMetaContacts().count() + KopeteContactList::contactList()->selectedGroups().count();


	//FIXME: this assume that the parent is the main window. That should be the case, but it is not good desing
	//       for me, the KopeteWindow should be moved into contactlist/ and be accessed easily with it
	KMainWindow *window=dynamic_cast<KMainWindow*>(parent());
	if(!window)
	{
		kdWarning(14000) << "KopeteContactListView::showContextMenu: WARNING main window not found" << endl;
		return;
	}

	if( metaLVI && nb==1)
	{
		int px = mapFromGlobal( point ).x() -
			( header()->sectionPos( header()->mapToIndex( 0 ) ) +
			treeStepSize() * ( item->depth() +
			( rootIsDecorated() ? 1 : 0 ) ) + itemMargin() );
		int py = mapFromGlobal( point ).y() - itemRect( item ).y() -
			header()->height();

//		kdDebug(14000) << "KopeteContactListView::showContextMenu: x: " << px << ", y: " << py << endl;
		KopeteContact *c = metaLVI->getContactFromIcon( QPoint( px, py ) ) ;
		if( c )
		{
			KPopupMenu *p = c->popupMenu();
			p->exec( point );
			delete p;
		}
		else
		{
			KPopupMenu *popup = dynamic_cast<KPopupMenu*>(window->factory()->container("contact_popup",window));
			QString title=i18n( "Translators: format: '<nickname> (<online status>)'", "%1 (%2)" ).
#if QT_VERSION < 0x030200
				arg( metaLVI->metaContact()->displayName() ).arg( metaLVI->metaContact()->statusString() );
#else
				arg( metaLVI->metaContact()->displayName() , metaLVI->metaContact()->statusString() );
#endif
			if(title.length() > 43 )
				title = title.left(40) + QString::fromLatin1("...");

			if(popup->title(0).isNull())
				popup->insertTitle (title ,0,0);
			else
				popup->changeTitle (0,title);

			//-- Submenus for separate contacts actions
			bool sep=false;  //FIXME: find if there is already a separator in the end
			QPtrList<KopeteContact> it = metaLVI->metaContact()->contacts();
			for( KopeteContact *c = it.first(); c; c = it.next() )
			{
				if( sep )
				{
					popup->insertSeparator();
					sep = false;
				}

				KPopupMenu *contactMenu = it.current()->popupMenu();
				connect( popup, SIGNAL(aboutToHide()) , contactMenu , SLOT(deleteLater()));
				QString text= i18n( "Translators: format: '<displayName> (<id>)'", "%2 <%1>" ).
#if QT_VERSION < 0x030200
					arg( c->contactId() ).arg( c->displayName() );
#else
					arg( c->contactId(), c->displayName() );
#endif
				if(text.length() > 41 )
					text = text.left(38) + QString::fromLatin1("...");

				popup->insertItem( c->onlineStatus().iconFor( c, 16 ), text , contactMenu );
			}

			if(popup)
				popup->exec(point);
		}
	}
	else if(groupvi && nb==1)
	{
		KPopupMenu *popup = dynamic_cast<KPopupMenu*>(window->factory()->container("group_popup",window));
		QString title=groupvi->group()->displayName();
		if(title.length() > 32 )
			title = title.left(30) + QString::fromLatin1("...");
		//popup->insertTitle (title,0,0);
		if(popup->title(0).isNull())
			popup->insertTitle (title ,0,0);
		else
			popup->changeTitle (0,title);

		popup->changeTitle(0,title);

		if(popup)
			popup->exec(point);
	}
	else if(nb>=1)
	{
		KPopupMenu *popup = dynamic_cast<KPopupMenu*>(window->factory()->container("contactlistitems_popup",window));
		if(popup)
			popup->exec(point);
	}
	else
	{
		KPopupMenu *popup = dynamic_cast<KPopupMenu*>(window->factory()->container("contactlist_popup",window));

		if(popup->title(0).isNull())
			popup->insertTitle(i18n("Kopete") ,0,0);

		if(popup)
			popup->exec(point);
	}

//		popup->popup( QCursor::pos() );
}

void KopeteContactListView::slotShowAddContactDialog()
{
	( new AddContactWizard( qApp->mainWidget() ) )->show();
}

void KopeteContactListView::removeGroup()
{
	if(!KopetePrefs::prefs()->sortByGroup())
		return;

	if ( !removeGroupItem )
		return;

	QString msg;
	if ( removeGroupItem->firstChild() )
		msg = i18n( "<qt>Are you sure you want to remove the group %1 and all "
            "contacts that are contained within it?</qt>" ).arg(removeGroupItem->group()->displayName());
	else
		msg = i18n( "<qt>Are you sure you want to remove the group %1?</qt>" ).arg(removeGroupItem->group()->displayName());

	if( KMessageBox::warningYesNo( this, msg,
		i18n( "Remove Group" ) ) == KMessageBox::Yes )
	{
		QListViewItem *lvi, *lvi2;
		for( lvi = removeGroupItem->firstChild(); lvi; lvi = lvi2 )
		{
			lvi2 = lvi->nextSibling();
			KopeteMetaContactLVI *kc =
				dynamic_cast<KopeteMetaContactLVI*>( lvi );
			if( kc )
			{
				kc->metaContact()->removeFromGroup(removeGroupItem->group());
			}
		}

		if( removeGroupItem->childCount() >= 1 )
		{
			kdDebug(14000) << "KopeteContactListView::removeGroup(): "
				<< "all subMetaContacts are not removed... Aborting" << endl;
			return;
		}

		mGroups.remove( removeGroupItem );
		KopeteContactList::contactList()->removeGroup(removeGroupItem->group());
//		delete removeGroupItem;
	}
}

void KopeteContactListView::slotSettingsChanged( void )
{
	KopeteGroupViewItem *gi;
	mShowAsTree = KopetePrefs::prefs()->treeView();
	if( mShowAsTree )
	{
		setRootIsDecorated( true );
		setTreeStepSize( 20 );
	}
	else
	{
		setRootIsDecorated( false );
		setTreeStepSize( 0 );
	}
	if(KopetePrefs::prefs()->sortByGroup())
	{
		bool cont;
		do          //slotAddedToGroup change the m_metaContacts.current()
		{
			cont=false;
			KopeteMetaContactLVI *li = m_metaContacts.first();
			for( ; li; li = m_metaContacts.next() )
			{
				if ( !li->isGrouped() )
				{
					KopeteMetaContact *mc= li->metaContact();
					m_metaContacts.setAutoDelete(false);
					m_metaContacts.remove( li );
					delete li;

					if( mc->isTopLevel() )
						slotAddedToGroup( mc, KopeteGroup::toplevel );
					KopeteGroupList list=mc->groups();
					for(KopeteGroup *it=list.first() ; it; it=list.next() )
						slotAddedToGroup( mc, it );

					cont = true;
					break;
				}
			}
		} while (cont);

		delete m_onlineItem;
		m_onlineItem=0l;
		delete m_offlineItem;
		m_offlineItem=0l;
	}
	else
	{
		m_metaContacts.setAutoDelete(false);
		mGroups.setAutoDelete(false);
		bool cont;
		do  //slotContactStatusChanged change the m_metaContacts.current()
		{
			cont=false;
			for(KopeteMetaContactLVI *li = m_metaContacts.first(); li ; li=m_metaContacts.next() )
			{
				if ( li->isGrouped() && !li->metaContact()->isTemporary())
				{
					KopeteMetaContact *mc=li->metaContact();
					m_metaContacts.remove( li );
					delete li;
					slotContactStatusChanged( mc );
					cont=true;
					break;
				}
			}
		}  while (cont);
		gi = mGroups.first();
		while ( (gi=mGroups.current()) )
		{
			if ( gi->group() != KopeteGroup::temporary)
			{
				mGroups.remove( gi );
				delete gi;
			}
			else
				mGroups.next();
		}
	}
	for( gi = mGroups.first(); gi; gi = mGroups.next() )
	{
		if (!mShowAsTree)
			gi->setPixmap( 0, classic );
		else
		{
			if (isOpen(gi))
				gi->setPixmap( 0, open );
			else
				gi->setPixmap( 0, closed );
		}
	}
	delete gi;
	update();
/*
	if( KopetePrefs::prefs()->ctransparencyEnabled() )
	{
		kdDebug(14000) << k_funcinfo << "Requested Transparent ContactList." << endl;
		if( root == 0L )
		{
			kdDebug(14000) << k_funcinfo << "Enabling Transparency." << endl;
			root = new KRootPixmap( this );
			root->setFadeEffect(KopetePrefs::prefs()->transparencyValue() * 0.01, KopetePrefs::prefs()->transparencyColor() );
			root->start();
		}
		else
		{
			kdDebug(14000) << k_funcinfo << "Updating Transparency." << endl;
			root->stop();
			root->setFadeEffect(KopetePrefs::prefs()->transparencyValue() * 0.01, KopetePrefs::prefs()->transparencyColor() );
			root->start();
		}
		root->repaint( true );
	}
	else
	{
		kdDebug(14000) << k_funcinfo << "No Transparent ContactList wanted." << endl;
		if ( root != 0L )
		{
			kdDebug(14000) << k_funcinfo << "Disabling Transparency." << endl;
			root->stop();
			delete root;
			root = 0L;
		}
	}
*/
}

void KopeteContactListView::slotExecuted( QListViewItem *item,
	const QPoint &p, int /* col */ )
{
	KopeteMetaContactLVI *metaContactLVI =
		dynamic_cast<KopeteMetaContactLVI*>( item );

	QPoint pos = viewport()->mapFromGlobal( p );
	KopeteContact *c = 0L;
	if( metaContactLVI )
	{
		// Try if we are clicking a protocol icon. If so, open a direct
		// connection for that protocol
		QRect r = itemRect( item );
		QPoint relativePos( pos.x() - r.left() - ( treeStepSize() *
			( item->depth() + ( rootIsDecorated() ? 1 : 0 ) ) +
			itemMargin() ), pos.y() - r.top() );
		c = metaContactLVI->getContactFromIcon( relativePos );
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

void KopeteContactListView::slotDropped(QDropEvent *e, QListViewItem */*parent*/, QListViewItem *after)
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
		source_contact = source_metaLVI->getContactFromIcon( m_startDragPos );

	if(source_metaLVI  && dest_groupLVI)
	{
		if(source_metaLVI->group() == dest_groupLVI->group())
			return;
		if(source_metaLVI->metaContact()->isTemporary())
		{
			int r=KMessageBox::questionYesNo( qApp->mainWidget(), i18n( "<qt>Would you like to add this contact to your contaclist</qt>" ),
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
		if(source_metaLVI->group() == KopeteGroup::toplevel)
			return;
		if(source_metaLVI->metaContact()->isTemporary())
		{
			int r=KMessageBox::questionYesNo( qApp->mainWidget(), i18n( "<qt>Would you like to add this contact to your contaclist</qt>" ),
				i18n( "Kopete" ), KStdGuiItem::yes(),KStdGuiItem::no(),"addTemporaryWhenMoving" );

			if(r==KMessageBox::Yes)
				source_metaLVI->metaContact()->setTemporary( false, KopeteGroup::toplevel );
		}
		else
		{
//			kdDebug(14000) << "KopeteContactListView::slotDropped : moving the meta contact " << source_metaLVI->metaContact()->displayName()			<< " to top-level " <<	endl;
			source_metaLVI->metaContact()->moveToGroup(source_metaLVI->group() , KopeteGroup::toplevel);
		}
	}
	else if(source_contact && dest_metaLVI) //we are moving a contact to another metacontact
	{
		if(source_metaLVI->metaContact()->isTemporary())
		{
			/*int r=KMessageBox::questionYesNo( qApp->mainWidget(), i18n( "<qt>Would you like to add this contact to your contaclist</qt>" ),
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
		KopeteContact *c = dest_metaLVI->getContactFromIcon( QPoint( px, py ) ) ;

		for ( KURL::List::Iterator it = urlList.begin(); it != urlList.end(); ++it )
		{
			if(c)
				c->sendFile( *it );
			else
				dest_metaLVI->metaContact()->sendFile( *it );
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
	const_cast<KopeteContactListView *>( this )->findDrop( e->pos(),
		parent, afterme );

	KopeteMetaContactLVI *dest_metaLVI=dynamic_cast<KopeteMetaContactLVI*>(afterme);

	if( e->source() == viewport() )
	{
		KopeteMetaContactLVI *source_metaLVI=dynamic_cast<KopeteMetaContactLVI*>(source);
		KopeteGroupViewItem *dest_groupLVI=dynamic_cast<KopeteGroupViewItem*>(afterme);
		KopeteContact *source_contact=0L;

		if(source_metaLVI)
			source_contact = source_metaLVI->getContactFromIcon( m_startDragPos );

		if( source_metaLVI && dest_groupLVI && !source_contact) //we are moving a metacontact to another group
		{
			if(source_metaLVI->group() == dest_groupLVI->group())
				return false;
			if(dest_groupLVI->group() == KopeteGroup::temporary)
				return false;
	//		if(source_metaLVI->metaContact()->isTemporary())
	//			return false;
			return true;
		}
		else if(source_metaLVI  && !dest_metaLVI && !dest_groupLVI && source_contact) //we are moving a metacontact to toplevel
		{
			if(source_metaLVI->group() == KopeteGroup::toplevel)
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
			KopeteContact *c = dest_metaLVI->getContactFromIcon( QPoint( px, py ) ) ;
			if(c)
				return c->canAcceptFiles();
			else //to the metacontact
				return true;
		}
	}

	return false;
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
		currentItem()->setRenameEnabled(0, true);
		KListView::keyPressEvent(e);
		currentItem()->setRenameEnabled(0, false);
	}
	else if ( (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) && currentItem())
	{
		slotExecuted(currentItem() , QPoint() , 0);
	}
	else
		KListView::keyPressEvent(e);

}

void KopeteContactListView::contentsMouseMoveEvent( QMouseEvent *e )
{

#if QT_VERSION < 0x030100
//tooltips does not works for QT >= 3.1.   FIXME: but what is the problem???????
// If i remove this code, AND call setTooltipColumn(0) in the constructor, default toolTip works (i.e:
//  just the item's text, if it is turncated).
// I tried to reimpement KListView::toolTip , without succes. When looking at the KListView's code, I
// see that KListView::tooltip() , KListView::showTooltip, class KListView::Tooltip dos not seems to be
// even  KListView::setTooltipColumn() seems useless.. but then, WHT even default Tooltips DOES NOT WORK
// if i don't call setTooltipColumn(0) in KopeteContactListView's constructor  (which is set to 0 by default)??
// HELP ME! HELP ME! HELP ME! HELP ME! HELP ME! HELP ME! HELP ME! HELP ME! HELP ME!   -Olivier

	if( e->state() != Qt::NoButton )
	{
		KListView::contentsMouseMoveEvent( e );
		return;
	}

	QListViewItem *item = itemAt( e->pos () );
	if( !item )
	{
		KListView::contentsMouseMoveEvent( e );
		return;
	}

	QPoint pos = contentsToViewport( e->pos() );
	QString tip;

	// Hide any tooltips currently shown
	// If we dont do this first, the tooltip is continually shown (and updated)
	// when moving from one contact to the next!
	QToolTip::hide();

	// First, delete the last tooltip, since we dont need it anymore
	if(!m_onItem.isNull())
	{
/*		kdDebug(14000) << "KopeteContactListView::contentsMouseMoveEvent: "
			<< "Removing tooltip at top=" << m_onItem.top()
			<< ", left=" << m_onItem.left() << ", bottom="
			<< m_onItem.bottom() << ", right=" << m_onItem.right() << endl;*/
		QToolTip::remove( this, m_onItem );
	}

	//a little sanity check, there shouldnt already be a tip here, but if there is, don't add a new one
	if(QToolTip::textFor( this, e->pos() ).isNull() )
	{
		KopeteMetaContactLVI *metaLVI =
			dynamic_cast<KopeteMetaContactLVI*>( item );
		KopeteGroupViewItem *groupLVI =
			dynamic_cast<KopeteGroupViewItem*>( item );

		//FIXME: add more info to the tooltips! they are sorta bare at the
		// moment! Time last online, Idle time, members of groups, etc...
		KopeteContact *c = 0L;
		if( metaLVI )
		{
			// Try if we are hovering over a protocol icon. If so, use that
			// tooltip in the code below
			QRect r = itemRect( item );
			QPoint relativePos( pos.x() - r.left() - ( treeStepSize() *
				( item->depth() + ( rootIsDecorated() ? 1 : 0 ) ) +
				itemMargin() ), pos.y() - r.top() );
			c = metaLVI->getContactFromIcon( relativePos );
			if( c )
			{
				tip = i18n( "<b>%3</b><br>%2<br>Status: %1" ).
#if QT_VERSION < 0x030200
					arg( c->onlineStatus().description() ).arg( QStyleSheet::escape( c->contactId() ) ).arg( QStyleSheet::escape( c->displayName() ) );
#else
					arg( c->onlineStatus().description(), QStyleSheet::escape( c->contactId() ), QStyleSheet::escape( c->displayName() ) );
#endif
			}
			else
			{
				KopeteMetaContact *mc = metaLVI->metaContact();
				tip = i18n( "<b>%2</b><br>Status: %1" ).
#if QT_VERSION < 0x030200
					arg( mc->statusString() ).arg( QStyleSheet::escape( mc->displayName() ) );
#else
					arg( mc->statusString(), QStyleSheet::escape( mc->displayName() ) );
#endif
			}
		}
		else if( groupLVI )
		{
			//KopeteGroup *g=groupLVI->group();
			//FIXME: user item->text(0) for now so we get the # online / # total, since there is no interface
			//FIXME: to get these from KopeteGroup

			tip=QString("<b>%1</b>").arg(item->text(0));
		}

		m_onItem=itemRect(item);
		//we have to do this to account for the header, these co-ordinate systems are screwed :p
		m_onItem.moveBy(0,header()->height());
//		kdDebug(14000) << "KopeteContactListView::contentsMouseMoveEvent - ADDING TOOLTIP AT top="<<m_onItem.top()<<" tip="<<tip<<endl;
		QToolTip::add(this,m_onItem,tip);
	}
	else
	{
//		kdDebug(14000) << "KopeteContactListView::contentsMouseMoveEvent - Already has a tooltip, not adding a new one" << endl;
	}
#endif //qt < 3.1

	// Also call parent, or we'll break drag-n-drop
	KListView::contentsMouseMoveEvent( e );
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

	// Find out if we're starting a drag using parent's implementation
	QDragObject *drag= KListView::dragObject();
	if ( drag )
	{
		//we're starting to drag something
		KopeteMetaContactLVI *source_metaLVI =
			dynamic_cast<KopeteMetaContactLVI*>( currentItem() );
		KopeteContact *c = 0L;
		QPixmap pm;

		// get the pixmap depending what we're dragging
		if ( source_metaLVI )
		{
			c = source_metaLVI->getContactFromIcon( m_startDragPos );

			if ( c ) 	// dragging a contact
				pm = c->onlineStatus().iconFor( c, 12 ); // FIXME: fixed icon scaling
			else		// dragging a metacontact
			{
				// FIXME: first start at rendering the whole MC incl small icons
				// into a pixmap to drag - anyone know how to complete this?
				//QPainter p( pm );
				//source_metaLVI->paintCell( p, cg?, width(), 0, 0 );
				pm = UserIcon( source_metaLVI->metaContact()->statusIcon() );
			}
		}

		//QSize s = pm.size();
		drag->setPixmap( pm /*, QPoint( s.width() , s.height() )*/ );
	}
	return drag;
}

void KopeteContactListView::slotSelectionChanged()
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
	KopeteContactList::contactList()->setSelectedItems(contacts , groups);

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
		m->sendFile(QString::null);
}

void KopeteContactListView::slotMoveToGroup()
{
	KopeteMetaContactLVI *metaLVI=dynamic_cast<KopeteMetaContactLVI*>(currentItem());
	if(!metaLVI)
		return;
	KopeteMetaContact *m=metaLVI->metaContact();
	KopeteGroup *g=metaLVI->group();

	//FIXME! what ios two groups have the same name?
	KopeteGroup *to = actionMove->currentItem() ? KopeteContactList::contactList()->getGroup( actionMove->currentText() )  : KopeteGroup::toplevel;

	if( !to ||  to==KopeteGroup::temporary )
		return;


	if(m->isTemporary())
	{
		if( KMessageBox::questionYesNo( qApp->mainWidget(), i18n( "<qt>Would you like to add this contact to your contaclist</qt>" ),
				i18n( "Kopete" ), KStdGuiItem::yes(),KStdGuiItem::no(),"addTemporaryWhenMoving" ) == KMessageBox::Yes )
		{
			m->setTemporary(false,g);
		}
	}
	else if( !m->groups().contains( to ) )
		m->moveToGroup( g, to );

	actionMove->setCurrentItem( -1 );

}
void KopeteContactListView::slotCopyToGroup()
{
	KopeteMetaContact *m=KopeteContactList::contactList()->selectedMetaContacts().first();
	if(!m)
		return;

	//FIXME! what ios two groups have the same name?
	KopeteGroup *to = actionCopy->currentItem() ? KopeteContactList::contactList()->getGroup( actionCopy->currentText() )  : KopeteGroup::toplevel;

	if( !to ||  to==KopeteGroup::temporary )
		return;

	if(m->isTemporary())
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


	if(items.count()<=1)
	{  //we are deleting an empty contact
		QString msg;
		if(!contacts.isEmpty())
		{
			msg=i18n("Are you sure you want to the contact `%1` from your contact list?").arg(contacts.first()->displayName()) ;
		}
		else if(!groups.isEmpty())
		{
			msg = i18n( "Are you sure you want to remove the group `%1` and all contacts that are contained within it?" ).arg(groups.first()->displayName());
		}
		else return; //this should never happends

		if( KMessageBox::warningYesNo( this, msg, i18n( "Remove - Kopete" ) ) != KMessageBox::Yes )
			return;
	}
	else
	{
		QString msg;
		if(groups.isEmpty())
		{
			msg=i18n("Are you sure you want to remove theses contact from your contact list?");
		}
		else
		{
			msg = i18n( "Are you sure you want to remove theses groups or contacts from your cotactlist?" );
		}

		if( KMessageBox::questionYesNoList(this, msg , items , i18n("Remove - Kopete") , KStdGuiItem::yes(), KStdGuiItem::no(), "askRemovingContactOrGroup" )
			 != KMessageBox::Yes )
			 	return;
	}

	for( KopeteMetaContact *it = contacts.first(); it; it = contacts.next() )
	{
		KopeteContactList::contactList()->removeMetaContact( it );
	}
	for( KopeteGroup *it = groups.first(); it; it = groups.next() )
	{
		removeGroupItem=getGroup(it , false);
		QListViewItem *lvi,*lvi2;
		for( lvi = removeGroupItem->firstChild(); lvi; lvi = lvi2 )
		{
			lvi2 = lvi->nextSibling();
			KopeteMetaContactLVI *kc =
				dynamic_cast<KopeteMetaContactLVI*>( lvi );
			if( kc )
			{
				kc->metaContact()->removeFromGroup(it);
			}
		}

		if( removeGroupItem->childCount() >= 1 )
		{
			kdDebug(14000) << "KopeteContactListView::slotRemove(): "
				<< "all subMetaContacts are not removed... Aborting" << endl;
			continue;
		}

		mGroups.remove( removeGroupItem );
		KopeteContactList::contactList()->removeGroup(removeGroupItem->group());
	}
}

void KopeteContactListView::slotRename()
{
	KopeteMetaContactLVI *metaLVI=dynamic_cast<KopeteMetaContactLVI*>(currentItem());

	if(metaLVI)
		metaLVI->slotRename();
	else if( KopeteContactList::contactList()->selectedGroups().count()==1)
	{
		if(!KopetePrefs::prefs()->sortByGroup())
			return;

		KopeteGroup* group=KopeteContactList::contactList()->selectedGroups().first();

		bool ok;
		QString newname = KLineEditDlg::getText(
			i18n( "Rename Group" ),
			i18n( "Please enter the new name for the group '%1':" ).arg(group->displayName()),
			group->displayName(), &ok );
		if( !ok )
			return;
		group->setDisplayName(newname);
	}
}

void KopeteContactListView::slotAddContact()
{
	if(!sender()) return;

	KopeteMetaContact *metacontact=KopeteContactList::contactList()->selectedMetaContacts().first();
	KopeteAccount *account=dynamic_cast<KopeteAccount*>(sender()->parent());

	if( account && metacontact && !metacontact->isTemporary())
	{
		KDialogBase *addDialog= new KDialogBase( this, "addDialog", true, i18n( "Add Contact" ), KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );

		//addDialog->resize( 543, 379 );

		AddContactPage *addContactPage = account->protocol()->createAddContactWidget(addDialog, account);
		if (!addContactPage)
		{
			kdDebug(14000) << "KopeteContactListView::slotAddContact : error while creating addcontactpage" <<endl;
		}
		else
		{
			addDialog->setMainWidget(addContactPage);
			if(addDialog->exec() == QDialog::Accepted && addContactPage->validateData())
			{
				addContactPage->apply(account , metacontact);
			}
		}
		addDialog->deleteLater();
	}
}

#include "kopetecontactlistview.moc"

// vim: set noet ts=4 sts=4 sw=4:

