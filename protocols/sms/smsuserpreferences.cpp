#include "smsuserpreferences.h"
#include "smspreferencesbase.h"
#include "smsuserprefs.h"
#include "smscontact.h"

#include <qcheckbox.h>

#include <klocale.h>
#include <kconfig.h>
#include <klineedit.h>

SMSUserPreferences::SMSUserPreferences( SMSContact* contact )
	: KDialogBase( 0L, "userPrefs", true, i18n("User preferences"), Ok|Apply|Cancel, Ok, true )
{
	m_contact = contact;
	topWidget = makeVBoxMainWidget();
	userPrefs = new SMSUserPrefsUI( topWidget );
	prefBase = new SMSPreferencesBase( contact, topWidget );

	if (m_contact->serviceName() != QString::null)
	{
		prefBase->setEnabled(true);
		userPrefs->uSpecific->setChecked(true);
	}
	else
		prefBase->setEnabled(false);

	userPrefs->telNumber->setText(m_contact->phoneNumber());

	connect (userPrefs->uSpecific, SIGNAL(toggled(bool)), prefBase, SLOT(setEnabled(bool)));
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
	if (userPrefs->uSpecific->isOn())
		prefBase->save();
	else
		m_contact->clearServicePrefs();

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

