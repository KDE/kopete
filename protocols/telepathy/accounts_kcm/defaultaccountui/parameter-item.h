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

#ifndef TELEPATHY_ACCOUNTS_KCM_PARAMETER_ITEM_H
#define TELEPATHY_ACCOUNTS_KCM_PARAMETER_ITEM_H

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <TelepathyQt4/ConnectionManager>

class ParameterItem : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ParameterItem);

public:
    ParameterItem(Tp::ProtocolParameter *parameter,
                  const QVariant &originalValue,
                  QObject *parent = 0);
    virtual ~ParameterItem();

    QString name() const;
    QString localizedName() const;
    QVariant::Type type() const;
    QVariant value() const;
    bool isSecret() const;
    bool isRequired() const;
    bool isRequiredForRegistration() const;
    Tp::ProtocolParameter *parameter();

    void setValue(const QVariant &value);

private:
    Tp::ProtocolParameter *m_parameter;
    const QVariant m_originalValue;
    QVariant m_currentValue;
    QString m_localizedName;
};


#endif // header guard

