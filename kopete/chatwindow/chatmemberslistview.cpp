/*
    ChatMembersListView

    Copyright (c) 2007 by Duncan Mac-Vicar Prett <duncan@kde.org>
   
    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "chatmemberslistview.h"

#include <QToolTip>
#include <QHelpEvent>
#include <QDrag>
#include <QMimeData>

#include "kdebug.h"
#include "kmenu.h"
#include "kopetecontact.h"
#include "chatsessionmemberslistmodel.h"

using namespace Kopete;

ChatMembersListView::ChatMembersListView( QWidget *parent )
	 : QListView( parent )
{
	setContextMenuPolicy (Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenuRequested(QPoint)));
}

void ChatMembersListView::slotContextMenuRequested( const QPoint & pos )
{
	kDebug() << "context menu requested";
	QModelIndex index = indexAt(pos);
	if ( model() )
	{
		ChatSessionMembersListModel *membermodel = dynamic_cast<ChatSessionMembersListModel *>(model());
		if ( membermodel )
		{
			Kopete::Contact *c = membermodel->contactAt(index);
			
			if (!c)
				return;
	
			KMenu *p = c->popupMenu();
			connect( p, SIGNAL(aboutToHide()), p, SLOT(deleteLater()) );
			p->popup( mapToGlobal(pos) );
		}
	}
}

ChatMembersListView::~ChatMembersListView()
{
}

#include "chatmemberslistview.moc"

// vim: set noet ts=4 sts=4 sw=4:

