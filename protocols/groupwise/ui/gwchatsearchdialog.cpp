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

#include <klistview.h>
#include <qpushbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include "client.h"
#include "gwaccount.h"
#include "gwprotocol.h"
#include "gwchatsearchwidget.h"
#include "tasks/searchchattask.h"
#include "gwchatsearchdialog.h"

GroupWiseChatSearchDialog::GroupWiseChatSearchDialog( GroupWiseAccount * account, QWidget *parent, const char *name )
	: KDialogBase(  parent, name, false, i18n( "Search Chatrooms" ),
					KDialogBase::Ok|KDialogBase::Apply|KDialogBase::Cancel, Ok, true ), m_account( account )
{
	m_widget = new GroupWiseChatSearchWidget( this );
	setMainWidget( m_widget );

	populateWidget();

	slotUpdate();
	
	connect( m_widget->m_btnRefresh, SIGNAL( clicked() ), SLOT( slotUpdate() ) );

	show();
}

GroupWiseChatSearchDialog::~GroupWiseChatSearchDialog()
{
}

void GroupWiseChatSearchDialog::populateWidget()
{
	QValueList<ChatroomSearchResult> results = m_account->chatrooms();
	QValueListIterator<ChatroomSearchResult> it = results.begin();
	QValueListIterator<ChatroomSearchResult> end = results.end();
	while ( it != end )
	{
		new QListViewItem( m_widget->m_chatrooms,
						   (*it).name,
						   m_account->protocol()->dnToDotted( (*it).owner ),
						   QString::number( (*it).participants ) );
		++it;
	}
}

void GroupWiseChatSearchDialog::slotUpdate()
{
	kdDebug() << "updating chatroom list " << endl;
	QListViewItem * first = m_widget->m_chatrooms->firstChild();
	if ( first )
		new QListViewItem( first, i18n("Updating chatroom list..." ) );
	else
		new QListViewItem( m_widget->m_chatrooms, i18n("Updating chatroom list..." ) );
	SearchChatTask * sct = new SearchChatTask( m_account->client()->rootTask() );
	sct->search( m_widget->m_chatrooms->childCount() ? SearchChatTask::SinceLastSearch : SearchChatTask::FetchAll );
	connect( sct, SIGNAL( finished() ), SLOT( slotGotSearchResults() ) );
	sct->go( true );
}

void GroupWiseChatSearchDialog::slotGotSearchResults()
{
	kdDebug() << k_funcinfo << endl;
	SearchChatTask * sct = (SearchChatTask *)sender();
	QValueList<ChatroomSearchResult> rooms = m_account->chatrooms();
	rooms += sct->results();
	m_account->setChatrooms( rooms );
	m_widget->m_chatrooms->clear();
	populateWidget();
}


#include "gwchatsearchdialog.moc"
