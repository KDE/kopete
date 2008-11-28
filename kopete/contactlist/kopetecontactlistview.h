/*
    Kopete Contact List View

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Nick Betcher <nbetcher@usinternet.com>
    Copyright (c) 2002      by Stefan Gehn <metz@gehn.net>
    Copyright (c) 2002-2005 by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2004      by Richard Smith <kde@metafoo.co.uk>
    Copyright     2007-2008 by Matt Rogers <mattr@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_CONTACTLISTVIEW_H
#define KOPETE_CONTACTLISTVIEW_H

#include <QTreeView>

#include <QPixmap>
#include <QList>
#include <QStringList>
#include <QRect>
#include <QTimer>
#include <QPointer>
#include <QMouseEvent>
#include <QDropEvent>

class KopeteMetaContactLVI;
class KopeteGroupViewItem;
class KActionCollection;
class KAction;
class KSelectAction;
class KActionMenu;

class KopeteContactListViewPrivate;

namespace Kopete
{
class Contact;
class MetaContact;
class Group;
class MessageEvent;
}

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteContactListView : public QTreeView
{
	Q_OBJECT

public:
	KopeteContactListView( QWidget *parent = 0 );
	~KopeteContactListView();


public Q_SLOTS:
    void contactActivated( const QModelIndex& index );
	void itemExpanded( const QModelIndex& index );
	void itemCollapsed( const QModelIndex& index );

private:
	Kopete::MetaContact* metaContactFromIndex( const QModelIndex& index );

private:
	typedef QList<KopeteMetaContactLVI*> MetaContactLVIList;
	typedef QList<KopeteGroupViewItem*> GroupViewItemList;

	QRect m_onItem;

	/* ACTIONS */
	KAction *actionSendMessage;
	KAction *actionStartChat;
	KAction *actionSendFile;
	KAction *actionSendEmail;
	KSelectAction *actionMove;
	KSelectAction *actionCopy;
	KAction *actionRename;
	KAction *actionRemove;
	KAction *actionAddTemporaryContact;
	KAction *actionProperties;
	KAction *actionUndo;
	KAction *actionRedo;
	KAction *actionMakeMetaContact;

	KopeteContactListViewPrivate *d;

};


#endif
// vim: set noet ts=4 sts=4 sw=4:
