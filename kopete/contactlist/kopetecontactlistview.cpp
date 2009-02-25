/*
    Kopete Contact List View

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Nick Betcher           <nbetcher@usinternet.com>
    Copyright (c) 2002      by Stefan Gehn            <metz@gehn.net>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright     2007      by Matt Rogers            <mattr@kde.org>
    Copyright     2009      by Roman Jarosz           <kedgedev@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers  <kopete-devel@kde.org>

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

#include <QHeaderView>

#include <KDebug>
#include <KIcon>
#include <KMenu>
#include <KStandardAction>
#include <KActionMenu>
#include <KActionCollection>
#include <KXmlGuiWindow>
#include <KLocalizedString>
#include <kxmlguifactory.h>

#include "kopeteuiglobal.h"
#include "kopetecontactlistelement.h"
#include "kopetemetacontact.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetegroup.h"
#include "kopetecontactlist.h"
#include "kopetestdaction.h"
#include "kopetegrouplistaction.h"
#include "kopetelviprops.h"
#include "contactlistlayoutmanager.h"

#include "contactlistmodel.h"
#include "kopeteitembase.h"
#include "kopeteitemdelegate.h"

class KopeteContactListViewPrivate
{
public:
	// HACK: Used to update the KMEnu title - DarkShock
	QMap<KMenu*, QAction*> menuTitleMap;
};

KopeteContactListView::KopeteContactListView( QWidget *parent )
: QTreeView( parent ), d( new KopeteContactListViewPrivate() )
{
	header()->hide();

	setSelectionMode( QAbstractItemView::ExtendedSelection );
	setDragEnabled( true );
	setAcceptDrops( true );
	setAlternatingRowColors( true );
	setAnimated( true );
	setDropIndicatorShown( true );
	setItemDelegate( new KopeteItemDelegate( this ) );

	connect( this, SIGNAL( activated(const QModelIndex&)),
	         this, SLOT( contactActivated(const QModelIndex&)));
	connect( this, SIGNAL(expanded(const QModelIndex&)),
	         this, SLOT(itemExpanded(const QModelIndex&)));
	connect( this, SIGNAL(collapsed(const QModelIndex&)),
	         this, SLOT(itemCollapsed(const QModelIndex&)));
	connect( ContactList::LayoutManager::instance(), SIGNAL(activeLayoutChanged()),
	         this, SLOT(reset()) );

	setEditTriggers( NoEditTriggers );
	// Load in the user's initial settings
	//slotSettingsChanged();
}

void KopeteContactListView::initActions( KActionCollection *ac )
{
	actionUndo = KStandardAction::undo( this , SLOT( slotUndo() ) , ac );
	actionRedo = KStandardAction::redo( this , SLOT( slotRedo() ) , ac );
	actionUndo->setEnabled(false);
	actionRedo->setEnabled(false);


	KAction *actionCreateNewGroup = new KAction( i18n( "Create New Group..." ), ac );
	actionCreateNewGroup->setIcon( KIcon( "user-group-new" ) );
	connect( actionCreateNewGroup, SIGNAL( triggered(bool) ), this, SLOT( addGroup() ) );
	ac->addAction( "AddGroup", actionCreateNewGroup );

	actionSendMessage = KopeteStdAction::sendMessage( this, SLOT( slotSendMessage() ), ac );
	ac->addAction( "contactSendMessage", actionSendMessage );
	actionStartChat = KopeteStdAction::chat( this, SLOT( slotStartChat() ), ac );
	ac->addAction( "contactStartChat", actionStartChat );

	actionMove = new KopeteGroupListAction( i18n( "&Move To" ), QLatin1String( "edit-cut" ),
                                                KShortcut(), this, SLOT( slotMoveToGroup() ), ac );
	ac->addAction( "contactMove", actionMove );
	actionCopy = new KopeteGroupListAction( i18n( "&Copy To" ), QLatin1String( "edit-copy" ),
                                                KShortcut(), this, SLOT( slotCopyToGroup() ), ac );
	ac->addAction( "contactCopy", actionCopy );

	actionMakeMetaContact = new KAction(KIcon("list-add-user"), i18n("Make Meta Contact"), ac);
	ac->addAction( "makeMetaContact", actionMakeMetaContact );
	connect (actionMakeMetaContact, SIGNAL(triggered(bool)), this, SLOT(mergeMetaContact()));

	actionRemove = KopeteStdAction::deleteContact( this, SLOT( slotRemove() ), ac );
	ac->addAction( "contactRemove", actionRemove );

	actionSendEmail = new KAction( KIcon("mail-send"), i18n( "Send Email..." ), ac );
	ac->addAction( "contactSendEmail", actionSendEmail );
	connect( actionSendEmail, SIGNAL( triggered(bool) ), this, SLOT( slotSendEmail() ) );

// 	-* this actionRename is buggy, and useless with properties, removed in kopeteui.rc*-
	actionRename = new KAction( KIcon("edit-rename"), i18n( "Rename" ), ac );
	ac->addAction( "contactRename", actionRename );
	connect( actionRename, SIGNAL( triggered(bool) ), this, SLOT( slotRename() ) );

	actionSendFile = KopeteStdAction::sendFile( this, SLOT( slotSendFile() ), ac );
	ac->addAction( "contactSendFile", actionSendFile );

	actionAddContact = new KActionMenu( KIcon( QLatin1String("list-add-user") ), i18n( "&Add Contact" ), ac );
	ac->addAction( "contactAddContact", actionAddContact );
	actionAddContact->menu()->addTitle( i18n("Select Account") );

	actionAddTemporaryContact = new KAction( KIcon("list-add-user"), i18n( "Add to Your Contact List" ), ac );
	ac->addAction( "contactAddTemporaryContact", actionAddTemporaryContact );
	connect( actionAddTemporaryContact, SIGNAL( triggered(bool) ), this, SLOT( slotAddTemporaryContact() ) );

	connect( Kopete::ContactList::self(), SIGNAL( metaContactSelected( bool ) ), this, SLOT( slotMetaContactSelected( bool ) ) );

	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered( Kopete::Account* )), SLOT(slotAddSubContactActionNewAccount(Kopete::Account*)));
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered( const Kopete::Account* )), SLOT(slotAddSubContactActionAccountDeleted(const Kopete::Account *)));

	actionProperties = new KAction( KIcon("user-properties"), i18n( "&Properties" ), ac );
	ac->addAction( "contactProperties", actionProperties );
	actionProperties->setShortcut( KShortcut(Qt::Key_Alt + Qt::Key_Return) );
	connect( actionProperties, SIGNAL( triggered(bool) ), this, SLOT( showItemProperties() ) );

	// Update enabled/disabled actions
	//	slotViewSelectionChanged();
}


KopeteContactListView::~KopeteContactListView()
{
	delete d;
}

Kopete::MetaContact* KopeteContactListView::metaContactFromIndex( const QModelIndex& index ) const
{
	QObject* metaContactObject = qVariantValue<QObject*>( index.data( Kopete::Items::ObjectRole ) );
	return qobject_cast<Kopete::MetaContact*>(metaContactObject);
}

Kopete::Group* KopeteContactListView::groupFromIndex( const QModelIndex& index ) const
{
	QObject* groupObject = qVariantValue<QObject*>( index.data( Kopete::Items::ObjectRole ) );
	return qobject_cast<Kopete::Group*>(groupObject);
}

void KopeteContactListView::contactActivated( const QModelIndex& index )
{
	QVariant v = index.data( Kopete::Items::ElementRole );
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		Kopete::MetaContact* mc = metaContactFromIndex( index );
		if ( mc )
			mc->execute();
	}


}

void KopeteContactListView::reset()
{
	// TODO: Save/restore expand state
	QTreeView::reset();
}

void KopeteContactListView::itemExpanded( const QModelIndex& index )
{
	Q_ASSERT(model());
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		model()->setData( index, KIcon(KOPETE_GROUP_DEFAULT_OPEN_ICON),
	                      Qt::DecorationRole );
	}
}

void KopeteContactListView::itemCollapsed( const QModelIndex& index )
{
	Q_ASSERT(model());
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		model()->setData( index, KIcon(KOPETE_GROUP_DEFAULT_CLOSED_ICON),
		                  Qt::DecorationRole );
	}
}

void KopeteContactListView::showItemProperties()
{
	QModelIndex index = currentIndex();
	if ( !index.isValid() )
		return;

	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		KopeteMetaLVIProps *propsDialog = new KopeteMetaLVIProps( metaContactFromIndex( index ), 0L );
		propsDialog->exec(); // modal
		delete propsDialog;
	}
	else if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		KopeteGVIProps *propsDialog = new KopeteGVIProps( groupFromIndex( index ), 0L );
		propsDialog->exec(); // modal
		delete propsDialog;
	}
}

void KopeteContactListView::mergeMetaContact()
{
	// Get metaContacts as indexes could change during merge.
	QList<Kopete::MetaContact *> metaContactList;
	foreach ( QModelIndex index, selectedIndexes() )
	{
		Kopete::MetaContact* mc = metaContactFromIndex( index );
		if ( mc )
			metaContactList.append( mc );
	}

	if ( metaContactList.count() < 2 )
		return;

	Kopete::MetaContact* mainMetaContact = metaContactList.first();
	for ( int i = 1; i < metaContactList.count(); i++ )
	{
		QList<Kopete::Contact*> contactList = metaContactList.at( i )->contacts();
		foreach ( Kopete::Contact *contact, contactList )
			contact->setMetaContact( mainMetaContact );
	}
}

void KopeteContactListView::contextMenuEvent( QContextMenuEvent* event )
{
	Q_ASSERT(model());
	QModelIndexList indexList = selectedIndexes();
	if ( indexList.isEmpty() )
		return;

	if ( indexList.count() > 1 )
	{
		miscPopup( indexList, event->globalPos() );
	}
	else
	{
		QModelIndex index = indexList.first();
		if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
			metaContactPopup( metaContactFromIndex( index ), event->globalPos() );
		else if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
			groupPopup( groupFromIndex( index ), event->globalPos() );
	}
	event->accept();
}

void KopeteContactListView::groupPopup( Kopete::Group *group, const QPoint& pos )
{
	Q_ASSERT(group);
	KXmlGuiWindow *window = dynamic_cast<KXmlGuiWindow *>(topLevelWidget());
	if ( !window )
	{
		kError( 14000 ) << "Main window not found, unable to display context-menu; "
			<< "Kopete::UI::Global::mainWidget() = " << Kopete::UI::Global::mainWidget() << endl;
		return;
	}

	KMenu *popup = dynamic_cast<KMenu *>( window->factory()->container( "group_popup", window ) );
	if ( popup )
	{
		QString title = group->displayName();
		if ( title.length() > 32 )
			title = title.left( 30 ) + QLatin1String( "..." );

		// HACK: Used to update the KMenu title -DarkShock
		if( d->menuTitleMap.contains(popup) )
		{
			QAction *action = d->menuTitleMap[popup];
			popup->removeAction( action );
			delete action;
		}
		d->menuTitleMap.insert( popup, popup->addTitle(title, popup->actions().first()) );
		popup->popup( pos );
	}
}

void KopeteContactListView::metaContactPopup( Kopete::MetaContact *metaContact, const QPoint& pos )
{
	Q_ASSERT(metaContact);
	KXmlGuiWindow *window = dynamic_cast<KXmlGuiWindow *>(topLevelWidget());
	if ( !window )
	{
		kError( 14000 ) << "Main window not found, unable to display context-menu; "
			<< "Kopete::UI::Global::mainWidget() = " << Kopete::UI::Global::mainWidget() << endl;
		return;
	}

	KMenu *popup = dynamic_cast<KMenu *>( window->factory()->container( "contact_popup", window ) );
	if ( popup )
	{
		QString title = i18nc( "Translators: format: '<nickname> (<online status>)'", "%1 (%2)",
		                       metaContact->displayName(), metaContact->statusString() );

		if ( title.length() > 43 )
			title = title.left( 40 ) + QLatin1String( "..." );

		// HACK: Used to update the KMenu title -DarkShock
		if( d->menuTitleMap.contains(popup) )
		{
			QAction *action = d->menuTitleMap[popup];
			popup->removeAction( action );
			delete action;
		}
		d->menuTitleMap.insert( popup, popup->addTitle(title, popup->actions().first()) );

		// Submenus for separate contact actions
		bool sep = false;  //FIXME: find if there is already a separator in the end - Olivier
		foreach( Kopete::Contact* c , metaContact->contacts() )
		{
			if( sep )
			{
				popup->addSeparator();
				sep = false;
			}

			KMenu *contactMenu = c->popupMenu();
			connect( popup, SIGNAL( aboutToHide() ), contactMenu, SLOT( deleteLater() ) );
			QString nick = c->property(Kopete::Global::Properties::self()->nickName()).value().toString();
			QString text = nick.isEmpty() ?  c->contactId() : i18nc( "Translators: format: '<displayName> (<id>)'", "%2 <%1>", c->contactId(), nick );
			text=text.replace('&',"&&"); // cf BUG 115449

			if ( text.length() > 41 )
				text = text.left( 38 ) + QLatin1String( "..." );

			contactMenu->setTitle(text);
			contactMenu->setIcon(c->onlineStatus().iconFor( c ));
			popup->addMenu( contactMenu );
		}
		popup->popup( pos );
	}
}

void KopeteContactListView::miscPopup( QModelIndexList indexes, const QPoint& pos )
{
	Q_ASSERT(indexes.count() > 1);
	KXmlGuiWindow *window = dynamic_cast<KXmlGuiWindow *>(topLevelWidget());
	if ( !window )
	{
		kError( 14000 ) << "Main window not found, unable to display context-menu; "
			<< "Kopete::UI::Global::mainWidget() = " << Kopete::UI::Global::mainWidget() << endl;
		return;
	}
	
	bool onlyMetaContacts = true;
	foreach ( QModelIndex index, indexes )
	{
		if ( index.data( Kopete::Items::TypeRole ) != Kopete::Items::MetaContact )
		{
			onlyMetaContacts = false;
			break;
		}
	}
	
	KMenu *popup = 0;
	if ( onlyMetaContacts )
		popup = dynamic_cast<KMenu *>( window->factory()->container( "contactlistitems_popup", window ) );
	
	if ( popup )
		popup->popup( pos );
}

#include "kopetecontactlistview.moc"

// vim: set noet ts=4 sts=4 sw=4:
