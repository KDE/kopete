/*
    accountselector.cpp - An Accountselector

    Copyright (c) 2004      by Stefan Gehn <metz AT gehn.net>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef ACCOUNTSELECTOR_H
#define ACCOUNTSELECTOR_H

#include <qwidget.h>
#include <kopeteprotocol.h>

class AccountSelectorPrivate;
class QListViewItem;
/**
 * @author Stefan Gehn <metz AT gehn.net>
 */
class AccountSelector : public QWidget
{
Q_OBJECT

public:
	AccountSelector(QWidget *parent=0, const char *name=0);
	AccountSelector(KopeteProtocol *proto, QWidget *parent=0, const char *name=0);
	~AccountSelector();

	void setSelected(KopeteAccount *account);
	bool isSelected(KopeteAccount *account);
	KopeteAccount *selectedItem();

	signals:
		void selectionChanged(KopeteAccount *acc);

	private slots:
		void slotSelectionChanged(QListViewItem *item);

	private:
		void initUI();

	private:
		AccountSelectorPrivate *d;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
