/***************************************************************************
                          cryptographypreferences.h  -  description
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

#ifndef CryptographyPREFERENCES_H
#define CryptographyPREFERENCES_H

#include "configmodule.h"

class CryptographyPrefsUI;

/**
  *@author Olivier Goffart
  */

class CryptographyPreferences : public ConfigModule  {
   Q_OBJECT
public:
	CryptographyPreferences(const QString &pixmap, QObject *parent=0);
	~CryptographyPreferences();

	virtual void save();
	virtual void reopen();

	const QString &privateKey();

private:
	CryptographyPrefsUI *preferencesDialog;
	QString m_signKeyID;
	QString m_signKeyMail;
public slots: // Public slots
  /** No descriptions */
  void slotSelectPressed();
};

#endif
