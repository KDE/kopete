/*
    accounttreewidget.h  -  Kopete account tree widget

    Copyright (c) 2009      by Roman Jarosz           <kedgedev@centrum.cz>
    Kopete    (c) 2009      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef ACCOUNTTREEWIDGET_H
#define ACCOUNTTREEWIDGET_H

#include <QTreeWidget>
#include <QPointer>

#include "kopeteaccount.h"

class AccountTreeWidget : public QTreeWidget
{
	Q_OBJECT
public:
	AccountTreeWidget( QWidget *parent = 0 );

signals:
	void itemPositionChanged();

protected:
	void dragEnterEvent( QDragEnterEvent *event );
	void dropEvent( QDropEvent *event );

};

class KopeteAccountLVI : public QTreeWidgetItem
{
public:
	KopeteAccountLVI( Kopete::Account *a, QTreeWidgetItem* parent) : QTreeWidgetItem(parent) , m_account(a) { }
	Kopete::Account *account() { return m_account; }
	
private:
	//need to be guarded because some accounts may be linked (that's the case of jabber transports)
	QPointer<Kopete::Account> m_account;
};

class KopeteIdentityLVI : public QTreeWidgetItem
{
public:
	KopeteIdentityLVI( Kopete::Identity *i, QTreeWidget* parent) : QTreeWidgetItem(parent), m_identity (i) { }
	Kopete::Identity *identity() { return m_identity; }
	
private:
	Kopete::Identity *m_identity;
};

#endif
