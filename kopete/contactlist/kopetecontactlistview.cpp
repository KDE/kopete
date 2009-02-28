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
#include <KInputDialog>
#include <KMessageBox>
#include <KToolInvocation>
#include <kxmlguifactory.h>
#include <kabc/stdaddressbook.h>

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
#include "kopeteaccount.h"
#include "addcontactpage.h"
#include "kopeteappearancesettings.h"
#include "kopetebehaviorsettings.h"

#include "contactlistmodel.h"
#include "kopeteitembase.h"
#include "kopeteitemdelegate.h"

class KopeteContactListViewPrivate
{
public:
	KopeteContactListViewPrivate() : controlPressed( false )
	{}
	//QRect m_onItem;
	
	// HACK: Used to update the KMEnu title - DarkShock
	QMap<KMenu*, QAction*> menuTitleMap;

	/* ACTIONS */
	KAction *actionSendMessage;
	KAction *actionStartChat;
	KAction *actionSendFile;
	KActionMenu *actionAddContact;
	KAction *actionSendEmail;
	KSelectAction *actionMove;
	KSelectAction *actionCopy;
	KAction *actionRename;
	KAction *actionRemove;
	KAction *actionAddTemporaryContact;
	KAction *actionProperties;
	KAction *actionUndo;
	KAction *actionRedo;
	KAction *actionMakeMetaContact;

	QMap<KAction*, Kopete::Account*> addContactAccountMap;
	QSet<int> notExpandedGroups;

	bool controlPressed;
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
	setExpandsOnDoubleClick( true );

	connect( this, SIGNAL( activated(const QModelIndex&)),
	         this, SLOT( contactActivated(const QModelIndex&)));
	connect( this, SIGNAL(expanded(const QModelIndex&)),
	         this, SLOT(itemExpanded(const QModelIndex&)));
	connect( this, SIGNAL(collapsed(const QModelIndex&)),
	         this, SLOT(itemCollapsed(const QModelIndex&)));
	connect( ContactList::LayoutManager::instance(), SIGNAL(activeLayoutChanged()),
	         this, SLOT(reset()) );
	connect( Kopete::BehaviorSettings::self(), SIGNAL(configChanged()), SLOT(slotSettingsChanged()) );
	connect( Kopete::AppearanceSettings::self(), SIGNAL(configChanged()), SLOT(slotSettingsChanged()) );

	setEditTriggers( NoEditTriggers );
	// Load in the user's initial settings
	slotSettingsChanged();
}

