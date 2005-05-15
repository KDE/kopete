/***************************************************************************
                          cryptographypreferences.h
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

#ifndef CryptographyPREFERENCES_H
#define CryptographyPREFERENCES_H

#include "kcautoconfigmodule.h"

class CryptographyPrefsUI;
class KAutoConfig;

/**
 * Preference widget for the Cryptography plugin
 * @author Olivier Goffart
 */
class CryptographyPreferences : public KCAutoConfigModule  {
   Q_OBJECT
public:
	CryptographyPreferences(QWidget *parent = 0, const char *name = 0, const QStringList &args = QStringList());
private:
	CryptographyPrefsUI *preferencesDialog;
private slots: // Public slots
	void slotSelectPressed();
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
