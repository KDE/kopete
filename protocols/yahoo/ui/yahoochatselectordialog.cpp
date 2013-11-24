/*
    yahoochatselectordialog.h

    Copyright (c) 2006 by Andre Duffeck <duffeck@kde.org>
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "yahoochatselectordialog.h"

#include <QDomDocument>
#include <QTreeWidgetItem>
#include <QHeaderView>

#include <kdebug.h>

#include "ui_yahoochatselectorwidgetbase.h"

YahooChatSelectorDialog::YahooChatSelectorDialog( QWidget *parent )
	: KDialog( parent )
{	
	setCaption( i18n( "Choose a chat room..." ) );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	showButtonSeparator( true );
	mUi = new Ui_YahooChatSelectorWidgetBase();

	mUi->setupUi( mainWidget() );

	mUi->treeCategories->header()->hide();
	mUi->treeRooms->header()->hide();

	QTreeWidgetItem *loading = new QTreeWidgetItem(mUi->treeCategories);
	loading->setText( 0, i18n("Loading...") );
	mUi->treeCategories->addTopLevelItem( loading );

 	connect(mUi->treeCategories, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), 
		this, SLOT(slotCategorySelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(mUi->treeRooms, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotChatRoomDoubleClicked(QTreeWidgetItem*,int)) );
}

YahooChatSelectorDialog::~YahooChatSelectorDialog()
{
	delete mUi;
}

void YahooChatSelectorDialog::slotCategorySelectionChanged( QTreeWidgetItem *newItem, QTreeWidgetItem *oldItem )
{
	Q_UNUSED( oldItem );
	kDebug(YAHOO_RAW_DEBUG) << "Selected Category: " << newItem->text( 0 ) <<  "(" << newItem->data( 0, Qt::UserRole ).toInt() << ")";

	mUi->treeRooms->clear();
	
	QTreeWidgetItem *loading = new QTreeWidgetItem(mUi->treeRooms);
	loading->setText( 0, i18n("Loading...") );
	mUi->treeRooms->addTopLevelItem( loading );

	Yahoo::ChatCategory category;
	category.id = newItem->data( 0, Qt::UserRole ).toInt();
	category.name = newItem->text( 0 );

	emit chatCategorySelected( category );
}

void YahooChatSelectorDialog::slotChatRoomDoubleClicked( QTreeWidgetItem * item, int column )
{
	Q_UNUSED( column );
	Q_UNUSED( item );
	QDialog::accept();
}

void YahooChatSelectorDialog::slotSetChatCategories( const QDomDocument &doc )
{
	kDebug(YAHOO_RAW_DEBUG) << doc.toString();
	mUi->treeCategories->takeTopLevelItem(0);

	QTreeWidgetItem *root = new QTreeWidgetItem( mUi->treeCategories );
	root->setText( 0, i18n("Yahoo Chat rooms") );
	QDomNode child = doc.firstChild();
	mUi->treeCategories->setItemExpanded( root, true );
	while( !child.isNull() )
	{
		parseChatCategory(child, root);
		child = child.nextSibling();
	}
	
}

void YahooChatSelectorDialog::parseChatCategory( const QDomNode &node, QTreeWidgetItem *parentItem )
{
	QTreeWidgetItem *newParent = parentItem;
	if( node.nodeName().startsWith( "category" ) )
	{
		QTreeWidgetItem *item = new QTreeWidgetItem( parentItem );
	
		item->setText( 0, node.toElement().attribute( "name" ) );
		item->setData( 0, Qt::UserRole, node.toElement().attribute( "id" ) );
		parentItem->addChild( item );
		newParent = item;
	}
	QDomNode child = node.firstChild();
	while( !child.isNull() )
	{
		parseChatCategory(child, newParent);
		child = child.nextSibling();
	}
}

void YahooChatSelectorDialog::slotSetChatRooms( const Yahoo::ChatCategory &category, const QDomDocument &doc )
{
	kDebug(YAHOO_RAW_DEBUG) << doc.toString();
	Q_UNUSED( category );
	mUi->treeRooms->clear();

	QDomNode child = doc.firstChild();
	while( !child.isNull() )
	{
		parseChatRoom( child );
		child = child.nextSibling();
	}
	
}

void YahooChatSelectorDialog::parseChatRoom( const QDomNode &node )
{
	if( node.nodeName().startsWith( "room" ) )
	{
		QTreeWidgetItem *item = new QTreeWidgetItem( mUi->treeRooms );
		QDomElement elem = node.toElement();
		QString name = elem.attribute( "name" );
		QString id = elem.attribute( "id" );
		item->setText( 0, name );
		item->setData( 0, Qt::ToolTipRole, elem.attribute( "topic" ) );
		item->setData( 0, Qt::UserRole, id );
		
		QDomNode child;
		for( child = node.firstChild(); !child.isNull(); child = child.nextSibling() )
		{
			if( child.nodeName().startsWith( "lobby" ) )
			{
				QTreeWidgetItem *lobby = new QTreeWidgetItem( item );
				QDomElement e = child.toElement();
				QString voices = e.attribute( "voices" );
				QString users = e.attribute( "users" );
				QString webcams = e.attribute( "webcams" );
				QString count = e.attribute( "count" );
				lobby->setText( 0, name + QString( ":%1" )
						.arg( count ) );
				lobby->setData( 0, Qt::ToolTipRole, i18n( "Users: %1 Webcams: %2 Voices: %3", users, webcams, voices ) );
				lobby->setData( 0, Qt::UserRole, id );
				item->addChild( lobby );
			}
		}
	}
	else
	{
		QDomNode child = node.firstChild();
		while( !child.isNull() )
		{
			parseChatRoom( child );
			child = child.nextSibling();
		}
	}
}

Yahoo::ChatRoom YahooChatSelectorDialog::selectedRoom()
{
	Yahoo::ChatRoom room;
	QTreeWidgetItem *item = mUi->treeRooms->selectedItems().first();
	room.name =  item->text( 0 );
	room.topic = item->data( 0, Qt::ToolTipRole ).toString();
	room.id = item->data( 0, Qt::UserRole ).toInt();

	return room;
}

#include "yahoochatselectordialog.moc"

