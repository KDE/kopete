/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>
    Copyright (C) 2008-2009 Pali Rohár <pali.rohar@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/
#include "skypeaddcontact.h"
#include "skypeprotocol.h"
#include "ui_skypeaddcontactbase.h"
#include "skypeaccount.h"

#include <kdebug.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <kmessagebox.h>
#include <klocale.h>

namespace Ui { class SkypeAddContactBase; }

class SkypeAddContactPrivate {
	public:
		SkypeProtocol *protocol;
		Ui::SkypeAddContactBase *widget;
		SkypeAccount *account;
};

SkypeAddContact::SkypeAddContact(SkypeProtocol *protocol, QWidget *parent, SkypeAccount *account, const char *) : AddContactPage(parent) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	d = new SkypeAddContactPrivate();//create the d ponter
	d->protocol = protocol;//remember the protocol
	d->account = account;

	QVBoxLayout *topLayout = new QVBoxLayout( this );//create the layout
	QWidget* w = new QWidget( this );
	topLayout->addWidget( w );
	d->widget = new Ui::SkypeAddContactBase();//create the insides
	d->widget->setupUi( w );
}


SkypeAddContact::~SkypeAddContact() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	//free everything
	delete d->widget;
	delete d;
}

bool SkypeAddContact::validateData() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	d->widget->NameEdit->setText(d->widget->NameEdit->text().toLower());

	if (!d->account->canComunicate()) {
		KMessageBox::sorry(this, i18n("You must connect to Skype first."), i18n("Not Connected"), QFlags<KMessageBox::Option>());
		return false;
	}

	if (d->widget->NameEdit->text().isEmpty()) {//He wrote nothing
		KMessageBox::sorry(this, i18n("You must write the contact's name."), i18n("Wrong Information"));//Tell the user I don't like this at all
		return false;//and don't allow to continue
	}

	if (d->widget->NameEdit->text() == "echo123") {
		KMessageBox::sorry(this, i18n("Contact echo123 is not needed. You can make test call in Skype protocol actions."), i18n("Wrong Information"));//Tell the user
		return false;//and don't allow to continue
	}

	if (d->account->contact(d->widget->NameEdit->text())) {//this contact already exists in this account
		KMessageBox::sorry(this, i18n("This contact already exists in this account."), i18n("Wrong Information"));//Tell the user
		return false;//do not proceed
	}

	if (d->account->getMyselfSkypeName() == d->widget->NameEdit->text()){
		KMessageBox::sorry(this, i18n("You cannot add yourself as a contact."), i18n("Wrong Information"));//Tell the user
		return false;//do not proceed
	}

	return true;
}

bool SkypeAddContact::apply(Kopete::Account *, Kopete::MetaContact *metaContact) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	d->account->registerContact(d->widget->NameEdit->text());
	d->account->addContact(d->widget->NameEdit->text(), metaContact, Kopete::Account::ChangeKABC);
	return true;//all OK
}

#include "skypeaddcontact.moc"
