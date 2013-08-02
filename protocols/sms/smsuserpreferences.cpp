/*
  smsuserpreferences.cpp  -  SMS Plugin User Preferences

  Copyright (c) 2003      by Richard Lärkäng        <nouseforaname@home.se>
  Copyright (c) 2003      by Gav Wood               <gav@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "smsuserpreferences.h"

#include <qlabel.h>

#include <klocale.h>
#include <klineedit.h>

#include "smsuserprefs.h"
#include "smscontact.h"

SMSUserPreferences::SMSUserPreferences( SMSContact* contact )
	: KDialog( 0L)
{
	m_contact = contact;
	setCaption(i18n("User Preferences"));
	setButtons(Ok|Cancel);
	setDefaultButton(Ok);
	setModal(true);
	showButtonSeparator(true);
	topWidget = new KVBox(this);
	setMainWidget(topWidget);
	userPrefs = new SMSUserPrefsUI( topWidget );

	userPrefs->telNumber->setText(m_contact->phoneNumber());
	userPrefs->title->setText(m_contact->displayName());
	connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
	connect(this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
}

SMSUserPreferences::~SMSUserPreferences()
{

}

void SMSUserPreferences::slotOk()
{
	if (userPrefs->telNumber->text() != m_contact->phoneNumber())
		m_contact->setPhoneNumber(userPrefs->telNumber->text());
	slotCancel();
}

void SMSUserPreferences::slotCancel()
{
	deleteLater();
}

#include "smsuserpreferences.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

