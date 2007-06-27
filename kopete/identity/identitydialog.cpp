/*
    identitydialog.cpp  -  Kopete identity configuration dialog

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

    Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "identitydialog.h"
#include "ui_identitygeneral.h"

#include <KIcon>
#include <kopeteidentity.h>

class IdentityDialog::Private
{
public:
	Kopete::Identity *identity;
	Ui::IdentityGeneral general;
};

IdentityDialog::IdentityDialog(Kopete::Identity *identity, QWidget *parent)
: Kopete::UI::InfoDialog(parent, i18n("Identity Information"), "identity")
{
	d = new Private();
	d->identity = identity;
	
	// add the general page
	QWidget *w = new QWidget(this);
	d->general.setupUi(w);
	d->general.selectPhoto->setIcon(KIcon("fileview-preview"));
	d->general.clearPhoto->setIcon(KIcon("clear-left"));
	addWidget(w, i18n("General Information"));
}

IdentityDialog::~IdentityDialog()
{
	delete d;
}

#include "identitydialog.moc"
// vim: set noet ts=4 sts=4 sw=4:
