/*
 * This file is part of telepathy-accounts-kcm
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef TELEPATHY_ACCOUNTS_KCM_PARAMETER_EDIT_DELEGATE_H
#define TELEPATHY_ACCOUNTS_KCM_PARAMETER_EDIT_DELEGATE_H

#include <KWidgetItemDelegate>

class ParameterEditDelegate : public KWidgetItemDelegate
{
    Q_OBJECT

public:
    explicit ParameterEditDelegate(QAbstractItemView *itemView, QObject *parent = 0);
    virtual ~ParameterEditDelegate();

    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private Q_SLOTS:
    void onLineEditTextChanged(QString text);
    void onCheckBoxToggled(bool checked);
    void onIntegerEditTextChanged(const QString &text);
    void onUnsignedIntegerEditTextChanged(const QString &text);

Q_SIGNALS:
    void dataChanged(const QModelIndex &index, const QVariant &value, int role);

protected:
    virtual QList<QWidget*> createItemWidgets() const;
    virtual void updateItemWidgets(const QList<QWidget*> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QPersistentModelIndex &index) const;
};


#endif  // Header guard

