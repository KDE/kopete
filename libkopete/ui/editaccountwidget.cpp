/*
    editaccountwidget.cpp - Kopete Account Widget

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "editaccountwidget.h"

class KopeteEditAccountWidgetPrivate
{
public:
	Kopete::Account *account;
};

KopeteEditAccountWidget::KopeteEditAccountWidget( Kopete::Account *account )
{
	d = new KopeteEditAccountWidgetPrivate;
	d->account = account;
}

KopeteEditAccountWidget::~KopeteEditAccountWidget()
{
	delete d;
}

Kopete::Account * KopeteEditAccountWidget::account() const
{
	return d->account;
}

void KopeteEditAccountWidget::setAccount( Kopete::Account *account )
{
	d->account = account;
}

// vim: set noet ts=4 sts=4 sw=4:

