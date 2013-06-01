/*
 * privacylistmodel.cpp
 * Copyright (C) 2006  Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
#include "privacylistmodel.h"
#include <QAbstractListModel>
#include <QPointer>

#include "privacylist.h"
#include "privacyruledlg.h"

PrivacyListModel::PrivacyListModel(const PrivacyList& list, QObject* parent) : QAbstractListModel(parent), list_(list)
{
}

int PrivacyListModel::rowCount(const QModelIndex&) const
{
	return list_.items().count();
}

int PrivacyListModel::columnCount(const QModelIndex&) const
{
	return 2;
}

QVariant PrivacyListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= list_.items().count())
		return QVariant();

	if (role == Qt::DisplayRole) {
		if (index.column() == TextColumn) 
			return list_.item(index.row()).toString();
		else if (index.column() == ValueColumn) 
			return list_.item(index.row()).value();
	}
	else if (role == BlockedRole) {
		return list_.item(index.row()).isBlock();
	}
	
	return QVariant();
}

void PrivacyListModel::setList(const PrivacyList& list)
{
	list_ = list;
	reset();
}

bool PrivacyListModel::moveUp(const QModelIndex& index)
{
	if (index.isValid() && list_.moveItemUp(index.row())) {
		reset();
		return true;
	}
	return false;
}

bool PrivacyListModel::moveDown(const QModelIndex& index)
{
	if (index.isValid() && list_.moveItemDown(index.row())) {
		reset();
		return true;
	}
	return false;
}

bool PrivacyListModel::removeRows(int row, int count, const QModelIndex&)
{
	//kDebug (JABBER_DEBUG_GLOBAL) << "PrivacyListModel::removeRows";
	beginRemoveRows(QModelIndex(), row, row+count-1);
	while(count > 0) {
		list_.removeItem(row);
		count--;
	}
	endRemoveRows();
	return true;
}

bool PrivacyListModel::add()
{
	QPointer <PrivacyRuleDlg> d = new PrivacyRuleDlg;
	if (d->exec() == QDialog::Accepted && d) {
		list_.insertItem(0,d->rule());
		delete d;
		reset();
		return true;
	}
	delete d;
	return false;
}

bool PrivacyListModel::edit(const QModelIndex& index)
{
	if (index.isValid()) {
		QPointer <PrivacyRuleDlg> d = new PrivacyRuleDlg;
		d->setRule(list_.item(index.row()));
		if (d->exec() == QDialog::Accepted && d) {
			list_.updateItem(index.row(),d->rule());
			delete d;
			reset();
			return true;
		}
		delete d;
	}
	return false;
}

