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

// Kopete Includes
#include <addcontactpage.h>

// Local Includes
#include "wpaddcontactbase.h"
#include "wpaccount.h"
#include "wpdebug.h"
#include "wpaddcontact.h"

WPAddContact::WPAddContact(WPProtocol *owner, WPAccount *newAccount, QWidget *parent, const char *name): AddContactPage(parent, name)
{
	DEBUG(WPDMETHOD, "WPAddContact::WPAddContact(<owner>, " << newAccount << ", <parent>, " << name << ")");

	(new QVBoxLayout(this))->setAutoAdd(true);
	theDialog = new WPAddContactBase(this);
	connect(theDialog->mHostGroup, SIGNAL(activated(const QString &)), this, SLOT(slotSelected(const QString &)));
	connect(theDialog->mRefresh, SIGNAL(clicked()), this, SLOT(slotUpdateGroups()));
	theDialog->show();
	theProtocol = owner;
	theAccount = newAccount;

	slotUpdateGroups();
	slotSelected(theDialog->mHostGroup->currentText());
}

WPAddContact::~WPAddContact()
{
	DEBUG(WPDMETHOD, "WPAddContact::~WPAddContact()");
}

void WPAddContact::slotUpdateGroups()
{
	DEBUG(WPDMETHOD, "WPAddContact::slotUpdateGroups()");

	theDialog->mHostGroup->clear();
    QStringList Groups = theAccount->getGroups();
	for(QStringList::Iterator i = Groups.begin(); i != Groups.end(); i++)
		theDialog->mHostGroup->insertItem(SmallIcon("network"), *i);
	slotSelected(theDialog->mHostGroup->currentText());
}

void WPAddContact::slotSelected(const QString &Group)
{
	DEBUG(WPDMETHOD, "WPAddContact::slotSelected(" << Group << ")");

	theDialog->mHostName->clear();
	QStringList Hosts = theAccount->getHosts(Group);
	for(QStringList::Iterator i = Hosts.begin(); i != Hosts.end(); i++)
		theDialog->mHostName->insertItem(SmallIcon("personal"), *i);
}

bool WPAddContact::validateData()
{
	DEBUG(WPDMETHOD, "WPAddContact::validateData()");

	return !theDialog->mHostName->currentText().isEmpty();
}

bool WPAddContact::apply(Kopete::Account *theAccount, Kopete::MetaContact *theMetaContact)
{
	DEBUG(WPDMETHOD, "WPAddContact::apply(" << theAccount << ", " << theMetaContact << ")");

	// TODO: make the displayname an option
	theAccount->addContact(theDialog->mHostName->currentText(), theDialog->mHostName->currentText(), theMetaContact, Kopete::Account::ChangeKABC );
	DEBUG(WPDMETHOD, "WPAddContact::apply()");
	return true;
}

#include "wpaddcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

