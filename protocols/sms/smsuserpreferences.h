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
	void slotCancel();
} ;

#endif //SMSUSERPREFERENCES_H
