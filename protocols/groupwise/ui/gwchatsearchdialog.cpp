/*
    Kopete Groupwise Protocol
    gwchatsearchdialog.cpp - dialog for searching for chatrooms

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

#include <qmap.h>

#include <klistview.h>
#include <klistviewsearchline.h>

#include <kpushbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include "client.h"
#include "chatroommanager.h"

#include "gwaccount.h"
#include "gwprotocol.h"
#include "gwchatsearchwidget.h"
#include "gwchatpropsdialog.h"

#include "gwchatsearchdialog.h"

GroupWiseChatSearchDialog::GroupWiseChatSearchDialog( GroupWiseAccount * account, QWidget *parent, const char *name )
	: KDialogBase(  parent, name, false, i18n( "Search Chatrooms" ),
					KDialogBase::Ok|KDialogBase::Apply|KDialogBase::Cancel, Ok, true ), m_account( account )
{
	m_widget = new GroupWiseChatSearchWidget( this );
//	m_widget->m_searchLineWidget->createSearchLine( m_widget->m_chatrooms );
	setMainWidget( m_widget );

	m_manager = m_account->client()->chatroomManager();
	
	connect ( m_manager, SIGNAL( updated() ), SLOT( slotManagerUpdated() ) );
	connect ( m_manager, SIGNAL( gotProperties( const GroupWise::Chatroom & ) ),
			  SLOT( slotGotProperties( const GroupWise::Chatroom & ) ) );

	connect( m_widget->m_btnRefresh, SIGNAL( clicked() ), SLOT( slotUpdateClicked() ) );
	connect( m_widget->m_btnProperties, SIGNAL( clicked() ), SLOT( slotPropertiesClicked() ) );

	m_manager->updateRooms();
	show();
}

GroupWiseChatSearchDialog::~GroupWiseChatSearchDialog()
{
}

void GroupWiseChatSearchDialog::slotUpdateClicked()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << "updating chatroom list " << endl;
	m_widget->m_chatrooms->clear();
	QListViewItem * first = m_widget->m_chatrooms->firstChild();
	QString updateMessage = i18n("Updating chatroom list..." );
/*	if ( first )
		new QListViewItem( first, updateMessage );
	else*/
		new QListViewItem( m_widget->m_chatrooms, updateMessage );
	m_manager->updateRooms();

}

void GroupWiseChatSearchDialog::slotManagerUpdated()
{
	ChatroomMap rooms = m_manager->rooms();
	ChatroomMap::iterator it = rooms.begin();
	const ChatroomMap::iterator end = rooms.end();
	while ( it != end )
	{
		new QListViewItem( m_widget->m_chatrooms,
						   it.data().displayName,
						   m_account->protocol()->dnToDotted( it.data().ownerDN ),
						   QString::number( it.data().participantsCount ) );
		++it;
	}
}

void GroupWiseChatSearchDialog::slotPropertiesClicked()
{
	QListViewItem * selected  = m_widget->m_chatrooms->selectedItem();
	if ( selected )
	{
		m_manager->requestProperties( selected->text( 0 ) );
	}
}

void GroupWiseChatSearchDialog::slotGotProperties(const GroupWise::Chatroom & room)
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	new GroupWiseChatPropsDialog( room, true, this, "chatpropsdlg" );
}

#include "gwchatsearchdialog.moc"
