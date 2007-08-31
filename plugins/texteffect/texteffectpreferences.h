/*
    texteffectpreferences.h  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef TextEffectPREFERENCES_H
#define TextEffectPREFERENCES_H

#include <kcmodule.h>

namespace Ui { class TextEffectPrefs; }
class TextEffectConfig;
class QStringList;

/**
  *@author Olivier Goffart
  */

class TextEffectPreferences : public KCModule  {
   Q_OBJECT
public:

	explicit TextEffectPreferences(QWidget *parent = 0, const QVariantList &args = QVariantList());
	~TextEffectPreferences();

	// Overloaded from parent
	virtual void save();
	virtual void load();
    virtual void defaults();

private:
	QStringList colors();
	Ui::TextEffectPrefs *preferencesDialog;
	TextEffectConfig *config;

private slots: // Public slots
	void slotAddPressed();
	void slotRemovePressed();
	void slotUpPressed();
	void slotDownPressed();
	void slotSettingChanged();

};

#endif

