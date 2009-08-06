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

#include "optional-parameter-edit-widget.h"

#include <KDebug>

class OptionalParameterEditWidget::Private
{
public:
    Private()
    {
        kDebug();
    }
};

OptionalParameterEditWidget::OptionalParameterEditWidget(Tp::ProtocolParameterList parameters,
                                                         const QVariantMap &values,
                                                         QWidget *parent)
 : ParameterEditWidget(parameters, values, parent),
   d(new Private)
{
    kDebug();
}

OptionalParameterEditWidget::~OptionalParameterEditWidget()
{
    kDebug();

    delete d;
}

bool OptionalParameterEditWidget::validateParameterValues()
{
    kDebug();

    // TODO: Implement me!
    return true;
}


#include "optional-parameter-edit-widget.moc"

