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

KopeteContact::KopeteContact( const QString &protocolId, KopeteMetaContact *parent )
	: QObject( parent )
{
	connect(this, SIGNAL(incomingEvent(KopeteEvent *)), kopeteapp, SLOT(notifyEvent(KopeteEvent *)));

	m_metaContact = parent;
	m_protocolId = protocolId;
	m_cachedSize = 0;
	m_cachedOldStatus = Offline;
	
	/* Initialize the context Menu */
	initActions();
}

KopeteContact::~KopeteContact()
{
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

void KopeteContact::slotChangeMetaContact(){
	// TODO:Actually make this (mETz), see header file for description of
	// what it should do.

}

void KopeteContact::showContextMenu(const QPoint& p, const QString& group){
	/* Build the menu */	
	contextMenu = new KPopupMenu();
	contextMenu->insertTitle( QString("%1 <%2> (%3)").arg(displayName()).arg(id()).arg(statusText()) ); // Name (status)
	actionSendMessage->plug( contextMenu );
	actionViewHistory->plug( contextMenu );
	contextMenu->insertSeparator();
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
}

void KopeteContact::slotChangeDisplayName(){
	bool okClicked;
	QString newName = KLineEditDlg::getText(i18n("Change Alias"), i18n("New alias for %1").arg(id()),
											 displayName(), &okClicked);
	if(okClicked){
		setDisplayName( newName );
	}	
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

