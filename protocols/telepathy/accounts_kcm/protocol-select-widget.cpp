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

#include "protocol-select-widget.h"

#include "protocol-item.h"
#include "protocol-list-model.h"

#include "ui_protocol-select-widget.h"

#include <KDebug>

#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingStringList>

class ProtocolSelectWidget::Private
{
public:
    Private()
     : ui(0),
       model(0)
    {
        kDebug();
    }

    Ui::ProtocolSelectWidget *ui;
    ProtocolListModel *model;
};

ProtocolSelectWidget::ProtocolSelectWidget(QWidget *parent)
 : QWidget(parent),
   d(new Private)
{
    kDebug();

    // Set up the widget
    d->model = new ProtocolListModel(this);

    d->ui = new Ui::ProtocolSelectWidget;
    d->ui->setupUi(this);
    d->ui->protocolListView->setModel(d->model);

    connect(d->ui->protocolListView->selectionModel(),
            SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            SLOT(onCurrentChanged(const QModelIndex &)));

    // Load the list of all installed Telepathy Connection Managers Asynchronously
    QTimer::singleShot(0, this, SLOT(getConnectionManagerList()));
}

ProtocolSelectWidget::~ProtocolSelectWidget()
{
    kDebug();

    delete d;
}

void ProtocolSelectWidget::getConnectionManagerList()
{
    kDebug();

    // Ask TpQt4 for the list of all installed Connection Managers.
    Tp::PendingStringList *psl = Tp::ConnectionManager::listNames();

    connect(psl,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onConnectionManagerListGot(Tp::PendingOperation*)));
}

void ProtocolSelectWidget::onConnectionManagerListGot(Tp::PendingOperation *op)
{
    kDebug();

    // Check the operation completed successfully.
    if (op->isError()) {
        kWarning() << "Operation failed:" << op->errorName() << op->errorMessage();
        return;
    }

    // Check the operation we were passed is of the correct type.
    Tp::PendingStringList *psl = qobject_cast<Tp::PendingStringList*>(op);
    if (!psl) {
        kWarning() << "Operation is not of type PendingStringList.";
        return;
    }

    foreach (QString cmName, psl->result()) {
        // Add the CM to the ProtocolListModel
        d->model->addConnectionManager(Tp::ConnectionManager::create(cmName));
    }
}

// Return the selected ProtocolItem or 0 if nothing is selected.
ProtocolItem *ProtocolSelectWidget::selectedProtocol()
{
    kDebug();

    // Get the indexes of the selected items from the view
    QModelIndexList selectedIndexes = d->ui->protocolListView->selectionModel()->selectedIndexes();

    // If more than 1 protocol is selected (shouldn't be possible, but just in case) error.
    if (selectedIndexes.size() > 1) {
        kWarning() << "More than 1 protocol is selected.";
        return 0;
    }

    // If no indexes are selected, return 0.
    if (selectedIndexes.size() == 0) {
        return 0;
    }

    // 1 index is selected. Return the ProtocolItem for that.
    return d->model->itemForIndex(selectedIndexes.at(0));
}

void ProtocolSelectWidget::onCurrentChanged(const QModelIndex &current)
{
    kDebug();

    Q_EMIT selectedProtocolChanged(d->model->itemForIndex(current));
}


#include "protocol-select-widget.moc"

