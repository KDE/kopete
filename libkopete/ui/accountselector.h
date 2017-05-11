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

#include <QWidget>
#include <kopeteprotocol.h>
#include "libkopete_export.h"

class AccountSelectorPrivate;
class QTreeWidgetItem;
/**
 * \brief widget to select an account, based on QTreeWidget
 * @author Stefan Gehn <metz AT gehn.net>
 */
class LIBKOPETE_EXPORT AccountSelector : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * The parameter @p parent is handled by
     * QTreeWidget.
     */
    AccountSelector(QWidget *parent = nullptr);

    /**
     * Constructor for a list of accounts for one protocol only
     *
     * The parameters @p parent is handled by
     * QTreeWidget. @p proto defines the protocol whose accounts are
     * shown in the list
     */
    explicit AccountSelector(Kopete::Protocol *proto, QWidget *parent = nullptr);

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

Q_SIGNALS:
    /**
     * Emitted whenever the selection changed, @p acc is a pointer to the
     * newly selected account
     */
    void selectionChanged(Kopete::Account *acc);

private Q_SLOTS:
    void slotSelectionChanged(QTreeWidgetItem *item);

private:
    void initUI();

private:
    AccountSelectorPrivate *d;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
