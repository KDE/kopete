#include "smsuserpreferences.h"
#include "smspreferencesbase.h"
#include "smsuserprefs.h"

#include <qcheckbox.h>

#include <klocale.h>
#include <kconfig.h>
#include <klineedit.h>

SMSUserPreferences::SMSUserPreferences( const QString userId )
	: KDialogBase( 0L, "userPrefs", true, i18n("User preferences"), Ok|Apply|Cancel, Ok, true )
{
	m_userId = userId;
	topWidget = makeVBoxMainWidget();
	userPrefs = new SMSUserPrefsUI( topWidget );
	prefBase = new SMSPreferencesBase( userId, topWidget );

	if (KGlobal::config()->hasGroup(QString("SMS:%1").arg(m_userId)))
	{
		prefBase->setEnabled(KGlobal::config()->hasGroup(QString("SMS:%1").arg(m_userId)));
		userPrefs->uSpecific->setChecked(true);
	}

	userPrefs->telNumber->setText(userId);

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
	if (userPrefs->telNumber->text() != m_userId)
		emit updateUserId(userPrefs->telNumber->text());
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

