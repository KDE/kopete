/*
    nowlisteningpreferences.h

    Kopete Now Listening To plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef NOWLISTENINGPREFERENCES_H
#define NOWLISTENINGPREFERENCES_H

//#include <qcheckbox.h>
//#include <qspinbox.h>

//#include <qwidget.h>
//#include <klineedit.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>

#include "configmodule.h"
#include "nowlisteningprefs.h"

/**
  *@author Will Stephenson
  */

class NowListeningPreferences : public ConfigModule
{
Q_OBJECT
public:
	NowListeningPreferences(const QString &pixmap,QObject *parent=0);
	~NowListeningPreferences();
	virtual void save();

	int pollFrequency() { return preferencesDialog->m_freq->value(); };
	QString message() { return preferencesDialog->m_message->text(); };

signals:
	void saved();

private:
	NowListeningPrefsUI *preferencesDialog;	

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

