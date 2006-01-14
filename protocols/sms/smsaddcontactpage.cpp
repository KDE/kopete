/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *************************************************************************
*/

/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "smsadd.h"
#include "smsaddcontactpage.h"
#include "kopeteaccount.h"

#include <qlayout.h>
#include <qlineedit.h>



SMSAddContactPage::SMSAddContactPage(QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	smsdata = new smsAddUI(this);
}

SMSAddContactPage::~SMSAddContactPage()
{

}

bool SMSAddContactPage::apply(Kopete::Account* a, Kopete::MetaContact* m)
{
	if ( validateData() )
	{
		QString nr = smsdata->addNr->text();
		QString name = smsdata->addName->text();

		return a->addContact( nr, m, Kopete::Account::ChangeKABC );
	}

	return false;
}


bool SMSAddContactPage::validateData()
{
	return true;
}

#include "smsaddcontactpage.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

