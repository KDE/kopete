/***************************************************************************
                          texteffectpreferences.h  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TextEffectPREFERENCES_H
#define TextEffectPREFERENCES_H

#include <kcmodule.h>

class TextEffectPrefs;
class TextEffectConfig;
class QStringList;

/**
  *@author Olivier Goffart
  */

class TextEffectPreferences : public KCModule  {
   Q_OBJECT
public:

	TextEffectPreferences(QWidget *parent = 0, const char* name = 0, const QStringList &args = QStringList());
	~TextEffectPreferences();

	// Overloaded from parent
	virtual void save();
	virtual void load();
    virtual void defaults();

private:
	QStringList colors();
	TextEffectPrefs *preferencesDialog;
	TextEffectConfig *config;

private slots: // Public slots
	void slotAddPressed();
	void slotRemovePressed();
	void slotUpPressed();
	void slotDownPressed();
	void slotSettingChanged();

};

#endif

