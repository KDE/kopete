/***************************************************************************
                          texteffectpreferences.h  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
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

#include "configmodule.h"

class TextEffectPrefs;

/**
  *@author Olivier Goffart
  */

class TextEffectPreferences : public ConfigModule  {
   Q_OBJECT
public:

	TextEffectPreferences(const QString &pixmap, QObject *parent=0);
	~TextEffectPreferences();

	virtual void save();
	virtual void reopen();
	
	QStringList colors();
	bool color_lines();
	bool color_words();
	bool color_char();
	bool color_random();
	bool lamer();
	bool waves();


private:
	TextEffectPrefs *preferencesDialog;
public slots: // Public slots

};

#endif

