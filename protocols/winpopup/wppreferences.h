/***************************************************************************
                          wppreferences.h  -  description
                             -------------------
    begin                : Wed Jan 23 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __WPPREFERENCES_H
#define __WPPREFERENCES_H

// Local Includes
#include "wppreferencesbase.h"

// Kopete Includes
#include <configmodule.h>

// QT Includes

// KDE Includes

class WPProtocol;

class WPPreferences : public ConfigModule
{
	Q_OBJECT
private:
	WPProtocol *theProtocol;
	WPPreferencesBase *preferencesDialog;	// Preferences Dialog

public:
	WPPreferences(const QString & pixmap, QObject * parent = 0);
	~WPPreferences();
	virtual void save();	// save preferences method

public slots:
	void installSamba();

signals:
	void saved();			// Parent slot saved
};

#endif
