#include <qlabel.h>

#include <klocale.h>
#include <klineedit.h>

#include "smsuserpreferences.h"
#include "smsuserprefs.h"
#include "smscontact.h"

SMSUserPreferences::SMSUserPreferences( SMSContact* contact )
	: KDialogBase( 0L, "userPrefs", true, i18n("User Preferences"), Ok|Apply|Cancel, Ok, true )
{
	m_contact = contact;
	topWidget = makeVBoxMainWidget();
	userPrefs = new SMSUserPrefsUI( topWidget );

	userPrefs->telNumber->setText(m_contact->phoneNumber());
	userPrefs->title->setText(m_contact->displayName());
}

SMSUserPreferences::~SMSUserPreferences()
{

}

void SMSUserPreferences::slotOk()
{
	slotApply();
	slotCancel();
}

void SMSUserPreferences::slotApply()
{
	if (userPrefs->telNumber->text() != m_contact->phoneNumber())
		m_contact->setPhoneNumber(userPrefs->telNumber->text());
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

