/***************************************************************************
                          wppreferences.cpp  -  description
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net

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

// Local Includes
#include "wpaddcontactbase.h"
#include "wpprotocol.h"
#include "wpdebug.h"
#include "wpaddcontact.h"

// Kopete Includes
#include <addcontactpage.h>

// QT Includes
#include <qwidget.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qcheckbox.h>

// KDE Includes
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <klocale.h>
#include <kurlrequester.h>


// WinPopup Preferences
WPAddContact::WPAddContact(WPProtocol *owner, QWidget *parent, const char *name): AddContactPage(parent, name)
{
	DEBUG(WPDMETHOD, "WPAddContact::WPAddContact(<owner>, <parent>, " << name << ")");

	(new QVBoxLayout(this))->setAutoAdd(true);
	theDialog = new WPAddContactBase(this);
	connect(theDialog->HostGroup, SIGNAL(activated(const QString &)), this, SLOT(slotSelected(const QString &)));
	connect(theDialog->Refresh, SIGNAL(clicked()), this, SLOT(slotUpdateGroups()));
	theDialog->show();
	theProtocol = owner;

	slotUpdateGroups();
	slotSelected(theDialog->HostGroup->currentText());
}

// Destructor
WPAddContact::~WPAddContact()
{
	DEBUG(WPDMETHOD, "WPAddContact::~WPAddContact()");
}

void WPAddContact::slotUpdateGroups()
{
	DEBUG(WPDMETHOD, "WPAddContact::slotUpdateGroups()");

	theDialog->HostGroup->clear();
    QStringList Groups = theProtocol->getGroups();
	for(QStringList::Iterator i = Groups.begin(); i != Groups.end(); i++)
		theDialog->HostGroup->insertItem(SmallIcon("network"), *i);
	slotSelected(theDialog->HostGroup->currentText());
}

void WPAddContact::slotSelected(const QString &Group)
{
	DEBUG(WPDMETHOD, "WPAddContact::slotSelected(" << Group << ")");

	theDialog->HostName->clear();
	QStringList Hosts = theProtocol->getHosts(Group);
	for(QStringList::Iterator i = Hosts.begin(); i != Hosts.end(); i++)
		theDialog->HostName->insertItem(SmallIcon("personal"), *i);
}

bool WPAddContact::validateData()
{
	DEBUG(WPDMETHOD, "WPAddContact::validateData()");
    
	return theDialog->HostName->currentText() != "";
}

void WPAddContact::slotFinish(KopeteMetaContact *theMetaContact)
{
	DEBUG(WPDMETHOD, "WPAddContact::slotFinish()");

	theProtocol->getContact(theDialog->HostName->currentText(), theMetaContact);
}

#include "wpaddcontact.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

