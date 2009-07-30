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

#ifndef CHATMEMBERSLISTVIEW_H
#define CHATMEMBERSLISTVIEW_H

#include <QListView>

#include <qmap.h>

namespace Kopete
{
class ChatSession;
class Contact;
class OnlineStatus;
class PropertyContainer;
}

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class ChatMembersListView : public QListView
{
	Q_OBJECT
public:
	explicit ChatMembersListView( QWidget *parent = 0);
	virtual ~ChatMembersListView();

public slots:
	void slotContextMenuRequested( const QPoint & pos );
private:
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

