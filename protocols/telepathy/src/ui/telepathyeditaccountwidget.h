/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
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

#ifndef KOPETE_PROTOCOL_TELEPATHY_TELEPATHYEDITACCOUNTWIDGET_H
#define KOPETE_PROTOCOL_TELEPATHY_TELEPATHYEDITACCOUNTWIDGET_H

#include <ui/editaccountwidget.h>

#include <QtCore/QVariantMap>
#include <QtGui/QWidget>
#include <TelepathyQt4/ConnectionManager>

namespace Kopete {
    class Account;
}

class ProtocolItem;
class TelepathyAccount;

class TelepathyEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
    Q_OBJECT

public:
    explicit TelepathyEditAccountWidget(Kopete::Account *account, QWidget *parent = 0);
    ~TelepathyEditAccountWidget();

    virtual bool validateData();
    virtual Kopete::Account *apply();

protected:
    TelepathyAccount *account();

private slots:
    void writeConfig(const QString &connectionManager,
                     const QString &protocol,
                     const QVariantMap &parameters);

    void onProtocolGotSelected(bool selected);

private:
    void setupAddAccountUi();
    void setupEditAccountUi();

    Kopete::Account *applyAddedAccount();
    Kopete::Account *applyEditedAccount();

    class Private;
    Private *d;
};


#endif  // header guard

