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
#include <QScrollBar>

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
	KopeteContactListViewPrivate()
		: controlPressed( false ),
		  scrollAutoHideTimer(0),
		  scrollAutoHideCounter(10),
		  scrollAutoHideTimeout(10),
		  scrollAutoHide(false),
		  scrollHide(false)
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
	QPointer<Kopete::MetaContact> selectedMetaContact;

	QPointer<Kopete::Contact> pressedContact;
	bool controlPressed;

	//! The timer that will manage scroll bar auto-hiding
	int scrollAutoHideTimer;
	//! Timeout counter for scroll bar auto-hiding
	int scrollAutoHideCounter;
	//! Timeout for scroll bar auto-hiding
	int scrollAutoHideTimeout;
	//! State of scroll auto hide
	bool scrollAutoHide;
	//! State of always hide scrollbar feature
	bool scrollHide;
};

KopeteContactListView::KopeteContactListView( QWidget *parent )
: QTreeView( parent ), d( new KopeteContactListViewPrivate() )
{
	header()->hide();

	setSelectionMode( QAbstractItemView::ExtendedSelection );
	setDragEnabled( true );
	setDragDropMode( QAbstractItemView::DragDrop );
	setAcceptDrops( true );
	setAlternatingRowColors( true );
	setAnimated( true );
	setDropIndicatorShown( true );
	setItemDelegate( new KopeteItemDelegate( this ) );
	setExpandsOnDoubleClick( true );
	setEditTriggers(QAbstractItemView::EditKeyPressed);

	connect( this, SIGNAL(activated(QModelIndex)),
	         this, SLOT(contactActivated(QModelIndex)));
	connect( this, SIGNAL(expanded(QModelIndex)),
	         this, SLOT(itemExpanded(QModelIndex)));
	connect( this, SIGNAL(collapsed(QModelIndex)),
	         this, SLOT(itemCollapsed(QModelIndex)));
	connect( ContactList::LayoutManager::instance(), SIGNAL(activeLayoutChanged()),
	         this, SLOT(reset()) );
	connect( Kopete::BehaviorSettings::self(), SIGNAL(configChanged()), SLOT(slotSettingsChanged()) );
	connect( Kopete::AppearanceSettings::self(), SIGNAL(configChanged()), SLOT(slotSettingsChanged()) );

	setEditTriggers( NoEditTriggers );
	// Load in the user's initial settings
	slotSettingsChanged();
}

KopeteContactListView::~KopeteContactListView()
{
	delete d;
}

