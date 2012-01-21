/*
    Kopete Contact List View

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Nick Betcher <nbetcher@usinternet.com>
    Copyright (c) 2002      by Stefan Gehn <metz@gehn.net>
    Copyright (c) 2002-2005 by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2004      by Richard Smith <kde@metafoo.co.uk>
    Copyright     2007-2008 by Matt Rogers <mattr@kde.org>
    Copyright     2009      by Roman Jarosz <kedgedev@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers <kopete-devel@kde.org>

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

#include <kopete_export.h>

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
class Account;
}

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KOPETE_CONTACT_LIST_EXPORT KopeteContactListView : public QTreeView
{
	Q_OBJECT

public:
	KopeteContactListView( QWidget *parent = 0 );
	~KopeteContactListView();

	void initActions( KActionCollection *ac );
	virtual void setModel( QAbstractItemModel *newModel );

	int visibleContentHeight() const;

	virtual void keyboardSearch( const QString &search );

public Q_SLOTS:
	virtual void reset();
	void contactActivated( const QModelIndex& index );

	void showItemProperties();
	void mergeMetaContact();
	void addGroup();
	void startChat();
	void sendFile();
	void sendMessage();
	void sendEmail();
        void rename();
	void addTemporaryContact();
	void removeGroupOrMetaContact();
	void moveToGroup();
	void copyToGroup();

Q_SIGNALS:
	void visibleContentHeightChanged();

protected:
	virtual void contextMenuEvent( QContextMenuEvent* event );
	virtual void mouseReleaseEvent( QMouseEvent *event );
	virtual void startDrag( Qt::DropActions supportedActions );
	virtual void dragMoveEvent( QDragMoveEvent *event );
	virtual void timerEvent( QTimerEvent *event );
	virtual bool eventFilter( QObject *object, QEvent *event );
	virtual bool viewportEvent( QEvent *event );

protected slots:
	virtual void rowsInserted( const QModelIndex &parent, int start, int end );
	virtual void selectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

private slots:
	void reexpandGroups();
	void itemExpanded( const QModelIndex& index );
	void itemCollapsed( const QModelIndex& index );

	void updateActions();
	void updateMetaContactActions();
	void slotSettingsChanged();
	void addToAddContactMenu( Kopete::Account* account );
	void removeToAddContactMenu( const Kopete::Account *account );
	void addContact();

private:
	Kopete::MetaContact* metaContactFromIndex( const QModelIndex& index ) const;
	Kopete::Group* groupFromIndex( const QModelIndex& index ) const;

	void groupPopup( Kopete::Group *group, const QPoint& pos );
	void metaContactPopup( Kopete::MetaContact *metaContact, const QPoint& pos );
	void miscPopup( QModelIndexList indexes, const QPoint& pos );
	Kopete::Contact* contactAt( const QPoint& point ) const;
	
	void setScrollAutoHide( bool autoHide );
	void setScrollHide( bool hide );

	int visibleContentHeight( const QModelIndex& parent ) const;

	KopeteContactListViewPrivate *d;
};


#endif
// vim: set noet ts=4 sts=4 sw=4:
