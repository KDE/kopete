/*
    kopetecontact.cpp - Kopete Contact

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
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

#include "kopetecontact.h"

#include <qapplication.h>
#include <qimage.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qvbox.h>

#include <kdebug.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klistview.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "kopetecontactlist.h"
#include "kopetehistorydialog.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetestdaction.h"
#include "kopeteviewmanager.h"

// FIXME: What are these doing here and why are they #defines and not const ints? - Martijn
#define EMAIL_WINDOW 0
#define CHAT_WINDOW 1

class KopeteContactPrivate
{
public:
	KopeteHistoryDialog *historyDialog;
	QString displayName;
	bool fileCapable;
	int conversations;
	KopeteProtocol *protocol;

	QPixmap cachedScaledIcon;
	int cachedSize;
	int cachedOldImportance;

	KopeteContact::IdleState idleState;
	KopeteContact::OnlineStatus onlineStatus;

	KopeteMetaContact *metaContact;

	KAction *actionSendMessage;
	KAction *actionChat;
	KAction *actionDeleteContact;
	KAction *actionChangeMetaContact;
	KAction *actionViewHistory;
	KAction *actionChangeAlias;
	KAction *actionUserInfo;
	KAction *actionSendFile;
	KAction *actionAddContact;

	KListView *selectMetaContactListBox;

	KPopupMenu *contextMenu;

	QString contactId;
};

// Used in slotChangeMetaContact()
// FIXME: How about reusing KopeteMetaContactLVI ? - Martijn
class MetaContactListViewItem : public QListViewItem
{
public:
	KopeteMetaContact *metaContact;
	MetaContactListViewItem( KopeteMetaContact *m, QListView *p );
};

KopeteContact::KopeteContact( KopeteProtocol *protocol, const QString &contactId, KopeteMetaContact *parent )
: QObject( parent )
{
	d = new KopeteContactPrivate;

	//kdDebug() << k_funcinfo << "Creating contact with id " << contactId << endl;

	d->contactId = contactId;
	d->onlineStatus = Offline;

	d->metaContact = parent;
	d->protocol = protocol;
	d->cachedSize = 0;
	d->cachedOldImportance = 0;
	d->contextMenu = 0L;
	d->fileCapable = false;
	d->conversations = 0;
	d->historyDialog = 0L;
	d->idleState = Unspecified;
	d->displayName = contactId;

	if( protocol )
	{
		protocol->registerContact( this );
		connect( protocol, SIGNAL( unloading() ), SLOT( slotProtocolUnloading() ) );
	}

	// Initialize the context menu
	d->actionChat        = KopeteStdAction::chat( this,        SLOT( startChat() ),             this, "actionChat" );
	d->actionSendFile    = KopeteStdAction::sendFile( this,    SLOT( sendFile() ),              this, "actionSendFile" );
	d->actionUserInfo    = KopeteStdAction::contactInfo( this, SLOT( slotUserInfo() ),          this, "actionUserInfo" );
	d->actionSendMessage = KopeteStdAction::sendMessage( this, SLOT( sendMessage() ),           this, "actionSendMessage" );
	d->actionViewHistory = KopeteStdAction::viewHistory( this, SLOT( slotViewHistory() ),       this, "actionViewHistory" );
	d->actionChangeAlias = KopeteStdAction::changeAlias( this, SLOT( slotChangeDisplayName() ), this, "actionChangeAlias" );
	d->actionDeleteContact = KopeteStdAction::deleteContact( this, SLOT( slotDeleteContact() ), this, "actionDeleteContact" );
	d->actionChangeMetaContact = KopeteStdAction::changeMetaContact( this, SLOT( slotChangeMetaContact() ), this, "actionChangeMetaContact" );
	d->actionAddContact = new KAction( i18n("&Add Contact"), QString::fromLatin1( "bookmark_add" ),0, this, SLOT( slotAddContact() ), this, "actionAddContact" );

	// Need to check this because myself() has no parent
	if( parent )
	{
		connect( parent, SIGNAL( aboutToSave( KopeteMetaContact * ) ),
			protocol, SLOT( slotMetaContactAboutToSave( KopeteMetaContact * ) ) );

		parent->addContact( this );
	}
}

KopeteContact::~KopeteContact()
{
	emit( contactDestroyed( this ) );

	delete d;
}

void KopeteContact::slotProtocolUnloading()
{
	delete this;
}

void KopeteContact::setConversations( int value ) const
{
	d->conversations = value;
}

const int KopeteContact::conversations() const
{
	return d->conversations;
}

QString KopeteContact::identityId() const
{
	return QString::null;
}

void KopeteContact::rename( const QString &name )
{
	// Default implementation immediately sets the new name
	setDisplayName( name );
}

void KopeteContact::setDisplayName( const QString &name )
{
	if( name == d->displayName )
		return;

	d->displayName = name;
	emit displayNameChanged( name );
}

QString KopeteContact::displayName() const
{
	return d->displayName;
}

KopeteContact::OnlineStatus KopeteContact::onlineStatus() const
{
	return d->onlineStatus;
}

void KopeteContact::setOnlineStatus( KopeteContact::OnlineStatus status )
{
	if( status == d->onlineStatus )
		return;

	d->onlineStatus = status;

	emit onlineStatusChanged( this, status );
}

QString KopeteContact::statusText() const
{
	OnlineStatus stat = onlineStatus();

	//kdDebug( 14010 ) << k_funcinfo << "onlineStatus=" << stat << endl;

	switch( stat )
	{
	case Online:
		return i18n( "Online" );
	case Away:
		return i18n( "Away" );
	case Unknown:
		return i18n( "Status not available" );
	case Offline:
	default:
		return i18n( "Offline" );
	}
}

QString KopeteContact::statusIcon() const
{
	return QString::fromLatin1( "unknown" );
}

QPixmap KopeteContact::scaledStatusIcon( int size )
{
	if ( ( importance() != d->cachedOldImportance ) || ( size != d->cachedSize ) )
	{
		QImage afScal = ( ( QPixmap( SmallIcon( statusIcon() ) ) ).convertToImage() ).smoothScale( size, size );
		d->cachedScaledIcon = QPixmap( afScal );
		d->cachedOldImportance = importance();
		d->cachedSize = size;
	}
	if ( d->idleState == Idle )
	{
		QPixmap tmp = d->cachedScaledIcon;
		KIconEffect::semiTransparent( tmp );
		return tmp;
	}
	return d->cachedScaledIcon;
}

int KopeteContact::importance() const
{
	OnlineStatus stat = onlineStatus();

	if( stat == Online )
		return 20;

	if( stat == Away )
		return 10;

	if( stat == Offline )
		return 0;

	return 0;
}

void KopeteContact::slotViewHistory()
{
	kdDebug( 14010 ) << k_funcinfo << endl;

	if( d->historyDialog )
	{
		d->historyDialog->raise();
	}
	else
	{
		d->historyDialog = new KopeteHistoryDialog( this, true, 50, qApp->mainWidget(), "KopeteHistoryDialog" );

		connect ( d->historyDialog, SIGNAL( destroyed() ), SLOT( slotHistoryDialogDestroyed() ) );
	}
}

void KopeteContact::slotHistoryDialogDestroyed()
{
	d->historyDialog = 0L;
}

void KopeteContact::sendFile( const KURL & /* sourceURL */, const QString & /* fileName */, uint /* fileSize */ )
{
	kdWarning( 14010 ) << k_funcinfo << "Plugin " << protocol()->pluginId() << " has enabled file sending, "
		<< "but didn't implement it!" << endl;
}