void KopeteContactListView::initActions( KActionCollection *ac )
{
// 	d->actionUndo = KStandardAction::undo( this , SLOT(slotUndo()) , ac );
// 	d->actionRedo = KStandardAction::redo( this , SLOT(slotRedo()) , ac );
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

	d->actionMove = new KopeteGroupListAction( i18n( "&Move To" ), QLatin1String( "edit-cut" ),
                                                KShortcut(), this, SLOT(moveToGroup()), ac );
	ac->addAction( "contactMove", d->actionMove );
	d->actionCopy = new KopeteGroupListAction( i18n( "&Copy To" ), QLatin1String( "edit-copy" ),
                                                KShortcut(), this, SLOT(copyToGroup()), ac );
	ac->addAction( "contactCopy", d->actionCopy );

	d->actionMakeMetaContact = new KAction(KIcon("list-add-user"), i18n("Merge Meta Contacts"), ac);
	ac->addAction( "makeMetaContact", d->actionMakeMetaContact );
	connect (d->actionMakeMetaContact, SIGNAL(triggered(bool)), this, SLOT(mergeMetaContact()));

	d->actionRemove = KopeteStdAction::deleteContact( this, SLOT(removeGroupOrMetaContact()), ac );
	ac->addAction( "contactRemove", d->actionRemove );

	d->actionSendEmail = new KAction( KIcon("mail-send"), i18n( "Send Email..." ), ac );
	ac->addAction( "contactSendEmail", d->actionSendEmail );
	connect( d->actionSendEmail, SIGNAL(triggered(bool)), this, SLOT(sendEmail()) );

	d->actionRename = new KAction( KIcon("edit-rename"), i18nc( "verb, rename a contact", "Rename" ), ac );
	d->actionRename->setShortcut( KShortcut(Qt::Key_F2) );
	ac->addAction( "contactRename", d->actionRename );
	connect( d->actionRename, SIGNAL(triggered(bool)), this, SLOT(rename()) );

	d->actionSendFile = KopeteStdAction::sendFile( this, SLOT(sendFile()), ac );
	ac->addAction( "contactSendFile", d->actionSendFile );

	d->actionAddContact = new KActionMenu( KIcon( QLatin1String("list-add-user") ), i18n( "&Add Contact" ), ac );
	ac->addAction( "contactAddContact", d->actionAddContact );
	d->actionAddContact->menu()->addTitle( i18n("Select Account") );

	d->actionAddTemporaryContact = new KAction( KIcon("list-add-user"), i18n( "Add to Your Contact List" ), ac );
	ac->addAction( "contactAddTemporaryContact", d->actionAddTemporaryContact );
	connect( d->actionAddTemporaryContact, SIGNAL(triggered(bool)), this, SLOT(addTemporaryContact()) );

// 	connect( Kopete::ContactList::self(), SIGNAL(metaContactSelected(bool)), this, SLOT(slotMetaContactSelected(bool)) );

	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account*)),
			this, SLOT(addToAddContactMenu(Kopete::Account*)) );
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered(const Kopete::Account*)),
			this, SLOT(removeToAddContactMenu(const Kopete::Account*)) );

	d->actionProperties = new KAction( KIcon("user-properties"), i18n( "&Properties" ), ac );
	ac->addAction( "contactProperties", d->actionProperties );
	d->actionProperties->setShortcut( KShortcut(Qt::Key_Alt + Qt::Key_Return) );
	connect( d->actionProperties, SIGNAL(triggered(bool)), this, SLOT(showItemProperties()) );

	// Update enabled/disabled actions
	//	slotViewSelectionChanged();
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
		if ( !d->controlPressed )
		{
			if ( d->pressedContact )
			{
				d->pressedContact->execute();
			}
			else
			{
				Kopete::MetaContact* mc = metaContactFromIndex( index );
				if ( mc )
					mc->execute();
			}
		}
	}


}

void KopeteContactListView::setModel( QAbstractItemModel *newModel )
{
	if ( model() )
	{
		disconnect( model(), SIGNAL(layoutChanged()), this, SLOT(reexpandGroups()) );
		disconnect( model(), SIGNAL(layoutChanged()), this, SIGNAL(visibleContentHeightChanged()) );
	}

	QTreeView::setModel( newModel );

	// TODO: This is not the best approach as this is emitted often, find a better way.
	connect( newModel, SIGNAL(layoutChanged()), this, SLOT(reexpandGroups()) );
	connect( newModel, SIGNAL(layoutChanged()), this, SIGNAL(visibleContentHeightChanged()) );
}

int KopeteContactListView::visibleContentHeight() const
{
	QModelIndex parent = rootIndex();

	int height = 0;
	int rows = model()->rowCount( parent );
	for ( int i = 0; i < rows; ++i )
		height += visibleContentHeight( model()->index( i, 0, parent ) );

	return height;
}

void KopeteContactListView::keyboardSearch( const QString &search )
{
	// FIXME: Why QTreeView::keyboardSearch updates selection only when selectionMode is SingleSelection?
	setSelectionMode( QAbstractItemView::SingleSelection );
	QTreeView::keyboardSearch( search );
	setSelectionMode( QAbstractItemView::ExtendedSelection );
}

void KopeteContactListView::reset()
{
	QTreeView::reset();

	if ( Kopete::AppearanceSettings::self()->groupContactByGroup() )
		setRootIndex( model()->index( 0, 0 ) );

	reexpandGroups();
	emit visibleContentHeightChanged();
}

void KopeteContactListView::showItemProperties()
{
	QModelIndex index = currentIndex();
	if ( !index.isValid() )
		return;

	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		QPointer <KopeteMetaLVIProps> propsDialog = new KopeteMetaLVIProps( metaContactFromIndex( index ), 0L );
		propsDialog->exec(); // modal
		delete propsDialog;
	}
	else if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		QPointer <KopeteGVIProps> propsDialog = new KopeteGVIProps( groupFromIndex( index ), 0L );
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
			// Can't remove topLevel group
			if ( group == Kopete::Group::topLevel() )
				continue;

			// Can't remove the offline group
			if ( group == Kopete::Group::offline() )
				continue;

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

