/*
    historypreferences.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#ifndef HISTORYPREFERENCES_H
#define HISTORYPREFERENCES_H


#include "kcmodule.h"

class msnPrefsUI;
class KAutoConfig;

/**
 * @author Olivier Goffart
 */
class HistoryPreferences : public KCModule
{
	Q_OBJECT

public:
	HistoryPreferences( QWidget *parent = 0, const char *name = 0, const QStringList &args = QStringList() );

	virtual void save();
	virtual void load();
	virtual void defaults();

private slots:
	void slotSettingsChanged();

private:
	KAutoConfig *kautoconfig;
};


#endif
