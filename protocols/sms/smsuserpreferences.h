
#ifndef SMSUSERPREFERENCES_H
#define SMSUSERPREFERENCES_H

#include <kdialogbase.h>
#include <qvbox.h>

class SMSPreferencesBase;
class SMSUserPrefsUI;

class SMSUserPreferences : public KDialogBase
{
	Q_OBJECT
public:
	SMSUserPreferences(const QString user);
	~SMSUserPreferences();
private:
	SMSPreferencesBase* prefBase;
	SMSUserPrefsUI* userPrefs;
	QString m_userId;
	QVBox* topWidget;
signals:
	void updateUserId(const QString newId);
public slots:
	void slotOk();
	void slotApply();
	void slotCancel();
} ;

#endif //SMSUSERPREFERENCES_H