void KopeteContact::slotAddContact()
{
	if( metaContact() )
	{
		metaContact()->setTemporary( false );
		KopeteContactList::contactList()->addMetaContact( metaContact() );
	}
}

KPopupMenu* KopeteContact::createContextMenu()
{
	//FIXME: This should perhaps be KActionCollection * KopeteContact::contactActions() to
	//FIXME: 	avoid passing around KPopupMenu's
	//FIXME: (Jason) - KActionCollections are bad for popup menus because they are unordered..
	//FIXME: 	in fact, I think customContextMenuActions should be remade into a popupmenu,
	//FIXME:	or a QPtrList<KAction>, or something that has a notion of order, because
	//FIXME:	currently the customContextMenuActions do not return in the order they are
	//FIXME:	added, which makes for a mess when you want certain things at the top and
	//FIXME:	others later on

	// Build the menu
	KPopupMenu *menu = new KPopupMenu();

	QString titleText;
	if( displayName() == contactId() )
		titleText = QString::fromLatin1( "%1 (%2)" ).arg( displayName() ).arg( statusText() );
	else
		titleText = QString::fromLatin1( "%1 <%2> (%3)" ).arg( displayName() ).arg( contactId() ).arg( statusText() );
	menu->insertTitle( titleText );

	if( metaContact() && metaContact()->isTemporary() )
	{
		d->actionAddContact->plug( menu );
		menu->insertSeparator();
	}

	d->actionSendMessage->plug( menu );
	d->actionChat->plug( menu );
	if( d->fileCapable )
		d->actionSendFile->plug( menu );
	d->actionViewHistory->plug( menu );

	// Protocol specific options will go below this separator
	// through the use of the customContextMenuActions() function

	// Get the custom actions from the protocols ( pure virtual function )
	KActionCollection *customActions = customContextMenuActions();
	if( customActions != 0L )
	{
		if ( !customActions->isEmpty() )
			menu->insertSeparator();

		for( uint i = 0; i < customActions->count(); i++ )
		{
			customActions->action( i )->plug( menu );
		}
	}

	menu->insertSeparator();
	if( metaContact() && !metaContact()->isTemporary() )
		d->actionChangeMetaContact->plug( menu );

	d->actionUserInfo->plug( menu );

	if( metaContact() && !metaContact()->isTemporary() )
	{
		d->actionChangeAlias->plug( menu );
		d->actionDeleteContact->plug( menu );
	}

	return menu;
}

