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

#include "skypeconference.h"
#include "skypecalldialog.h"

#include <qstring.h>
#include <qlayout.h>
#include <kdebug.h>
#include <klocale.h>

class SkypeConferencePrivate {
	public:
		///my id
		QString id;
		///The layout
		QHBoxLayout *layout;
};

SkypeConference::SkypeConference(const QString &id) : KDialog() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	//create the d pointer
	d = new SkypeConferencePrivate();

	//some UI
	setCaption(i18n("Conference Call"));
	d->layout = new QHBoxLayout(this);

	//remember all things
	d->id = id;

	//show myself
	show();
}

SkypeConference::~SkypeConference() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	//free all memory
	delete d->layout;
	delete d;
}

void SkypeConference::closeEvent(QCloseEvent *) {
	emit removeConference(d->id);

	deleteLater();
}

void SkypeConference::embedCall(SkypeCallDialog *dialog) {
	dialog->hide();
	///TODO: Port to kde4
	//insertChild(dialog);
	d->layout->addWidget(dialog);

	connect(this, SIGNAL(destroyed()), dialog, SLOT(hangUp()));
}

#include "skypeconference.moc"
