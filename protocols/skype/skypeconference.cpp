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

#include "skypeconference.h"
#include "skypecalldialog.h"

#include <qdict.h>
#include <qstring.h>
#include <qlayout.h>
#include <kdebug.h>
#include <klocale.h>

class SkypeConferencePrivate {
	public:
		//my id
		QString id;
		//The layout
		QHBoxLayout *layout;
};

SkypeConference::SkypeConference(const QString &id) : QDialog() {
	kdDebug(14311) << k_funcinfo << endl;

	//create the d pointer
	d = new SkypeConferencePrivate();

	//some UI
	setCaption(i18n("Conference call"));
	d->layout = new QHBoxLayout(this);

	//remember all things
	d->id = id;

	//show myself
	show();
}

SkypeConference::~SkypeConference() {
	kdDebug(14311) << k_funcinfo << endl;

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
	insertChild(dialog);
	d->layout->add(dialog);

	connect(this, SIGNAL(destroyed()), dialog, SLOT(hangUp()));
}

#include "skypeconference.moc"