KPopupMenu *KopeteContact::popupMenu()
{
	if( d->contextMenu )
		delete d->contextMenu;

	d->contextMenu = createContextMenu();
	return d->contextMenu;
}

void KopeteContact::showContextMenu( const QPoint& p )
{
	popupMenu()->exec( p );
	delete d->contextMenu;
	d->contextMenu = 0L;
}

void KopeteContact::slotChangeDisplayName(){
	bool okClicked;
	QString newName = KLineEditDlg::getText( i18n( "Change Alias" ), i18n( "New alias for %1:" ).arg( contactId() ),
		displayName(), &okClicked );

	if( okClicked )
		setDisplayName( newName );
}

void KopeteContact::slotChangeMetaContact()
{
	KDialogBase *moveDialog= new KDialogBase( qApp->mainWidget(), "moveDialog", true, i18n( "Move Contact" ),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );
	QVBox *w = new QVBox( moveDialog );
	w->setSpacing( 8 );
	new QLabel( i18n( "Choose the meta contact into which you want to move this contact." ), w );
	d->selectMetaContactListBox = new KListView ( w, "selectMetaContactListBox" );
	d->selectMetaContactListBox->addColumn( i18n( "Display Name" ) );
	d->selectMetaContactListBox->addColumn( i18n( "Contact IDs" ) );

	QPtrList<KopeteMetaContact> metaContacts = KopeteContactList::contactList()->metaContacts();
	for( KopeteMetaContact *mc = metaContacts.first(); mc ; mc = metaContacts.next() )
	{
		if( !mc->isTemporary() )
			new MetaContactListViewItem( mc, d->selectMetaContactListBox  ) ;
	}

	d->selectMetaContactListBox->sort();

	moveDialog->setMainWidget( w );
	connect( moveDialog, SIGNAL( okClicked() ), this, SLOT( slotMoveDialogOkClicked() ) );
	moveDialog->show();
}

void KopeteContact::slotMoveDialogOkClicked()
{
	KopeteMetaContact *mc = static_cast<MetaContactListViewItem*>( d->selectMetaContactListBox->currentItem() )->metaContact;
	if( !mc )
	{
		kdDebug( 14010 ) << "KopeteContact::slotMoveDialogOkClicked : metaContact not found, will create a new one" << endl;
		return;
	}
	KopeteMetaContact *old = d->metaContact;
	setMetaContact( mc );
	KopeteContactPtrList children = old->contacts();
	if( children.isEmpty() )
		KopeteContactList::contactList()->removeMetaContact( old );
}