void KopeteContactListView::moveToGroup()
{
	Q_ASSERT(model());
	QModelIndexList indexList = selectedIndexes();
	if ( indexList.count() != 1 )
		return;

	QModelIndex index = indexList.first();
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact && d->actionMove->currentAction() )
	{
		Kopete::MetaContact* metaContact = metaContactFromIndex( index );
		foreach( Kopete::Contact *c, metaContact->contacts() )
		{
			if ( !c->account()->isConnected() )
			{ // Some accounts are offline we can't move it
				const QString msg = i18n( "Account %1 is offline. Do you really want to move this metacontact?", c->account()->accountLabel() );
				if ( KMessageBox::warningYesNo( this, msg, i18n( "Move contact" ), KStandardGuiItem::yes(), KStandardGuiItem::no(),
				                                "askMoveMetaContactToGroup", KMessageBox::Notify | KMessageBox::Dangerous ) == KMessageBox::No )
					return;
			}
		}

		bool ok = false;
		uint groupId = d->actionMove->currentAction()->data().toUInt( &ok );
		if ( !ok )
			return;

		Kopete::Group *toGroup = Kopete::ContactList::self()->group( groupId );
		if ( !toGroup )
			return;

		QObject* groupObject = qVariantValue<QObject*>( index.data( Kopete::Items::MetaContactGroupRole ) );
		Kopete::Group* fromGroup = qobject_cast<Kopete::Group*>(groupObject);

		// Can't move to the offline group manually
		if ( toGroup == Kopete::Group::offline() )
		    return;

		metaContact->moveToGroup( fromGroup, toGroup );
	}
}

void KopeteContactListView::copyToGroup()
{
	Q_ASSERT(model());
	QModelIndexList indexList = selectedIndexes();
	if ( indexList.count() != 1 )
		return;

	QModelIndex index = indexList.first();
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact && d->actionCopy->currentAction() )
	{
		Kopete::MetaContact* metaContact = metaContactFromIndex( index );
		foreach( Kopete::Contact *c, metaContact->contacts() )
		{
			if ( !c->account()->isConnected() )
				return; // Some contact is offline we can't copy it
		}

		bool ok = false;
		uint groupId = d->actionCopy->currentAction()->data().toUInt( &ok );
		if ( !ok )
			return;

		Kopete::Group *toGroup = Kopete::ContactList::self()->group( groupId );
		if ( !toGroup )
			return;

		// Can't copy to the offline group manually
		if ( toGroup == Kopete::Group::offline() )
		    return;

		metaContact->addToGroup( toGroup );
	}
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
				KToolInvocation::invokeMailer( emailAddr, QString() );
			else
				KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "There is no email address set for this contact in the KDE address book." ), i18n( "No Email Address in Address Book" ) );
		}
		else
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "This contact was not found in the KDE address book. Check that a contact is selected in the properties dialog." ), i18n( "Not Found in Address Book" ) );
		}
	}
}

void KopeteContactListView::rename()
{
	Kopete::MetaContact* metaContact = metaContactFromIndex( currentIndex() );
	if ( metaContact )
	{
		edit(currentIndex());
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
		{
			Kopete::Contact* contact = contactAt( event->pos() );
			if ( contact )
			{
				KMenu *menu = contact->popupMenu();
				connect( menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()) );
				menu->popup( event->globalPos() );
			}
			else
			{
				metaContactPopup( metaContactFromIndex( index ), event->globalPos() );
			}
		}
		else if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
			groupPopup( groupFromIndex( index ), event->globalPos() );
	}
	event->accept();
}

