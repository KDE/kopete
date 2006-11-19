/*
    yahoochatselectorwidget.h

    Copyright (c) 2006 by Andre Duffeck <andre@duffeck.de>
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

#include <QDomDocument>
#include <QTreeWidgetItem>
#include <QHeaderView>

#include "ui_yahoochatselectorwidgetbase.h"
#include "yahoochatselectorwidget.h"

YahooChatSelectorWidget::YahooChatSelectorWidget( QWidget *parent )
	: QWidget(parent)
{
	mUi = new Ui_YahooChatSelectorWidgetBase;

	QBoxLayout *layout = new QVBoxLayout(this);
	QWidget *widget = new QWidget(this);
	mUi->setupUi(widget);
	layout->addWidget(widget);

	mUi->treeCategories->header()->hide();
	mUi->treeRooms->header()->hide();

	QTreeWidgetItem *loading = new QTreeWidgetItem(mUi->treeCategories);
	loading->setText( 0, i18n("Loading...") );
	mUi->treeCategories->addTopLevelItem( loading );

 	connect(mUi->treeCategories, SIGNAL(currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * )), 
		this, SLOT(slotCategorySelectionChanged( QTreeWidgetItem *, QTreeWidgetItem * )));
}

YahooChatSelectorWidget::~YahooChatSelectorWidget()
{
}

void YahooChatSelectorWidget::slotCategorySelectionChanged( QTreeWidgetItem *newItem, QTreeWidgetItem *oldItem )
{
	Q_UNUSED( oldItem );
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Selected Category: " << newItem->text( 0 ) <<  "(" << newItem->data( 0, Qt::UserRole ).toInt() << ")" << endl;

	mUi->treeRooms->clear();
	
	QTreeWidgetItem *loading = new QTreeWidgetItem(mUi->treeRooms);
	loading->setText( 0, i18n("Loading...") );
	mUi->treeRooms->addTopLevelItem( loading );

	Yahoo::ChatCategory category;
	category.id = newItem->data( 0, Qt::UserRole ).toInt();
	category.name = newItem->text( 0 );

	emit chatCategorySelected( category );
}

void YahooChatSelectorWidget::slotSetChatCategories( const QDomDocument &doc )
{
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

void YahooChatSelectorWidget::parseChatCategory( const QDomNode &node, QTreeWidgetItem *parentItem )
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

void YahooChatSelectorWidget::slotSetChatRooms( const Yahoo::ChatCategory &category, const QDomDocument &doc )
{
	Q_UNUSED( category );
	mUi->treeRooms->clear();

	QDomNode child = doc.firstChild();
	while( !child.isNull() )
	{
		parseChatRoom( child );
		child = child.nextSibling();
	}
	
}

void YahooChatSelectorWidget::parseChatRoom( const QDomNode &node )
{
	if( node.nodeName().startsWith( "room" ) )
	{
		QTreeWidgetItem *item = new QTreeWidgetItem( mUi->treeRooms );
	
		item->setText( 0, node.toElement().attribute( "name" ) );
		item->setData( 0, Qt::ToolTipRole, node.toElement().attribute( "topic" ) );
		item->setData( 0, Qt::UserRole, node.toElement().attribute( "id" ) );
	}
	QDomNode child = node.firstChild();
	while( !child.isNull() )
	{
		parseChatRoom( child );
		child = child.nextSibling();
	}
}

Yahoo::ChatRoom YahooChatSelectorWidget::selectedRoom()
{
	Yahoo::ChatRoom room;
	QTreeWidgetItem *item = mUi->treeRooms->selectedItems().first();
	room.name =  item->text( 0 );
	room.topic = item->data( 0, Qt::ToolTipRole ).toString();
	room.id = item->data( 0, Qt::UserRole ).toInt();

	return room;
}

#include "yahoochatselectorwidget.moc"