void KopeteContact::setMetaContact( KopeteMetaContact *m )
{
	kdDebug( 14010 ) << k_funcinfo << endl;

	KopeteMetaContact *old = d->metaContact;
	KopeteGroupList newGroups;
	if( m )
		newGroups = m->groups();

	if( old )
	{
		d->metaContact->removeContact( this );
		KopeteGroupList oldGroups = d->metaContact->groups();
		disconnect( old, SIGNAL( aboutToSave( KopeteMetaContact * ) ),
			protocol(), SLOT( slotMetaContactAboutToSave( KopeteMetaContact * ) ) );

		for( KopeteGroup *group = oldGroups.first(); group; group = oldGroups.next() )
		{
			if( oldGroups.contains( group ) && !newGroups.contains( group ) )
				removeFromGroup( group );
		}

		if( !old->contacts().isEmpty() )
			protocol()->slotMetaContactAboutToSave( old );
	}

	// Reparent the contact
	d->metaContact->removeChild( this );
	d->metaContact = m;

	if( m )
	{
		m->addContact( this );
		m->insertChild( this );

		connect( d->metaContact, SIGNAL( aboutToSave( KopeteMetaContact * ) ),
		protocol(), SLOT( slotMetaContactAboutToSave( KopeteMetaContact * ) ) );

		// Sync groups
		for( KopeteGroup *group = newGroups.first(); group; group = newGroups.next() )
		{
			if( !groups().contains( group ) )
				addToGroup( group );
		}
	}
}

MetaContactListViewItem::MetaContactListViewItem( KopeteMetaContact *m, QListView *p )
: QListViewItem( p )
{
	metaContact=m;
	setText( 0, m->displayName() );

	QString t;
	bool f=true;

	QPtrList<KopeteContact> contacts = metaContact->contacts();
	for( KopeteContact *c = contacts.first(); c ; c = contacts.next() )
	{
		if( f )
			t += QString::fromLatin1( "[ " );
		else
			t += QString::fromLatin1( " ; " );

		t += c->contactId();
		f = false;
	}
	if( !f )
		t += QString::fromLatin1( " ]" );

	setText( 1, t );
}

void KopeteContact::serialize( QMap<QString, QString> & /*serializedData */,
	QMap<QString, QString> & /* addressBookData */ )
{
	// Do nothing in the default implementation
}

void KopeteContact::setIdleState( KopeteContact::IdleState newState )
{
	d->idleState = newState;
	emit idleStateChanged( this, d->idleState );
}

void KopeteContact::addToGroup( KopeteGroup * /* newGroup */ )
{
	/* Default implementation does nothing */
}

void KopeteContact::removeFromGroup( KopeteGroup * /* group */ )
{
	/* Default implementation does nothing */
}

bool KopeteContact::isReachable()
{
	return false;
}

void KopeteContact::startChat()
{
	KopeteViewManager::viewManager()->launchWindow( manager( true ), KopeteMessage::Chat );
}

void KopeteContact::sendMessage()
{
	KopeteViewManager::viewManager()->launchWindow( manager( true ), KopeteMessage::Email );
}

void KopeteContact::execute()
{
	switch( KopetePrefs::prefs()->interfacePreference() )
	{
	case EMAIL_WINDOW:
		sendMessage();
		break;

	case CHAT_WINDOW:
	default:
		startChat();
		break;
	}
}

KopeteMessageManager *KopeteContact::manager( bool )
{
	kdDebug( 14010 ) << "Manager() not implimented for " << protocol()->displayName() << ", crash!" << endl;
	return 0L;
}

void KopeteContact::slotDeleteContact()
{
	/* Default implementation does nothing */
}

void KopeteContact::slotUserInfo()
{
	/* Default implementation does nothing */
}

void KopeteContact::moveToGroup( KopeteGroup * /* from */, KopeteGroup * /* to */ )
{
	/* Default implementation does nothing */
}

KopeteGroupList KopeteContact::groups() const
{
	/* Default implementation does nothing */
	return KopeteGroupList();
}

bool KopeteContact::isOnline() const
{
	return onlineStatus() != Offline && onlineStatus() != Unknown;
}

KopeteMetaContact * KopeteContact::metaContact() const
{
	return d->metaContact;
}

QString KopeteContact::contactId() const
{
	return d->contactId;
}

KopeteProtocol * KopeteContact::protocol() const
{
	return d->protocol;
}

KActionCollection * KopeteContact::customContextMenuActions()
{
	return 0L;
};

bool KopeteContact::isFileCapable() const
{
	return d->fileCapable;
}

void KopeteContact::setFileCapable( bool filecap )
{
	d->fileCapable = filecap;
}

bool KopeteContact::canAcceptFiles() const
{
	return isOnline() && d->fileCapable;
}

KopeteContact::IdleState KopeteContact::idleState() const
{
	return d->idleState;
}

#include "kopetecontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

