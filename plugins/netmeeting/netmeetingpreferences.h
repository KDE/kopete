/***************************************************************************
                          netmeetingpreferences.h  -  description
                             -------------------
    copyright            : (C) 2004 by Olivier Goffart
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

#ifndef NetmeetingPREFERENCES_H
#define NetmeetingPREFERENCES_H

#include <kcmodule.h>
#include <qstring.h>

class NetmeetingPrefsUI;

/**
  *@author Olivier Goffart
  */

class NetmeetingPreferences : public KCModule  {
   Q_OBJECT
public:

	NetmeetingPreferences(QWidget *parent = 0, const char* name = 0, const QStringList &args = QStringList());
	~NetmeetingPreferences();

	virtual void save();
	virtual void load();

private:
	NetmeetingPrefsUI *preferencesDialog;
	
private slots:
	void slotChanged();
};

#endif
