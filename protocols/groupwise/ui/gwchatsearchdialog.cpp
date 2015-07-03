/*
    Kopete Groupwise Protocol
    gwchatsearchdialog.cpp - dialog for searching for chatrooms

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2005      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwchatsearchdialog.h"
#include <qmap.h>

#include <QTreeWidget>
#include <ktreewidgetsearchline.h>

#include <kpushbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include "client.h"
#include "chatroommanager.h"

#include "gwaccount.h"
#include "gwprotocol.h"
#include "gwchatpropsdialog.h"

GroupWiseChatSearchDialog::GroupWiseChatSearchDialog( GroupWiseAccount * account, QWidget *parent, const char * /*name*/ )
	: KDialog(  parent),
					m_account( account )
{
	QWidget * wid = new QWidget( this );
	m_ui.setupUi( wid );
	setMainWidget( wid );
	setCaption(i18n( "Search Chatrooms" ));
	setButtons(KDialog::Ok|KDialog::Apply|KDialog::Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);
//	m_ui.searchLineWidget->createSearchLine( m_ui.chatrooms );

	m_manager = m_account->client()->chatroomManager();
	
	connect ( m_manager, SIGNAL(updated()), SLOT(slotManagerUpdated()) );
	connect ( m_manager, SIGNAL(gotProperties(GroupWise::Chatroom)),
			  SLOT(slotGotProperties(GroupWise::Chatroom)) );

	connect( m_ui.btnRefresh, SIGNAL(clicked()), SLOT(slotUpdateClicked()) );
	connect( m_ui.btnProperties, SIGNAL(clicked()), SLOT(slotPropertiesClicked()) );

	m_manager->updateRooms();
	show();
}

GroupWiseChatSearchDialog::~GroupWiseChatSearchDialog()
{
}

void GroupWiseChatSearchDialog::slotUpdateClicked()
{
	kDebug () << "updating chatroom list ";
	QStringList updateMessage;
	QTreeWidgetItem * first = m_ui.chatrooms->topLevelItem(0);
	updateMessage.append(i18n("Updating chatroom list..." ));
	if ( first )
		new QTreeWidgetItem(first, updateMessage);
	else
		new QTreeWidgetItem( m_ui.chatrooms, updateMessage );
	m_manager->updateRooms();

}

void GroupWiseChatSearchDialog::slotManagerUpdated()
{
	m_ui.chatrooms->clear();
	ChatroomMap rooms = m_manager->rooms();
	ChatroomMap::iterator it = rooms.begin();
	const ChatroomMap::iterator end = rooms.end();
	while ( it != end )
	{
		QStringList argsList;
		// Populate the String List
		argsList.append(it.value().displayName);
		argsList.append(m_account->protocol()->dnToDotted(it.value().ownerDN));
		argsList.append(QString::number(it.value().participantsCount));

		new QTreeWidgetItem( m_ui.chatrooms, argsList);
		++it;
	}
}

void GroupWiseChatSearchDialog::slotPropertiesClicked()
{
	QTreeWidgetItem * selected  = m_ui.chatrooms->currentItem();
	if ( selected )
	{
		m_manager->requestProperties( selected->text( 0 ) );
	}
}

void GroupWiseChatSearchDialog::slotGotProperties(const GroupWise::Chatroom & room)
{
	kDebug() ;
	new GroupWiseChatPropsDialog( room, true, this );
}

#include "gwchatsearchdialog.moc"
