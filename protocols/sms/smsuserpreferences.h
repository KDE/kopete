
#ifndef SMSUSERPREFERENCES_H
#define SMSUSERPREFERENCES_H

#include <kdialogbase.h>
#include <qvbox.h>

class SMSPreferencesBase;
class SMSUserPrefsUI;
class SMSContact;

class SMSUserPreferences : public KDialogBase
{
	Q_OBJECT
public:
	SMSUserPreferences(SMSContact* contact);
	~SMSUserPreferences();
private:
	SMSPreferencesBase* prefBase;
	SMSUserPrefsUI* userPrefs;
	QVBox* topWidget;

	SMSContact* m_contact;
public slots:
	void slotOk();
	void slotApply();
	void slotCancel();
} ;

#endif //SMSUSERPREFERENCES_H
