/*
    translatorprefs.h

    Kopete Translatorfish Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

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

#ifndef BABELPREFERENCES_H
#define BABELPREFERENCES_H

#include <qmap.h>
#include <qcombobox.h>

#include <qwidget.h>
#include <klineedit.h>
#include <kcmodule.h>
//#include "translatorplugin.h"

class QString;
class TranslatorPrefsUI;
class TranslatorLanguages;
class KAutoConfig;

/**
  * @author Duncan Mac-Vicar Prett   <duncan@kde.org>
  *
  * Translator Plugin Preferences
  *
  */

class TranslatorPreferences : public KCModule
{
Q_OBJECT
public:
	TranslatorPreferences(QWidget *parent = 0, const char *name = 0, const QStringList &args = QStringList());
	~TranslatorPreferences();

	virtual void save();
	virtual void defaults();

private:
	TranslatorPrefsUI *preferencesDialog;
	KAutoConfig *kautoconfig;
	TranslatorLanguages *languages;

private slots:
	void widgetModified();
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

