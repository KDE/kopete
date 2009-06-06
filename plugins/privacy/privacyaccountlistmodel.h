/*
    privacyaccountlist.h - a list of accounts that are part of the black/whitelist

    Copyright (c) 2006 by Andre Duffeck             <duffeck@kde.org>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef PRIVACYACCOUNTLIST_H
#define PRIVACYACCOUNTLIST_H

#include <QAbstractTableModel>
#include <QPair>
#include <kopete_export.h>

class QStringList;
namespace Kopete { class Protocol; }

typedef QPair< QString, Kopete::Protocol *> AccountListEntry;

class KOPETEPRIVACY_EXPORT PrivacyAccountListModel : public QAbstractTableModel
{
Q_OBJECT

public:
	PrivacyAccountListModel(QObject *parent = 0);
	~PrivacyAccountListModel();

	void loadAccounts( const QStringList &accounts );
	void addAccount(const QString &accountId, Kopete::Protocol *protocol);
	void addAccount(const AccountListEntry &entry);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool removeRow(int position, const QModelIndex &index = QModelIndex());
	bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

	QStringList toStringList() const;

private:
	QList< AccountListEntry > m_list;
};

#endif
