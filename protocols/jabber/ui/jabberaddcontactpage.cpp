
/***************************************************************************
                          jabberaddcontactpage.cpp  -  Add contact widget
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
                           (C) 2003 by Daniel Stone <dstone@kde.org>
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <klineedit.h>

#include <klocale.h>

#include "jabberaddcontactpage.h"

JabberAddContactPage::JabberAddContactPage (KopeteAccount * owner, QWidget * parent, const char *name):AddContactPage (parent, name)
{
	(new QVBoxLayout (this))->setAutoAdd (true);
	if (owner->isConnected ())
	{
		jabData = new dlgAddContact (this);
		jabData->show ();

		canadd = true;

	}
	else
	{
		noaddMsg1 = new QLabel (i18n ("You need to be connected to be able to add contacts."), this);
		noaddMsg2 = new QLabel (i18n ("Connect to the Jabber network and try again."), this);
		canadd = false;
	}

}

JabberAddContactPage::~JabberAddContactPage ()
{
}

bool JabberAddContactPage::validateData ()
{
	return true;
}


bool JabberAddContactPage::apply (KopeteAccount * i, KopeteMetaContact * m)
{

	if(canadd)
		if (validateData ())
			return static_cast<JabberAccount *>(i)->addContact(jabData->addID->text(), jabData->addID->text(), m, KopeteAccount::ChangeKABC );

	return false;
}

#include "jabberaddcontactpage.moc"

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
