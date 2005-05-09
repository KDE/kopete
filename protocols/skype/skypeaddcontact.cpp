/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <vorner@seznam.cz>

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
#include <skypeaddcontactbase.h>
#include "skypeaccount.h"

#include <kdebug.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <kmessagebox.h>
#include <klocale.h>

class SkypeAddContactWidget : public SkypeAddContactBase {
	private:
	public:
		SkypeAddContactWidget(QWidget *parent, const char *name = 0L) : SkypeAddContactBase(parent, name) {
			kdDebug(65320) << k_funcinfo << endl;//some debug info
		}
};

class SkypeAddContactPrivate {
	public:
		SkypeProtocol *protocol;
		SkypeAddContactWidget *widget;
		SkypeAccount *account;
};

SkypeAddContact::SkypeAddContact(SkypeProtocol *protocol, QWidget *parent, SkypeAccount *account, const char *name) : AddContactPage(parent, name) {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	d = new SkypeAddContactPrivate();//create the d ponter
	d->protocol = protocol;//remember the protocol
	d->account = account;

	(new QVBoxLayout(this))->setAutoAdd(true);//create the layout and add there automatically
	d->widget = new SkypeAddContactWidget(this);//create the insides
}


SkypeAddContact::~SkypeAddContact() {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	//free everything (the widget is deleted automatically)
	delete d;
}

bool SkypeAddContact::validateData() {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	if (d->widget->NameEdit->text().isEmpty()) {//He wrote nothing
		KMessageBox::sorry(d->widget, i18n("You must write the contact's name"), i18n("Wrong information"));//Tell the user I don't like this at all
		return false;//and don't allow to continue
	}

	if (d->account->contact(d->widget->NameEdit->text())) {//this contact already exists in this account
		KMessageBox::sorry(d->widget, i18n("This contact already exists in this account"), i18n("Wring information"));//Tell the user
		return false;//do not proceed
	}

	return true;
}

bool SkypeAddContact::apply(Kopete::Account *, Kopete::MetaContact *metaContact) {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	d->account->addContact(d->widget->NameEdit->text(), metaContact, Kopete::Account::ChangeKABC);//add it there
	return true;//all OK
}

#include "skypeaddcontact.moc"
