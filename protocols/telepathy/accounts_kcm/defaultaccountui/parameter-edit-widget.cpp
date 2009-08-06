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

#include "parameter-edit-widget.h"

#include "parameter-edit-delegate.h"
#include "parameter-edit-model.h"

#include "ui_parameter-edit-widget.h"

#include <KDebug>

class ParameterEditWidget::Private
{
public:
    Private()
     : ui(0), delegate(0), model(0)
    {
        kDebug();
    }

    Ui::ParameterEditWidget *ui;
    ParameterEditDelegate *delegate;
    ParameterEditModel *model;
};

ParameterEditWidget::ParameterEditWidget(Tp::ProtocolParameterList parameters,
                                         const QVariantMap &values,
                                         QWidget *parent)
 : AbstractAccountParametersWidget(parameters, values, parent),
   d(new Private)
{
    kDebug();

    // Set up the UI.
    d->ui = new Ui::ParameterEditWidget;
    d->ui->setupUi(this);

    d->model = new ParameterEditModel(this);
    d->ui->parameterListView->setModel(d->model);
    d->delegate = new ParameterEditDelegate(d->ui->parameterListView, this);
    d->ui->parameterListView->setItemDelegate(d->delegate);

    connect(d->delegate,
            SIGNAL(dataChanged(QModelIndex, QVariant, int)),
            SLOT(onDelegateDataChanged(QModelIndex, QVariant, int)));

    // Add the parameters to the model.
    foreach (Tp::ProtocolParameter *parameter, parameters) {
        d->model->addItem(parameter, values.value(parameter->name(), parameter->defaultValue()));
    }
}

ParameterEditWidget::~ParameterEditWidget()
{
    kDebug();

    delete d;
}

QMap<Tp::ProtocolParameter*, QVariant> ParameterEditWidget::parameterValues() const
{
    return d->model->parameterValues();
}


void ParameterEditWidget::onDelegateDataChanged(const QModelIndex &index, const QVariant &value, int role)
{
    d->model->setData(index, value, role);
}


#include "parameter-edit-widget.moc"