void KopeteContactListView::mouseReleaseEvent(QMouseEvent *event)
{
	if ( (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier )
		d->controlPressed = true;

	d->pressedContact = contactAt( event->pos() );

	QTreeView::mouseReleaseEvent( event );
	d->pressedContact = 0;
	d->controlPressed = false;
}

void KopeteContactListView::startDrag( Qt::DropActions supportedActions )
{
	QModelIndexList indexes = selectedIndexes();
	for ( int i = indexes.count() - 1 ; i >= 0; --i )
	{
		if ( !(model()->flags( indexes.at(i) ) & Qt::ItemIsDragEnabled) )
			indexes.removeAt(i);
	}

	if (indexes.count() > 0)
	{
		QMimeData *data = model()->mimeData( indexes );
		if ( !data )
			return;

		QDrag *drag = new QDrag( this );
		drag->setMimeData( data );

		Qt::DropAction defaultDropAction = Qt::MoveAction;
		drag->exec( supportedActions, defaultDropAction );
	}
}

void KopeteContactListView::dragMoveEvent ( QDragMoveEvent * event )
{
	const QMimeData *data = event->mimeData();

	QTreeView::dragMoveEvent ( event );
	if ( !event->isAccepted() )
		return;

	QModelIndex index;
	switch ( dropIndicatorPosition() )
	{
	case QAbstractItemView::AboveItem:
	case QAbstractItemView::BelowItem:
		index = indexAt( event->pos() ).parent();
		break;
	case QAbstractItemView::OnItem:
		index = indexAt( event->pos() );
		break;
	case QAbstractItemView::OnViewport:
		index = rootIndex();
		break;
	default: // Should not happen
		event->ignore();
		return;
	}
	
	Kopete::AppearanceSettings* as = Kopete::AppearanceSettings::self();
	bool groupContactByGroup = as->groupContactByGroup();

	bool accept = false;
	if ( data->hasFormat( "application/kopete.metacontacts.list" ) )
	{
		if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
			accept = (event->proposedAction() & Qt::MoveAction); // MetaContact merge (copy&merge not supported)
		else if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
			accept = (event->proposedAction() & (Qt::MoveAction | Qt::CopyAction)); // Move/copy between groups
		else if ( !groupContactByGroup && !index.isValid() )
			accept = (event->proposedAction() & Qt::MoveAction); // In plain view metaContact can only be moved
		else
			accept = false;
	}
	else if ( data->hasFormat( "application/kopete.group" ) )
	{
		if ( !groupContactByGroup )
			accept = false; // Plain view doesn't support groups
		else if ( !index.parent().isValid() )
			accept = (event->proposedAction() & Qt::MoveAction); // In tree view groups can only be moved
		else
			accept = false;
	}
	else
		accept = true; // Accept by default (e.g. urls)

	if ( !accept )
	{
		event->ignore();
		return;
	}

	event->acceptProposedAction();
}

void KopeteContactListView::timerEvent( QTimerEvent *event )
{
	QTreeView::timerEvent( event );

	if ( event->timerId() == d->scrollAutoHideTimer )
	{
		if ( !d->scrollAutoHideCounter-- )
			setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	}
}

bool KopeteContactListView::eventFilter( QObject *object, QEvent *event )
{
	if ( d->scrollAutoHide && object == verticalScrollBar() )
	{
		if ( event->type() == QEvent::MouseMove )
			d->scrollAutoHideCounter = 9999;
		else if ( event->type() == QEvent::Enter )
			d->scrollAutoHideCounter = 9999;
		else if ( event->type() == QEvent::Leave )
			d->scrollAutoHideCounter = d->scrollAutoHideTimeout;
	}
	else if ( d->scrollAutoHide && object == viewport() )
	{
		if ( event->type() == QEvent::MouseMove )
		{
			setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
			d->scrollAutoHideCounter = 9999;	// Mouse is on the contact list, so don't hide it
		}
		else if ( event->type() == QEvent::Leave )
		{
			d->scrollAutoHideCounter = d->scrollAutoHideTimeout; // Mouse left the contact list, hide it after timeout
		}
	}

	return QTreeView::eventFilter( object, event );
}

bool KopeteContactListView::viewportEvent( QEvent *event )
{
	// FIXME: Fix for crash, easily reproducible by assigning shortcut to show offline users action
	// and holding down that action and moving mouse over contact list.
	// Is this a Qt bug or are we using invalidate() in ContactListProxyModel wrongly?
	// The same crash was in KTorrent bug 172198.
	executeDelayedItemsLayout();
	return QTreeView::viewportEvent( event );
}

void KopeteContactListView::rowsInserted( const QModelIndex &parent, int start, int end )
{
	QTreeView::rowsInserted( parent, start, end );

	const int delta = end - start + 1;
	for (int i = 0; i < delta; ++i)
	{
		QModelIndex index = model()->index( i + start, 0, parent );
		if ( index.isValid() && index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
			setExpanded( index , index.data( Kopete::Items::ExpandStateRole ).toBool() );
	}
}

void KopeteContactListView::selectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
	QTreeView::selectionChanged( selected, deselected );

	QSet<Kopete::MetaContact*> contacts;
	QSet<Kopete::Group*> groups;

	foreach ( const QModelIndex& index, selected.indexes() )
	{
		if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
			contacts.insert( metaContactFromIndex( index ) );
		else if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
			groups.insert( groupFromIndex( index ) );
	}

	Kopete::ContactList::self()->setSelectedItems( contacts.toList(), groups.toList() );

	updateActions();

	if ( d->selectedMetaContact )
	{ // Delete previous connection
		disconnect( d->selectedMetaContact, SIGNAL(onlineStatusChanged(Kopete::MetaContact*,Kopete::OnlineStatus::StatusType)),
		            this, SLOT(updateMetaContactActions()) );
		d->selectedMetaContact = 0;
	}

	if ( contacts.count() == 1 && groups.empty() )
	{
		d->selectedMetaContact = contacts.values().first();
		connect( d->selectedMetaContact, SIGNAL(onlineStatusChanged(Kopete::MetaContact*,Kopete::OnlineStatus::StatusType)),
		         this, SLOT(updateMetaContactActions()) );
	}

	updateMetaContactActions();
}

void KopeteContactListView::reexpandGroups()
{
	// Set expanded state of groups
	QModelIndex parent = rootIndex();
	for (int i = 0; i < model()->rowCount( parent ); ++i)
	{
		QModelIndex index = model()->index( i, 0, parent );
		if ( index.isValid() && index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
			setExpanded( index, model()->data( index, Kopete::Items::ExpandStateRole ).toBool() );
	}
}

void KopeteContactListView::itemExpanded( const QModelIndex& index )
{
	Q_ASSERT(model());
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
		model()->setData( index, true, Kopete::Items::ExpandStateRole );
}

void KopeteContactListView::itemCollapsed( const QModelIndex& index )
{
	Q_ASSERT(model());
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
		model()->setData( index, false, Kopete::Items::ExpandStateRole );
}

void KopeteContactListView::updateActions()
{
	QModelIndexList selected = selectedIndexes();

	bool singleContactSelected = (selected.count() == 1 && selected.first().data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact);
	bool singleGroupSelected = (selected.count() == 1 && selected.first().data( Kopete::Items::TypeRole ) == Kopete::Items::Group);
	Kopete::MetaContact* metaContact = ( singleContactSelected ) ? metaContactFromIndex( selected.first() ) : 0;
	Kopete::Group* group = ( singleGroupSelected ) ? groupFromIndex( selected.first() ) : 0;

	bool inkabc = false;
	if ( singleContactSelected )
	{
		QString kabcid = metaContact->kabcId();
		inkabc = !kabcid.isEmpty() && !kabcid.contains(":");
	}
	d->actionSendEmail->setEnabled( inkabc );

	if ( singleContactSelected )
	{
		d->actionRename->setText( i18n("Rename Contact") );
		d->actionRemove->setText( i18n("Remove Contact") );
		d->actionSendMessage->setText( i18n("Send Single Message...") );
		d->actionRename->setEnabled( true );
		d->actionRemove->setEnabled( true );
		d->actionAddContact->setText( i18n("&Add Subcontact") );
		d->actionAddContact->setEnabled( !metaContact->isTemporary() );
	}
	else if ( singleGroupSelected )
	{
		d->actionRename->setText( i18n("Rename Group") );
		d->actionRemove->setText( i18n("Remove Group") );
		d->actionSendMessage->setText( i18n("Send Message to Group") );
		d->actionRename->setEnabled( true );
		d->actionRemove->setEnabled( true );
		d->actionSendMessage->setEnabled( true );
		d->actionAddContact->setText( i18n("&Add Contact to Group") );
		d->actionAddContact->setEnabled( group->type() == Kopete::Group::Normal );
	}
	else
	{
		d->actionRename->setText( i18n("Rename") );
		d->actionRemove->setText( i18n("Remove") );
		d->actionRename->setEnabled( false );
		d->actionRemove->setEnabled( !selected.isEmpty() );
		d->actionAddContact->setEnabled( false );
		d->actionMakeMetaContact->setText( i18n("Make Meta Contact") );

		bool hasContactInSelection = false;
		foreach ( const QModelIndex& index, selected )
		{
			if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
			{
				hasContactInSelection = true;
				break;
			}
		}
		d->actionMakeMetaContact->setEnabled( hasContactInSelection ); // Specifically for multiple contacts, not groups.
	}

	d->actionProperties->setEnabled( selected.count() == 1 );
}

void KopeteContactListView::updateMetaContactActions()
{
	bool reachable = false;

	if( d->selectedMetaContact )
	{
		reachable = d->selectedMetaContact->isReachable();
		d->actionAddTemporaryContact->setEnabled( d->selectedMetaContact->isTemporary() );
		d->actionSendFile->setEnabled( reachable && d->selectedMetaContact->canAcceptFiles() );
		d->actionSendMessage->setEnabled( reachable );
		d->actionStartChat->setEnabled( reachable );
	}
	else
	{
		QModelIndexList selected = selectedIndexes();
		bool singleGroupSelected = (selected.count() == 1 && selected.first().data( Kopete::Items::TypeRole ) == Kopete::Items::Group);
		
		d->actionAddTemporaryContact->setEnabled( false );
		d->actionSendFile->setEnabled( false );
		d->actionSendMessage->setEnabled( singleGroupSelected );
		d->actionStartChat->setEnabled( false );
	}

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

	if( Kopete::AppearanceSettings::self()->contactListHideVerticalScrollBar() )
	{
		setScrollAutoHide( false );
		setScrollHide( true );
	}
	else
	{
		setScrollHide( false );
		setScrollAutoHide( Kopete::AppearanceSettings::self()->contactListAutoHideVScroll() );
	}

	d->scrollAutoHideTimeout = Kopete::AppearanceSettings::self()->contactListAutoHideTimeout();

	//FIXME: I don't see it in any UI
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

	QPointer <KDialog> addDialog = new KDialog( this );
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
	if ( addDialog )
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

	if ( group == Kopete::Group::offline() )
	    return;

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
			connect( popup, SIGNAL(aboutToHide()), contactMenu, SLOT(deleteLater()) );
			QString text = i18nc( "Translators: format: '<displayName> (<id>)'", "%2 <%1>", c->contactId(), c->nickName() );
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

Kopete::Contact* KopeteContactListView::contactAt( const QPoint& point ) const
{
	QModelIndex index = indexAt( point );
	if ( !index.isValid() )
		return 0;

	QRect rect = visualRect( index );
	if ( rect.width() <= 0 || rect.height() <= 0 )
		return 0;

	KopeteItemDelegate* delegate = dynamic_cast<KopeteItemDelegate*>(itemDelegate( index ));
	if ( !delegate )
		return 0;

	QStyleOptionViewItem option = viewOptions();
	option.rect = rect;
	return delegate->contactAt( option, index, point );
}


void KopeteContactListView::setScrollAutoHide( bool autoHide )
{
	if ( d->scrollAutoHide == autoHide )
		return;

	if ( autoHide )
	{
		viewport()->installEventFilter( this );
		viewport()->setMouseTracking( true );
		verticalScrollBar()->installEventFilter( this );
		verticalScrollBar()->setMouseTracking( true );

		// Set scrollbar auto-hiding state true
		d->scrollAutoHide = true;
		// Turn of the bar now
		setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
		// Start the timer to handle auto-hide
		killTimer( d->scrollAutoHideTimer );
		d->scrollAutoHideTimer = startTimer( 1000 );
	}
	else
	{
		viewport()->removeEventFilter( this );
		viewport()->setMouseTracking( false );
		verticalScrollBar()->removeEventFilter( this );
		verticalScrollBar()->setMouseTracking( false );

		d->scrollAutoHide = false;
		setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
		killTimer( d->scrollAutoHideTimer );
	}
}

void KopeteContactListView::setScrollHide( bool hide )
{
	if ( d->scrollHide == hide )
		return;

	d->scrollHide = hide;
	if ( hide )
		setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	else
		setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
}

int KopeteContactListView::visibleContentHeight( const QModelIndex& parent ) const
{
	if ( !parent.isValid() )
		return 0;

	int height = rowHeight( parent );
	if ( height <= 0 ) // Assume that items is invisible
		return 0;

	int rows = model()->rowCount( parent );
	for ( int i = 0; i < rows; ++i )
		height += visibleContentHeight( model()->index( i, 0, parent ) );

	return height;
}

#include "kopetecontactlistview.moc"

// vim: set noet ts=4 sts=4 sw=4:
