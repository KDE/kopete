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
#include <qregexp.h>
#include <qvbox.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <klineeditdlg.h>
#include <kiconeffect.h>

#include "kopetecontactlist.h"
#include "kopetehistorydialog.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetestdaction.h"
#include "kopeteviewmanager.h"

KopeteContact::KopeteContact( KopeteProtocol *protocol, const QString &contactId, KopeteMetaContact *parent )
: QObject( parent )
{
//	kdDebug() << "KopeteContact::KopeteContact: Creating contact with id " << contactId << endl;

	m_contactId = contactId;

	m_metaContact = parent;
	m_protocol = protocol;
	m_cachedSize = 0;
	m_cachedOldImportance = 0;
	contextMenu = 0L;
	mFileCapable = false;
	m_historyDialog = 0L;
	m_idleState = Unspecified;
	m_displayName = contactId;

	if( protocol )
	{
		protocol->registerContact( this );
		connect( protocol, SIGNAL( unloading() ), SLOT( slotProtocolUnloading() ) );
	}

	// Initialize the context menu
	actionSendMessage = KopeteStdAction::sendMessage( this,
		SLOT( sendMessage() ), this, "actionSendMessage" );
	actionChat = KopeteStdAction::chat( this,
		SLOT( startChat() ), this, "actionChat" );
	actionViewHistory = KopeteStdAction::viewHistory( this,
		SLOT( slotViewHistory() ), this, "actionViewHistory" );
	actionChangeMetaContact = KopeteStdAction::changeMetaContact( this,
		SLOT( slotChangeMetaContact() ), this, "actionChangeMetaContact" );
	actionDeleteContact = KopeteStdAction::deleteContact( this,
		SLOT( slotDeleteContact() ), this, "actionDeleteContact" );
	actionUserInfo = KopeteStdAction::contactInfo( this,
		SLOT( slotUserInfo() ), this, "actionUserInfo" );
	actionChangeAlias = KopeteStdAction::changeAlias( this,
		SLOT( slotChangeDisplayName() ), this, "actionChangeAlias" );
	actionSendFile = KopeteStdAction::sendFile( this,
		SLOT( sendFile() ), this, "actionSendFile");

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
}

void KopeteContact::slotProtocolUnloading()
{
	delete this;
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
	if( name == m_displayName )
		return;

	m_displayName = name;
	emit displayNameChanged( name );
}

QString KopeteContact::displayName() const
{
	return m_displayName;
}

KopeteContact::ContactStatus KopeteContact::status() const
{
	return Online;
}

QString KopeteContact::statusText() const
{
	ContactStatus stat = status();

	//kdDebug(14010) << "[KopeteContact] statusText() with status= " << stat << endl;

	switch( stat )
	{
	case Online:
		return i18n("Online");
	case Away:
		return i18n("Away");
	case Unknown:
		return i18n("Status not available");
	case Offline:
	default:
		return i18n("Offline");
	}
}

QString KopeteContact::statusIcon() const
{
	return QString::fromLatin1( "unknown" );
}

QPixmap KopeteContact::scaledStatusIcon(int size)
{
	if ( (importance() != m_cachedOldImportance) || ( size != m_cachedSize ) )
	{
		QImage afScal = ((QPixmap(SmallIcon(statusIcon()))).convertToImage()).smoothScale( size, size );
		m_cachedScaledIcon = QPixmap(afScal);
		m_cachedOldImportance = importance();
		m_cachedSize = size;
	}
	if ( m_idleState == Idle )
	{
		QPixmap tmp = m_cachedScaledIcon;
		KIconEffect::semiTransparent(tmp);
		return tmp;
	}
	return m_cachedScaledIcon;
}

int KopeteContact::importance() const
{
	ContactStatus stat = status();

	if (stat == Online)
		return 20;

	if (stat == Away)
		return 10;

	if (stat == Offline)
		return 0;

	return 0;
}

void KopeteContact::slotViewHistory()
{
	kdDebug(14010) << "KopteContact::slotViewHistory()" << endl;

	if( m_historyDialog )
	{
		m_historyDialog->raise();
	}
	else
	{
		m_historyDialog = new KopeteHistoryDialog(this,
			true, 50, qApp->mainWidget(), "KopeteHistoryDialog" );

		connect ( m_historyDialog, SIGNAL( destroyed()),
			this, SLOT( slotHistoryDialogDestroyed() ) );
	}
}

void KopeteContact::slotHistoryDialogDestroyed()
{
	m_historyDialog = 0L;
}

void KopeteContact::sendFile( const KURL &, const QString &, const long unsigned int)
{
	kdDebug(14010) << "[KopeteContact] Opps, the plugin hasn't implemented file sending, yet it was turned on! :(" << endl;
}