void KopeteContactListView::initActions( KActionCollection *ac )
{
// 	d->actionUndo = KStandardAction::undo( this , SLOT( slotUndo() ) , ac );
// 	d->actionRedo = KStandardAction::redo( this , SLOT( slotRedo() ) , ac );
// 	d->actionUndo->setEnabled(false);
// 	d->actionRedo->setEnabled(false);


	KAction *actionCreateNewGroup = new KAction( i18n( "Create New Group..." ), ac );
	actionCreateNewGroup->setIcon( KIcon( "user-group-new" ) );
	connect( actionCreateNewGroup, SIGNAL(triggered(bool)), this, SLOT(addGroup()) );
	ac->addAction( "AddGroup", actionCreateNewGroup );

	d->actionSendMessage = KopeteStdAction::sendMessage( this, SLOT(sendMessage()), ac );
	ac->addAction( "contactSendMessage", d->actionSendMessage );
	d->actionStartChat = KopeteStdAction::chat( this, SLOT(startChat()), ac );
	ac->addAction( "contactStartChat", d->actionStartChat );

// FIXME: Do we need this with drag&drop support?
// 	d->actionMove = new KopeteGroupListAction( i18n( "&Move To" ), QLatin1String( "edit-cut" ),
//                                                 KShortcut(), this, SLOT( slotMoveToGroup() ), ac );
// 	ac->addAction( "contactMove", d->actionMove );
// 	d->actionCopy = new KopeteGroupListAction( i18n( "&Copy To" ), QLatin1String( "edit-copy" ),
//                                                 KShortcut(), this, SLOT( slotCopyToGroup() ), ac );
// 	ac->addAction( "contactCopy", d->actionCopy );

	d->actionMakeMetaContact = new KAction(KIcon("list-add-user"), i18n("Merge Meta Contacts"), ac);
	ac->addAction( "makeMetaContact", d->actionMakeMetaContact );
	connect (d->actionMakeMetaContact, SIGNAL(triggered(bool)), this, SLOT(mergeMetaContact()));

	d->actionRemove = KopeteStdAction::deleteContact( this, SLOT(removeGroupOrMetaContact()), ac );
	ac->addAction( "contactRemove", d->actionRemove );

	d->actionSendEmail = new KAction( KIcon("mail-send"), i18n( "Send Email..." ), ac );
	ac->addAction( "contactSendEmail", d->actionSendEmail );
	connect( d->actionSendEmail, SIGNAL(triggered(bool)), this, SLOT(sendEmail()) );

// FIXME: Do we need this, it's in properties
// 	-* this actionRename is buggy, and useless with properties, removed in kopeteui.rc*-
// 	d->actionRename = new KAction( KIcon("edit-rename"), i18n( "Rename" ), ac );
// 	ac->addAction( "contactRename", d->actionRename );
// 	connect( d->actionRename, SIGNAL( triggered(bool) ), this, SLOT( slotRename() ) );

	d->actionSendFile = KopeteStdAction::sendFile( this, SLOT(sendFile()), ac );
	ac->addAction( "contactSendFile", d->actionSendFile );

	d->actionAddContact = new KActionMenu( KIcon( QLatin1String("list-add-user") ), i18n( "&Add Contact" ), ac );
	ac->addAction( "contactAddContact", d->actionAddContact );
	d->actionAddContact->menu()->addTitle( i18n("Select Account") );

	d->actionAddTemporaryContact = new KAction( KIcon("list-add-user"), i18n( "Add to Your Contact List" ), ac );
	ac->addAction( "contactAddTemporaryContact", d->actionAddTemporaryContact );
	connect( d->actionAddTemporaryContact, SIGNAL(triggered(bool)), this, SLOT(slotAddTemporaryContact()) );

// 	connect( Kopete::ContactList::self(), SIGNAL( metaContactSelected( bool ) ), this, SLOT( slotMetaContactSelected( bool ) ) );

	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account*)),
	         this, SLOT(addToAddContactMenu(Kopete::Account*)) );
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered( const Kopete::Account* )),
	         this, SLOT(removeToAddContactMenu(const Kopete::Account*)) );

	d->actionProperties = new KAction( KIcon("user-properties"), i18n( "&Properties" ), ac );
	ac->addAction( "contactProperties", d->actionProperties );
	d->actionProperties->setShortcut( KShortcut(Qt::Key_Alt + Qt::Key_Return) );
	connect( d->actionProperties, SIGNAL(triggered(bool)), this, SLOT(showItemProperties()) );

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

void KopeteContactListView::saveExpandedGroups()
{
	QModelIndex parent = rootIndex();
	d->notExpandedGroups.clear();
	for (int i = 0; i < model()->rowCount( parent ); ++i)
	{
		QModelIndex child = model()->index( i, 0, parent );
		if ( !isExpanded( child ) )
			d->notExpandedGroups << model()->data( child, Kopete::Items::IdRole ).toInt();
	}
}

void KopeteContactListView::expandGroups()
{
	QModelIndex parent = rootIndex();
	for (int i = 0; i < model()->rowCount( parent ); ++i)
	{
		QModelIndex child = model()->index( i, 0, parent );
		int groupId = model()->data( child, Kopete::Items::IdRole ).toInt();
		setExpanded( child, !d->notExpandedGroups.contains( groupId ) );
	}
}

void KopeteContactListView::contactActivated( const QModelIndex& index )
{
	QVariant v = index.data( Kopete::Items::ElementRole );
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		if ( !d->controlPressed )
		{
			Kopete::MetaContact* mc = metaContactFromIndex( index );
			if ( mc )
				mc->execute();
		}
	}


}

void KopeteContactListView::reset()
{
	saveExpandedGroups();
	QTreeView::reset();
	expandGroups();
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
	Kopete::MetaContact* destMetaContact = metaContactFromIndex( currentIndex() );
	if ( !destMetaContact )
		return;

	// Get metaContacts as indexes could change during merge.
	QList<Kopete::MetaContact *> metaContactList;
	foreach ( QModelIndex index, selectedIndexes() )
	{
		Kopete::MetaContact* mc = metaContactFromIndex( index );
		if ( mc && mc != destMetaContact )
			metaContactList.append( mc );
	}

	if ( metaContactList.isEmpty() )
		return;

	Kopete::ContactList::self()->mergeMetaContacts( metaContactList, destMetaContact );
}

