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


#ifndef YAHOOCHATSELECTORDIALOG_H
#define YAHOOCHATSELECTORDIALOG_H

#include <QDomDocument>
#include <KDialog>
#include "yahootypes.h"

class Ui_YahooChatSelectorWidgetBase;
class QTreeWidgetItem;

class YahooChatSelectorDialog : public KDialog
{
	Q_OBJECT
public:
	YahooChatSelectorDialog( QWidget *parent = 0);
	~YahooChatSelectorDialog();

	Yahoo::ChatRoom selectedRoom();
public Q_SLOTS:
	void slotSetChatCategories( const QDomDocument & );
	void slotSetChatRooms( const Yahoo::ChatCategory &, const QDomDocument & );
Q_SIGNALS:
	void chatCategorySelected( const Yahoo::ChatCategory & );
private Q_SLOTS:
	void slotCategorySelectionChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );
	void slotChatRoomDoubleClicked( QTreeWidgetItem * item, int column );
private:
	void parseChatCategory( const QDomNode &, QTreeWidgetItem * );
	void parseChatRoom( const QDomNode & );

	Ui_YahooChatSelectorWidgetBase *mUi;
};

#endif
