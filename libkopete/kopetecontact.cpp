/*
    kopetecontact.cpp - Kopete Contact

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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

KopeteContact::KopeteContact( KopeteProtocol *protocol, const QString &contactId,
	KopeteMetaContact *parent ) : QObject( parent )
{
	m_contactId = contactId;

	m_metaContact = parent;
	m_protocol = protocol;
	m_cachedSize = 0;
	m_cachedOldStatus = Unknown;
	contextMenu = 0L;
	mFileCapable = false;
	m_historyDialog = 0L;
	m_idleState = Unspecified;

	if( protocol )
		protocol->registerContact( this );

	connect( protocol, SIGNAL( unloading() ), SLOT( slotProtocolUnloading() ) );

	// Initialize the context menu
	actionSendMessage = KopeteStdAction::sendMessage( this,
		SLOT( execute() ), this, "actionSendMessage" );
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
		
	//Need to check this because ourself has no parent
	if( parent )
		parent->addContact( this );
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

void KopeteContact::setDisplayName( const QString &name )
{
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
		return i18n("Status not avaliable");
	case Offline:
	default:
		return i18n("Offline");
	}
}

QString KopeteContact::statusIcon() const
{
	return "unknown";
}

QPixmap KopeteContact::scaledStatusIcon(int size)
{
	if ( (this->status() != m_cachedOldStatus) || ( size != m_cachedSize ) )
	{
		QImage afScal = ((QPixmap(SmallIcon(this->statusIcon()))).convertToImage()).smoothScale( size, size );
		m_cachedScaledIcon = QPixmap(afScal);
		m_cachedOldStatus = this->status();
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
	//FIXME: to avoid passing aorund KPopupMenu's

	/* Build the menu */
	KPopupMenu *menu = new KPopupMenu();
	menu->insertTitle( displayName()+" <"+contactId()+"> ("+statusText()+")" );

	actionSendMessage->plug( menu );
	actionSendMessage->setEnabled( isReachable() );

	actionViewHistory->plug( menu );

	menu->insertSeparator();

	actionChangeMetaContact->setEnabled( !m_metaContact->isTemporary() );
	actionChangeMetaContact->plug( menu );
	actionUserInfo->plug( menu );
	actionChangeAlias->plug( menu );
	actionDeleteContact->plug( menu );

	if (mFileCapable)
		actionSendFile->plug( menu );

	/* Protocol specific options will go below this separator
	 * through the use of the customContextMenuActions() function
	 */
	/* Get the custom actions from the protocols (pure virtual function) */
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

void KopeteContact::showContextMenu(const QPoint& p)
{
	contextMenu=createContextMenu();
	contextMenu->exec( p );
	delete contextMenu;
	contextMenu = 0L;
}

void KopeteContact::slotChangeDisplayName(){
	bool okClicked;
	QString newName = KLineEditDlg::getText(i18n("Change Alias"), i18n("New alias for %1").arg(contactId()),
											 displayName(), &okClicked);
	if(okClicked){
		setDisplayName( newName );
	}
}

void KopeteContact::slotChangeMetaContact()
{
	KDialogBase *moveDialog= new KDialogBase( qApp->mainWidget(), "moveDialog" , true, i18n("Move Contact") , KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )  ;
	QVBox *w=new QVBox(moveDialog);
	new QLabel(i18n("Choose the meta contact into which you want to move this contact.") , w);
	m_selectMetaContactListBox= new KListBox ( w , "m_selectMetaContactListBox");

	QPtrList<KopeteMetaContact> metaContacts = KopeteContactList::contactList()->metaContacts();
	for( 	KopeteMetaContact *mc = metaContacts.first(); mc ; mc = metaContacts.next() )
	{
		if(!mc->isTemporary())
			new MetaContactListBoxItem(mc , m_selectMetaContactListBox  ) ;
	}

	moveDialog->setMainWidget(w);
	connect( moveDialog, SIGNAL( okClicked()) , this, SLOT( slotMoveDialogOkClicked() ) );
	moveDialog->show();
}

void KopeteContact::slotMoveDialogOkClicked()
{
	KopeteMetaContact *mc= static_cast<MetaContactListBoxItem*>(m_selectMetaContactListBox->item(m_selectMetaContactListBox->currentItem())) ->metaContact;
	if(!mc)
	{
		kdDebug(14010) << "KopeteContact::slotMoveDialogOkClicked : WARNING metaContact not found" << endl;
		return;
	}
	moveToMetaContact(mc);
}

void KopeteContact::moveToMetaContact(KopeteMetaContact *m)
{
	KopeteMetaContact *old=m_metaContact;
	m_metaContact->removeContact(this);
	m->addContact(this);

/*	KopeteGroupList groups_new=m->groups();
	KopeteGroupList groups_old=m_metaContact->groups();
	KopeteGroupList groups_current=groups();*/

	m_metaContact->removeChild(this);
	m->insertChild(this);
	m_metaContact=m;

/*	for( KopeteGroupList::ConstIterator it = groups_new.begin(); it != groups_new.end(); ++it )
	{
		KopeteGroup group=*it;
		if(!groups_current.contains(group))
			addToGroup(group);
	}
	for( KopeteGroupList::ConstIterator it = groups_old.begin(); it != groups_old.end(); ++it )
	{
		KopeteGroup group=*it;
		if(groups_current.contains(group) && !groups_new.contains(group))
			removeFromGroup(group);
	}*/

	emit moved(old, this);
}

KopeteContact::MetaContactListBoxItem::MetaContactListBoxItem(KopeteMetaContact *m, QListBox *p)
		:QListBoxText(p)
{
	metaContact=m;
	QString t=m->displayName();
	bool f=true;

	QPtrList<KopeteContact> contacts = metaContact->contacts();
	for( KopeteContact *c = contacts.first(); c ; c = contacts.next() )
	{
		if(f) t+=" [";
		else t+=" ; ";
		t+=c->contactId();
		f=false;
	}
	if(!f) t+="]";

	setText(t);
}

QString KopeteContact::contactId() const
{
	return m_contactId;
}

void KopeteContact::setIdleState( KopeteContact::IdleState newState )
{
	m_idleState = newState;
	emit idleStateChanged( this, m_idleState );
}

#include "kopetecontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

