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
#include "configmodule.h"
#include "translatorprefs.h"

/**
  *@author duncan
  */
class QString;
class TranslatorPrefsUI;

class TranslatorPreferences : public ConfigModule
{
Q_OBJECT
public:
	TranslatorPreferences(const QString &pixmap,QObject *parent=0);
	~TranslatorPreferences();
   virtual void save();

	void addLanguage( const QString &key, const QString &name);
	const QString &myLang();

signals:
	void saved();

private:
	TranslatorPrefsUI *preferencesDialog;	
	/* int to lang key and viceversa*/
    QMap<int, QString> m_langMap;
	QMap<QString, int> m_keyMap;
    /* key to description */
    QMap<QString, QString> m_descMap;
	/* Lang counter */
	int m_lc;
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