void KopeteContactListView::addGroup()
{
	QString groupName = KInputDialog::getText( i18n( "New Group" ),
	                                           i18n( "Please enter the name for the new group:" ) );

	if ( !groupName.isEmpty() )
		Kopete::ContactList::self()->findGroup( groupName );
}

void KopeteContactListView::removeGroupOrMetaContact()
{
	QList<Kopete::MetaContact *> metaContactList;
	QList<Kopete::Group *> groupList;
	QStringList displayNameList;

	foreach ( const QModelIndex& index, selectedIndexes() )
	{
		if ( Kopete::MetaContact* metaContact = metaContactFromIndex( index ) )
		{
			metaContactList.append( metaContact );

			if( !metaContact->displayName().isEmpty() )
				displayNameList.append( metaContact->displayName() );
		}
		else if ( Kopete::Group* group = groupFromIndex( index ) )
		{
			groupList.append( group );

			if( !group->displayName().isEmpty() )
				displayNameList.append( group->displayName() );
		}
	}

	if ( groupList.isEmpty() && metaContactList.isEmpty() )
		return;	

	if( (groupList.count() + metaContactList.count()) == 1 )
	{
		QString msg;
		if( !metaContactList.isEmpty() )
		{
			msg = i18n( "<qt>Are you sure you want to remove the contact <b>%1</b>" \
			            " from your contact list?</qt>" ,
			            metaContactList.first()->displayName() ) ;
		}
		else if( !groupList.isEmpty() )
		{
			msg = i18n( "<qt>Are you sure you want to remove the group <b>%1</b> " \
			            "and all contacts that are contained within it?</qt>" ,
			            groupList.first()->displayName() );
		}

		if( KMessageBox::warningContinueCancel( this, msg, i18n( "Remove" ), KGuiItem( i18n( "Remove" ), "edit-delete" ),
		                                        KStandardGuiItem::cancel(), "askRemovingContactOrGroup" ,
		                                        KMessageBox::Notify | KMessageBox::Dangerous ) != KMessageBox::Continue )
		{
			return;
		}
	}
	else
	{
		QString msg = groupList.isEmpty() ?
			i18n( "Are you sure you want to remove these contacts " \
			      "from your contact list?" ) :
			i18n( "Are you sure you want to remove these groups and " \
			      "contacts from your contact list?" );

		if( KMessageBox::warningContinueCancelList( this, msg, displayNameList, i18n("Remove"), KGuiItem( i18n( "Remove" ), "edit-delete" ),
		                                            KStandardGuiItem::cancel(), "askRemovingContactOrGroup",
		                                            KMessageBox::Notify | KMessageBox::Dangerous ) != KMessageBox::Continue )
		{
			return;
		}
	}

	foreach ( Kopete::MetaContact* metaContact, metaContactList )
		Kopete::ContactList::self()->removeMetaContact( metaContact );

	foreach ( Kopete::Group* group, groupList )
		Kopete::ContactList::self()->removeGroup( group );
}

void KopeteContactListView::startChat()
{
	Kopete::MetaContact* metaContact = metaContactFromIndex( currentIndex() );
	if ( metaContact )
		metaContact->startChat();
}

void KopeteContactListView::sendFile()
{
	Kopete::MetaContact* metaContact = metaContactFromIndex( currentIndex() );
	if ( metaContact )
		metaContact->sendFile( KUrl() );
}

void KopeteContactListView::sendMessage()
{
	if ( Kopete::MetaContact* metaContact = metaContactFromIndex( currentIndex() ) )
		metaContact->sendMessage();
	else if ( Kopete::Group* group = groupFromIndex( currentIndex() ) )
		group->sendMessage();
}

void KopeteContactListView::sendEmail()
{
	Kopete::MetaContact* metaContact = metaContactFromIndex( currentIndex() );
	if ( metaContact )
	{
		KABC::Addressee addressee = KABC::StdAddressBook::self()->findByUid( metaContact->kabcId() );
		if ( !addressee.isEmpty() )
		{
			QString emailAddr = addressee.fullEmail();
			
			kDebug( 14000 ) << "Email: " << emailAddr << "!";
			if ( !emailAddr.isEmpty() )
				KToolInvocation::invokeMailer( emailAddr, QString::null );	//krazy:exclude=nullstrassign for old broken gcc
			else
				KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "There is no email address set for this contact in the KDE address book." ), i18n( "No Email Address in Address Book" ) );
		}
		else
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "This contact was not found in the KDE address book. Check that a contact is selected in the properties dialog." ), i18n( "Not Found in Address Book" ) );
		}
	}
}