KPopupMenu* KopeteContact::createContextMenu()
{
	//FIXME: this should perhaps be KActionCollection * KopeteContact::contactActions()
	//FIXME: to avoid passing around KPopupMenu's

	// Build the menu
	KPopupMenu *menu = new KPopupMenu();

	menu->insertTitle( QString::fromLatin1("%1 <%2> (%3)").arg(displayName()).arg(contactId()).arg(statusText()) );


	actionSendMessage->plug( menu );
	actionSendMessage->setEnabled( isReachable() );
	actionChat->plug( menu );
	actionChat->setEnabled( isReachable() );

	actionViewHistory->plug( menu );

	menu->insertSeparator();

	actionChangeMetaContact->setEnabled( !m_metaContact->isTemporary() );
	actionChangeMetaContact->plug( menu );
	actionUserInfo->plug( menu );
	actionChangeAlias->plug( menu );
	actionDeleteContact->plug( menu );

	if (mFileCapable)
		actionSendFile->plug( menu );

	// Protocol specific options will go below this separator
	// through the use of the customContextMenuActions() function

	// Get the custom actions from the protocols (pure virtual function)
	KActionCollection *customActions = customContextMenuActions();
	if(customActions != 0L)
	{
		if ( !customActions->isEmpty() )
			menu->insertSeparator();

		for(unsigned int i = 0; i < customActions->count(); i++)
		{
			customActions->action(i)->plug( menu );
		}
	}

	return menu;
}

KPopupMenu *KopeteContact::popupMenu()
{
	contextMenu = createContextMenu();
	return contextMenu;
}

void KopeteContact::showContextMenu(const QPoint& p)
{
	contextMenu = createContextMenu();
	contextMenu->exec( p );
	delete contextMenu;
	contextMenu = 0L;
}

void KopeteContact::slotChangeDisplayName(){
	bool okClicked;
	QString newName = KLineEditDlg::getText(i18n("Change Alias"), i18n("New alias for %1").arg(contactId()),
		displayName(), &okClicked);

	if(okClicked)
		setDisplayName( newName );
}

void KopeteContact::slotChangeMetaContact()
{
	KDialogBase *moveDialog= new KDialogBase( qApp->mainWidget(), "moveDialog" , true, i18n("Move Contact") , KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )  ;
	QVBox *w = new QVBox(moveDialog);
	w->setSpacing( 8 );
	new QLabel(i18n("Choose the meta contact into which you want to move this contact.") , w);
	m_selectMetaContactListBox = new KListView ( w , "m_selectMetaContactListBox");
	m_selectMetaContactListBox->addColumn( i18n("Display Name") );
	m_selectMetaContactListBox->addColumn( i18n("Contact IDs") );

	QPtrList<KopeteMetaContact> metaContacts = KopeteContactList::contactList()->metaContacts();
	for( KopeteMetaContact *mc = metaContacts.first(); mc ; mc = metaContacts.next() )
	{
		if(!mc->isTemporary())
			new MetaContactListViewItem(mc , m_selectMetaContactListBox  ) ;
	}

	m_selectMetaContactListBox->sort();

	moveDialog->setMainWidget(w);
	connect( moveDialog, SIGNAL( okClicked()) , this, SLOT( slotMoveDialogOkClicked() ) );
	moveDialog->show();
}

void KopeteContact::slotMoveDialogOkClicked()
{
	KopeteMetaContact *mc= static_cast<MetaContactListViewItem*>( m_selectMetaContactListBox->currentItem() ) ->metaContact;
	if(!mc)
	{
		kdDebug(14010) << "KopeteContact::slotMoveDialogOkClicked : metaContact not found, will create a new one" << endl;
		return;
	}
	setMetaContact( mc );
}

void KopeteContact::setMetaContact( KopeteMetaContact *m )
{
	KopeteMetaContact *old = m_metaContact;
	m_metaContact->removeContact( this );
	m->addContact( this );

	KopeteGroupList newGroups = m->groups();
	KopeteGroupList oldGroups = m_metaContact->groups();
	KopeteGroupList currentGroups = groups();

	// Reparent the contact
	m_metaContact->removeChild( this );
	m->insertChild( this );
	m_metaContact = m;

	// Reconnect signals to the new meta contact
	disconnect( old, SIGNAL( aboutToSave( KopeteMetaContact * ) ),
		protocol(), SLOT( slotMetaContactAboutToSave( KopeteMetaContact * ) ) );
	connect( m_metaContact, SIGNAL( aboutToSave( KopeteMetaContact * ) ),
		protocol(), SLOT( slotMetaContactAboutToSave( KopeteMetaContact * ) ) );

	// Sync groups
	for( KopeteGroup *group = newGroups.first(); group; group = newGroups.next() )
	{
		if( !currentGroups.contains( group ) )
			addToGroup( group );
	}

	for( KopeteGroup *group = oldGroups.first(); group; group = oldGroups.next() )
	{
		if( currentGroups.contains( group ) && !newGroups.contains( group ) )
			removeFromGroup( group );
	}

	//the aboutToSave signal is disconnected from "old" , but it still contains
	//cached data in it, then, i serialize the old contact for removing old data
	protocol()->slotMetaContactAboutToSave(old);

	if( old->contacts().isEmpty() )
	{
		//Delete an empty MC after a move
		KopeteContactList::contactList()->removeMetaContact( old );
	}
}

KopeteContact::MetaContactListViewItem::MetaContactListViewItem(KopeteMetaContact *m, QListView *p)
		:QListViewItem(p)
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
	m_idleState = newState;
	emit idleStateChanged( this, m_idleState );
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
	KopeteViewManager::viewManager()->launchWindow( manager(true), KopeteView::Chat);
}

void KopeteContact::sendMessage()
{
	KopeteViewManager::viewManager()->launchWindow( manager(true), KopeteView::Email);
}

void KopeteContact::execute()
{
	// FIXME: Implement, don't hardcode startChat()!
	startChat();
}

KopeteMessageManager *KopeteContact::manager( bool )
{
	kdDebug(14010) << "Manager() not implimented for " << protocol()->displayName() << ", crash!" << endl;
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

#include "kopetecontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

