/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>

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

#include "skypedetails.h"
#include "skypeaccount.h"

#include <kdebug.h>
#include <klocale.h>
#include <qlineedit.h>
#include <qcombobox.h>

SkypeDetails::SkypeDetails() : SkypeDetailsBase() {
	kdDebug(14311) << k_funcinfo << endl;
}


SkypeDetails::~SkypeDetails() {
	kdDebug(14311) << k_funcinfo << endl;
}

void SkypeDetails::closeEvent(QCloseEvent *) {
	kdDebug(14311) << k_funcinfo << endl;
	deleteLater();
}

void SkypeDetails::changeAuthor(int item) {
	kdDebug(14311) << k_funcinfo << endl;
	switch (item) {
		case 0:
			account->authorizeUser(idEdit->text());
			break;
		case 1:
			account->disAuthorUser(idEdit->text());
			break;
		case 2:
			account->blockUser(idEdit->text());
			break;
	}
}

SkypeDetails &SkypeDetails::setNames(const QString &id, const QString &nick, const QString &name) {
	setCaption(i18n("Details for user %1").arg(id));
	idEdit->setText(id);
	nickEdit->setText(nick);
	nameEdit->setText(name);
	return *this;
}

SkypeDetails &SkypeDetails::setPhones(const QString &priv, const QString &mobile, const QString &work) {
	privatePhoneEdit->setText(priv);
	mobilePhoneEdit->setText(mobile);
	workPhoneEdit->setText(work);
	return *this;
}

SkypeDetails &SkypeDetails::setHomepage(const QString &homepage) {
	homepageEdit->setText(homepage);
	return *this;
}

SkypeDetails &SkypeDetails::setAuthor(int author, SkypeAccount *account) {
	authorCombo->setCurrentItem(author);
	this->account = account;
	return *this;
}

SkypeDetails &SkypeDetails::setSex(const QString &sex) {
	sexEdit->setText(sex);
	return *this;
}

#include "skypedetails.moc"
