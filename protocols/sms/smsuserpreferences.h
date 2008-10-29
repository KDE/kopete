/*
  smsuserpreferences.h  -  SMS Plugin User Preferences

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

#ifndef SMSUSERPREFERENCES_H
#define SMSUSERPREFERENCES_H

#include <kdialog.h>
#include <kvbox.h>

class SMSPreferencesBase;
class SMSUserPrefsUI;
class SMSContact;

class SMSUserPreferences : public KDialog
{
	Q_OBJECT
public:
	SMSUserPreferences(SMSContact* contact);
	~SMSUserPreferences();
private:
	SMSPreferencesBase* prefBase;
	SMSUserPrefsUI* userPrefs;
	KVBox* topWidget;

	SMSContact* m_contact;
public slots:
	void slotOk();
	void slotCancel();
} ;

#endif //SMSUSERPREFERENCES_H
