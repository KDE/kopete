/*
    identitydialog.cpp  -  Kopete identity configuration dialog

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2007         Will Stephenson        <wstephenson@kde.org>

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
#include "ui_identitydetailed.h"

#include <KIcon>
#include <kopeteidentity.h>
#include <avatardialog.h>

class IdentityDialog::Private
{
public:
	Kopete::Identity *identity;
	Kopete::Global::Properties *props;
	Ui::IdentityGeneral general;
	Ui::IdentityDetailed detailed;
	QString photoPath;
};

IdentityDialog::IdentityDialog(Kopete::Identity *identity, QWidget *parent)
: Kopete::UI::InfoDialog(parent, i18n("Identity Information"), "identity"), d(new Private())
{
	Q_ASSERT(identity);

	setTitle(identity->label());
	setWindowTitle(i18n("Identity Information"));

	d->identity = identity;
	d->props = Kopete::Global::Properties::self();
	
	// add the general page
	QWidget *w = new QWidget(this);
	d->general.setupUi(w);
	d->general.selectPhoto->setIcon(KIcon("view-preview"));
	d->general.clearPhoto->setIcon(KIcon("edit-clear-locationbar-rtl"));
	d->general.photo->setText( QString("<qt><a href=\"selectPhoto\">"
										"<p align=\"center\">%1</p>"
										"</a></qt>").arg( i18n("No Photo") ));

	connect(d->general.selectPhoto, SIGNAL(clicked(bool)),
			this, SLOT(slotSelectPhoto()));
	connect(d->general.photo, SIGNAL(linkActivated(QString)),
			this, SLOT(slotSelectPhoto()));
	connect(d->general.clearPhoto, SIGNAL(clicked(bool)),
			this, SLOT(slotClearPhoto()));
	addWidget(w, i18n("General Information"));

	// add the detailed page
	w = new QWidget(this);
	d->detailed.setupUi(w);
	addWidget(w, i18n("Detailed Information"));

	setIcon(KIcon(d->identity->customIcon()));

	load();
}

IdentityDialog::~IdentityDialog()
{
	delete d;
}

void IdentityDialog::load()
{
	//-------------- General Info ---------------------
	// Photo
	if (d->identity->hasProperty( d->props->photo().key() ))
		setPhoto( d->identity->property(d->props->photo()).value().toString() );


	// Label
	d->general.label->setText( d->identity->label() );

	// NickName
	if (d->identity->hasProperty( d->props->nickName().key() ))
		d->general.nickName->setText( d->identity->property(d->props->nickName()).value().toString() );

	// FirstName
	if (d->identity->hasProperty( d->props->firstName().key() ))
		d->general.firstName->setText( d->identity->property(d->props->firstName()).value().toString() );

	// LastName
	if (d->identity->hasProperty( d->props->lastName().key() ))
		d->general.lastName->setText( d->identity->property(d->props->lastName()).value().toString() );
	
	//-------------- Detailed Info --------------------
	// Email
	if (d->identity->hasProperty( d->props->emailAddress().key() ))
		d->detailed.email->setText( d->identity->property(d->props->emailAddress()).value().toString() );

	// PrivatePhone
	if (d->identity->hasProperty( d->props->privatePhone().key() ))
		d->detailed.privatePhone->setText( 
				d->identity->property(d->props->privatePhone()).value().toString() );

	// MobilePhone
	if (d->identity->hasProperty( d->props->privateMobilePhone().key() ))
		d->detailed.mobilePhone->setText( 
				d->identity->property(d->props->privateMobilePhone()).value().toString() );

}

void IdentityDialog::slotSave()
{
	//-------------- General Info ---------------------
	d->identity->setLabel( d->general.label->text() );
	if ( d->photoPath.isEmpty() )
		d->identity->removeProperty( d->props->photo() );
	else
		d->identity->setProperty( d->props->photo(), d->photoPath );
	d->identity->setProperty( d->props->nickName(), d->general.nickName->text() );
	d->identity->setProperty( d->props->firstName(), d->general.firstName->text() );
	d->identity->setProperty( d->props->lastName(), d->general.lastName->text() );

	//-------------- Detailed Info --------------------
	d->identity->setProperty( d->props->emailAddress(), d->detailed.email->text() );
	d->identity->setProperty( d->props->privatePhone(), d->detailed.privatePhone->text() );
	d->identity->setProperty( d->props->privateMobilePhone(), d->detailed.mobilePhone->text() );
}

void IdentityDialog::setPhoto(QString path)
{
	d->photoPath = path;
	if (!path.isEmpty())
	{
		d->general.photo->setText( QString("<qt><a href=\"selectPhoto\">"
											"<p align=\"center\"><img src=\"%1\"></p>"
										"</a>").arg( d->photoPath ) );
	}
	else
	{
		d->general.photo->setText( QString("<qt><a href=\"selectPhoto\">"
											"<p align=\"center\">No Photo</p>"
										"</a>").arg( i18n("No Photo") ));
	}
}

void IdentityDialog::slotSelectPhoto()
{
	bool ok;
	QString photo = Kopete::UI::AvatarDialog::getAvatar(this, d->photoPath, &ok);
	if ( ok )
		setPhoto( photo );
}

void IdentityDialog::slotClearPhoto()
{
	setPhoto( QString() );
}

#include "identitydialog.moc"
// vim: set noet ts=4 sts=4 sw=4:
