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

#include <qimage.h>
#include <qpixmap.h>
#include <qvbox.h>
#include <qlabel.h>

#include <kdebug.h>
#include <kiconloader.h>
//#include <klistview.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <klineeditdlg.h>

#include "kopete.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "kopetecontactlist.h"
#include "kopeteprotocol.h"
#include "kopetecontactlistview.h"

KopeteContact::KopeteContact( KopeteProtocol *protocol, KopeteMetaContact *parent )
	: QObject( parent )
{
	connect(this, SIGNAL(incomingEvent(KopeteEvent *)), kopeteapp, SLOT(notifyEvent(KopeteEvent *)));

	m_metaContact = parent;
	m_protocol = protocol;
	m_cachedSize = 0;
	m_cachedOldStatus = Unknown;
	contextMenu = 0L;
	mFileCapable = false;

	connect(protocol, SIGNAL(destroyed()), this, SLOT(slotProtocolUnloading()));
	
	/* Initialize the context Menu */
	initActions();
}

KopeteContact::~KopeteContact()
{
//	if(contextMenu != 0L)
/*	delete contextMenu;
	delete actionSendMessage;
	delete actionDeleteContact;
	delete actionChangeMetaContact;
	delete actionViewHistory;
	delete actionChangeAlias;
	delete actionUserInfo;
	delete actionSendFile;*/
	
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

	//kdDebug() << "[KopeteContact] statusText() with status= " << stat << endl;

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
		return m_cachedScaledIcon;
	}
	else
	{
		return m_cachedScaledIcon;
	}
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


void KopeteContact::initActions()
{
	/* Build the actions */
	actionSendMessage = KopeteStdAction::sendMessage(this, SLOT(execute()), this, "actionMessage" ); // "Send Message"
	actionViewHistory = KopeteStdAction::viewHistory( this, SLOT(slotViewHistory()), this, "actionViewHistory" );  // "View History"
	actionChangeMetaContact = KopeteStdAction::changeMetaContact( this, SLOT(slotChangeMetaContact()), this, "actionChangeMetaContact" ); // "Change MetaContact"
	actionDeleteContact = KopeteStdAction::deleteContact( this, SLOT(slotDeleteContact()), this, "actionDeleteContact" );
	actionUserInfo = KopeteStdAction::contactInfo( this, SLOT(slotUserInfo()), this, "actionUserInfo" );
	actionChangeAlias = KopeteStdAction::changeAlias( this, SLOT(slotChangeDisplayName()), this, "actionChangeAlias" );
	actionSendFile = KopeteStdAction::sendFile( this, SLOT(slotSendFile()), this, "actionSendFile");
}

void KopeteContact::slotSendFile()
{
	kdDebug() << "[KopeteContact] Opps, the plugin hasn't implemented file sending, yet it was turned on! :(" << endl;
}

KPopupMenu* KopeteContact::createContextMenu()
{
	//FIXME: this should perhaps be KActionCollection * KopeteContact::contactActions()
	//FIXME: to avoid passing aorund KPopupMenu's

	/* Build the menu */
	KPopupMenu *menu = new KPopupMenu();
	menu->insertTitle( displayName()+" <"+id()+"> ("+statusText()+")" );

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
	QString newName = KLineEditDlg::getText(i18n("Change Alias"), i18n("New alias for %1").arg(id()),
											 displayName(), &okClicked);
	if(okClicked){
		setDisplayName( newName );
	}
}

void KopeteContact::addThisTemporaryContact(KopeteGroup *group)
{
	if(m_metaContact->isTemporary())
		m_metaContact->setTemporary(false,group);
}

void KopeteContact::slotChangeMetaContact()
{
	KDialogBase *moveDialog= new KDialogBase(kopeteapp->contactList(), "moveDialog" , true, i18n("Move Contact") , KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )  ;
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
		kdDebug() << "KopeteContact::slotMoveDialogOkClicked : WARNING metaContact not found" << endl;
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
	for( 	KopeteContact *c = contacts.first(); c ; c = contacts.next() )
	{
		if(f) t+=" [";
		else t+=" ; ";
		t+=c->id();
		f=false;
	}
	if(!f) t+="]";

	setText(t);
}

#include "kopetecontact.moc"

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

