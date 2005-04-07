/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
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

#include <qlabel.h>

#include <klocale.h>
#include <klineedit.h>

#include "smsuserpreferences.h"
#include "smsuserprefs.h"
#include "smscontact.h"

SMSUserPreferences::SMSUserPreferences( SMSContact* contact )
	: KDialogBase( 0L, "userPrefs", true, i18n("User Preferences"), Ok|Cancel, Ok, true )
{
	m_contact = contact;
	topWidget = makeVBoxMainWidget();
	userPrefs = new SMSUserPrefsUI( topWidget );

	userPrefs->telNumber->setText(m_contact->phoneNumber());
	userPrefs->title->setText(m_contact->nickName());
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

