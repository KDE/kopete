/***************************************************************************
                          wppreferences.cpp  -  description
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// QT Includes
#include <qlayout.h>

// KDE Includes
#include <kcombobox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <klocale.h>

// Kopete Includes
#include <addcontactpage.h>

// Local Includes
#include "wpaddcontactbase.h"
#include "wpaccount.h"
#include "wpaddcontact.h"

WPAddContact::WPAddContact(QWidget *parent, WPAccount *newAccount, const char *name) : AddContactPage(parent, name)
{
//	kdDebug(14170) << "WPAddContact::WPAddContact(<owner>, " << newAccount << ", <parent>, " << name << ")" << endl;

	(new QVBoxLayout(this))->setAutoAdd(true);
	theDialog = new WPAddContactBase(this);
	connect(theDialog->mHostGroup, SIGNAL(activated(const QString &)), this, SLOT(slotSelected(const QString &)));
	connect(theDialog->mRefresh, SIGNAL(clicked()), this, SLOT(slotUpdateGroups()));
	theDialog->show();

	theAccount = newAccount;

	slotUpdateGroups();
	slotSelected(theDialog->mHostGroup->currentText());
}

WPAddContact::~WPAddContact()
{
}

void WPAddContact::slotUpdateGroups()
{
	kdDebug(14170) << "WPAddContact::slotUpdateGroups()" << endl;

	theDialog->mHostGroup->clear();
	QStringList Groups = theAccount->getGroups();
	QStringList::ConstIterator end = Groups.end();
	for (QStringList::ConstIterator i = Groups.begin(); i != end; i++)
		theDialog->mHostGroup->insertItem(SmallIcon("network"), *i);
	slotSelected(theDialog->mHostGroup->currentText());
}

void WPAddContact::slotSelected(const QString &Group)
{
	kdDebug(14170) << "WPAddContact::slotSelected(" << Group << ")" << endl;

	theDialog->mHostName->clear();
	QStringList Hosts = theAccount->getHosts(Group);
	QString ownHost = theAccount->myself()->contactId();
	QStringList::ConstIterator end = Hosts.end();
	for (QStringList::ConstIterator i = Hosts.begin(); i != end; i++)
		if (*i != ownHost) theDialog->mHostName->insertItem(SmallIcon("personal"), *i);
}

bool WPAddContact::validateData()
{
	kdDebug(14170) << "WPAddContact::validateData()" << endl;

	QString tmpHostName = theDialog->mHostName->currentText();

	if (tmpHostName.isEmpty()) {
		KMessageBox::sorry(this, i18n("<qt>You must enter a valid hostname.</qt>"), i18n("WinPopup"));
		return false;
	}

	// If our own host is not allowed as contact localhost should be forbidden as well,
	// additionally somehow localhost as contact crashes when receiving a message from it?? GF
	if (tmpHostName.upper() == QString::fromLatin1("LOCALHOST")) {
		KMessageBox::sorry(this, i18n("<qt>LOCALHOST is not allowed as contact.</qt>"), i18n("WinPopup"));
		return false;
	}

	return true;
}

bool WPAddContact::apply(Kopete::Account *theAccount, Kopete::MetaContact *theMetaContact)
{
	kdDebug(14170) << "WPAddContact::apply(" << theAccount << ", " << theMetaContact << ")" << endl;

	// TODO: make the nickname an option
	return theAccount->addContact(theDialog->mHostName->currentText(), theMetaContact, Kopete::Account::ChangeKABC );
}

#include "wpaddcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
