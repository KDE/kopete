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

#include <QtGui/QWidget>
#include <kopeteprotocol.h>
#include "kopete_export.h"

class AccountSelectorPrivate;
class Q3ListViewItem;
/**
 * \brief widget to select an account, based on K3ListView
 * @author Stefan Gehn <metz AT gehn.net>
 */
class KOPETE_EXPORT AccountSelector : public QWidget
{
Q_OBJECT

	public:
		/**
		 * Constructor.
		 *
		 * The parameter @p parent is handled by
		 * K3ListView.
		 */
		AccountSelector(QWidget *parent=0);

		/**
		 * Constructor for a list of accounts for one protocol only
		 *
		 * The parameters @p parent is handled by
		 * K3ListView. @p proto defines the protocol whose accounts are
		 * shown in the list
		 */
		explicit AccountSelector(Kopete::Protocol *proto, QWidget *parent=0);

		/**
		 * Destructor.
		 */
		~AccountSelector();

		/**
		 * Select @p account in the list, in case it's part of the list
		 */
		void setSelected(Kopete::Account *account);

		/**
		 * Returns true in case @p account is in the list and
		 * the currently selected item, false otherwise
		 */
		bool isSelected(Kopete::Account *account);

		/**
		 * @return the currently selected account.
		 */
		Kopete::Account *selectedItem();

	signals:
		/**
		 * Emitted whenever the selection changed, @p acc is a pointer to the
		 * newly selected account
		 */
		void selectionChanged(Kopete::Account *acc);

	private slots:
		void slotSelectionChanged(Q3ListViewItem *item);

	private:
		void initUI();

	private:
		AccountSelectorPrivate *d;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
