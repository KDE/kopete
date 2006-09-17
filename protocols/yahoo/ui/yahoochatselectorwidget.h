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


#ifndef YAHOOCHATSELECTORWIDGET_H
#define YAHOOCHATSELECTORWIDGET_H

#include <QWidget>
#include "yahootypes.h"

class Ui_YahooChatSelectorWidgetBase;
class QTreeWidgetItem;

class YahooChatSelectorWidget : public QWidget
{
	Q_OBJECT
public:
	YahooChatSelectorWidget( QWidget *parent = 0);
	~YahooChatSelectorWidget();

	Yahoo::ChatRoom selectedRoom();
public Q_SLOTS:
	void slotSetChatCategories( const QDomDocument & );
	void slotSetChatRooms( const Yahoo::ChatCategory &, const QDomDocument & );
Q_SIGNALS:
	void chatCategorySelected( Yahoo::ChatCategory );
private Q_SLOTS:
	void slotCategorySelectionChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );
private:
	void parseChatCategory( const QDomNode &, QTreeWidgetItem * );

	Ui_YahooChatSelectorWidgetBase *mUi;
};

#endif
