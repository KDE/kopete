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
#include <klistview.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <klineeditdlg.h>

#include "kopete.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"

KopeteContact::KopeteContact( const QString &protocolId, KopeteMetaContact *parent )
	: QObject( parent )
{
	connect(this, SIGNAL(incomingEvent(KopeteEvent *)), kopeteapp, SLOT(notifyEvent(KopeteEvent *)));

	m_metaContact = parent;
	m_protocolId = protocolId;
	m_cachedSize = 0;
	m_cachedOldStatus = Offline;
	contextMenu = 0L;
	
	/* Initialize the context Menu */
	initActions();
}

KopeteContact::~KopeteContact()
{
//	if(contextMenu != 0L)
	delete contextMenu;
	delete actionSendMessage;
	delete actionDeleteContact;
	delete actionChangeMetaContact;
	delete actionViewHistory;
	delete actionChangeAlias;
	delete actionUserInfo;
	
	emit( contactDestroyed( this ) );
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

	kdDebug() << "[KopeteContact] importance() with status= " << stat << endl;

	if (stat == Online)
		return 20;

	if (stat == Away)
		return 10;

	if (stat == Offline)
		return 0;

	return 0;
}

QStringList KopeteContact::groups()
{
	return QStringList();
}

void KopeteContact::addToGroup( const QString & /* group */ )
{
	kdDebug() << "KopeteContact::addToGroup: WARNING: "
		<< "Default implementation called! Function not implemented?" << endl;
}

void KopeteContact::removeFromGroup( const QString & /* group */ )
{
	kdDebug() << "KopeteContact::removeFromGroup: WARNING: "
		<< "Default implementation called! Function not implemented?" << endl;
}

void KopeteContact::moveToGroup( const QString & /* from */,
	const QString & /* to */ )
{
	kdDebug() << "KopeteContact::moveToGroup: WARNING: "
		<< "Default implementation called! Function not implemented?" << endl;
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
}

void KopeteContact::showContextMenu(const QPoint& p)
{
	/* Build the menu */	
	contextMenu = new KPopupMenu();
	contextMenu->insertTitle( displayName()+" <"+id()+"> ("+statusText()+")" );   
	actionSendMessage->plug( contextMenu );
	actionViewHistory->plug( contextMenu );
	contextMenu->insertSeparator();
	actionChangeMetaContact->setEnabled( !m_metaContact->isTemporary() );
	actionChangeMetaContact->plug( contextMenu );
	actionUserInfo->plug( contextMenu );
	actionChangeAlias->plug( contextMenu );
	actionDeleteContact->plug( contextMenu );
	contextMenu->insertSeparator();
	/* Protocol specific options will go below this separator
	 * through the use of the customContextMenuActions() function
	 */
	/* Get the custom actions from the protocols (pure virtual function) */
	KActionCollection *customActions = customContextMenuActions();
	if(customActions != 0L){
		for(unsigned int i = 0; i < customActions->count(); i++){
			customActions->action(i)->plug( contextMenu );
		}
	}
	
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

void KopeteContact::addThisTemporaryContact(QString group)
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
	m_metaContact->removeContact(this);
	m->addContact(this);

	QStringList groups_new=m->groups();
	QStringList groups_old=m_metaContact->groups();
	QStringList groups_current=groups();

	for( QStringList::ConstIterator it = groups_new.begin(); it != groups_new.end(); ++it )
	{
		QString group=*it;
		if(!groups_current.contains(group))
			addToGroup(group);
	}
	for( QStringList::ConstIterator it = groups_old.begin(); it != groups_old.end(); ++it )
	{
		QString group=*it;
		if(groups_current.contains(group) && !groups_new.contains(group))
			removeFromGroup(group);
	}

	m_metaContact->removeChild(this);
	m->insertChild(this);
	m_metaContact=m;

	//TODO: connect this signal in each protocol for uptade the KopeteMetaContact map
	emit moved(this);
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