void KopeteContactListView::addTemporaryContact()
{
	Kopete::MetaContact* metaContact = metaContactFromIndex( currentIndex() );
	if ( metaContact && metaContact->isTemporary() )
		metaContact->setTemporary( false );
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

void KopeteContactListView::mouseReleaseEvent(QMouseEvent *event)
{
	if ( (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier )
		d->controlPressed = true;

	QTreeView::mouseReleaseEvent( event );
	d->controlPressed = false;
}

void KopeteContactListView::slotSettingsChanged()
{
	if ( Kopete::AppearanceSettings::self()->contactListTreeView() )
	{
		setRootIsDecorated( true );
		setIndentation( 20 );
	}
	else
	{
		setRootIsDecorated( false );
		setIndentation( Kopete::AppearanceSettings::self()->contactListIndentContact() ? 20 : 0 );
	}

// 	if( Kopete::AppearanceSettings::self()->contactListHideVerticalScrollBar() )
// 	{
// 		// This will disable scrollbar auto-hide feature if it's enabled
// 		// and it will call setVScrollBarMode(Auto), so it must precede setScrollHide call
// 		setScrollAutoHide( false );
// 		setScrollHide( true );
// 	}
// 	else
// 	{
// 		// This will disable "always hide scrollbar" optio and call setVScrollBarMode(Auto)
// 		// so it must precede setScrollAutoHide call
// 		setScrollHide(false);
// 		setScrollAutoHide( Kopete::AppearanceSettings::self()->contactListAutoHideVScroll() );
// 	}

// 	setScrollAutoHideTimeout( Kopete::AppearanceSettings::self()->contactListAutoHideTimeout() );
// 	setMouseNavigation( Kopete::BehaviorSettings::self()->contactListMouseNavigation() );

	setAnimated( Kopete::AppearanceSettings::self()->contactListAnimateChange() );
/*	Kopete::UI::ListView::Item::setEffects( Kopete::AppearanceSettings::self()->contactListAnimateChange(),
	                                        Kopete::AppearanceSettings::self()->contactListFading(),
	                                        Kopete::AppearanceSettings::self()->contactListFolding() );*/
	
}

void KopeteContactListView::addToAddContactMenu( Kopete::Account* account )
{
	KAction *action = new KAction( KIcon( QIcon( account->accountIcon() ) ), account->accountLabel(), this );
	connect( action, SIGNAL(triggered(bool)), this, SLOT(addContact()) );
	d->addContactAccountMap.insert( action, account );
	d->actionAddContact->addAction( action );
}

void KopeteContactListView::removeToAddContactMenu( const Kopete::Account *account )
{
	QMapIterator<KAction *, Kopete::Account *> it( d->addContactAccountMap );
	while ( it.hasNext() )
	{
		it.next();
		if ( it.value() == account )
		{
			KAction *action = it.key();
			d->addContactAccountMap.remove( action );
			d->actionAddContact->removeAction( action );
		}
	}
}

void KopeteContactListView::addContact()
{
	if( !sender() )
		return;

	Kopete::MetaContact* metaContact = metaContactFromIndex( currentIndex() );
	Kopete::Group *group = groupFromIndex( currentIndex() );
	Kopete::Account *account = d->addContactAccountMap.value( dynamic_cast<KAction*>( sender() ) );

	if ( (metaContact && metaContact->isTemporary() ) ||
	     (group && group->type() == Kopete::Group::Temporary) )
		return;

	if( !account || !(metaContact || group) )
		return;

	KDialog *addDialog = new KDialog( this );
	addDialog->setCaption( i18n( "Add Contact" ) );
	addDialog->setButtons( KDialog::Ok | KDialog::Cancel );

	AddContactPage *addContactPage = account->protocol()->createAddContactWidget( addDialog, account );

	if ( !addContactPage )
	{
		kDebug(14000) << "Error while creating addcontactpage";
		return;
	}

	addDialog->setMainWidget( addContactPage );
	if( addDialog->exec() == QDialog::Accepted )
	{
		if( addContactPage->validateData() )
		{
			if( !metaContact )
			{
				metaContact = new Kopete::MetaContact();
				metaContact->addToGroup( group );
				if ( addContactPage->apply( account, metaContact ) )
					Kopete::ContactList::self()->addMetaContact( metaContact );
				else
					delete metaContact;
			}
			else
			{
				addContactPage->apply( account, metaContact );
			}
		}
	}
	addDialog->deleteLater();
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
