/***************************************************************************
                          yahooprefs.h  -  description
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Bruno Rodrigues
    email                : bruno.rodrigues@litux.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef YAHOOPREFS_H
#define YAHOOPREFS_H


// Local Includes

// Kopete Includes
#include <configmodule.h>
#include <kconfig.h>

#include <dlgpreferences.h>

// QT Includes

// KDE Includes


// Yahoo Preferences
class YahooPreferences : public ConfigModule
{
	Q_OBJECT
public:

	YahooPreferences(const QString &pixmap, QObject *parent = 0);
	~YahooPreferences();

	/**
	 * Method that saves preferences
	 */
	virtual void save();	// save preferences method

signals:

	void saved();	// Parent slot saved

private:

	dlgPreferences *m_yahooPrefsDialog;	// Preferences Dialog
	KConfig *m_config;
};

#endif


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

