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

#ifndef TELEPATHY_ACCOUNTS_KCM_PARAMETER_EDIT_MODEL_H
#define TELEPATHY_ACCOUNTS_KCM_PARAMETER_EDIT_MODEL_H

#include <QtCore/QAbstractListModel>

class ParameterItem;

namespace Tp {
    class ProtocolParameter;
}

class ParameterEditModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ParameterEditModel);

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        LocalizedNameRole,
        TypeRole,
        ValueRole,
        SecretRole,
        RequiredRole,
        RequiredForRegistrationRole,
        UserRole = Qt::UserRole + 42
    };

    explicit ParameterEditModel(QObject *parent = 0);
    virtual ~ParameterEditModel();

    virtual int rowCount(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    void addItem(Tp::ProtocolParameter *parameter, const QVariant &originalValue);
    QMap<Tp::ProtocolParameter*, QVariant> parameterValues() const;

private:
    QList<ParameterItem*> m_items;
};


#endif // header guard

