/*
    translatorpreferences.h

    Copyright (c) 2010      by Igor Poboiko <igor.poboiko@gmail.com>
    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2003-2010 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef TRANSLATORPREFERENCES_H
#define TRANSLATORPREFERENCES_H

#include <kcmodule.h>

#include "translatorlanguages.h"

namespace Ui { class TranslatorPrefsUI; }

/**
 * @author Igor Poboiko
 */
class TranslatorPreferences : public KCModule
{
	Q_OBJECT
	public:
		explicit TranslatorPreferences(QWidget *parent=0, const QVariantList &args = QVariantList());
		~TranslatorPreferences();

		virtual void save();
		virtual void load();

	private slots:
		void updateLanguageList();
		void slotModified();
		void slotShowPreviousChanged(bool);

	signals:
		void preferencesChanged();

	private:
		Ui::TranslatorPrefsUI *p;

		TranslatorLanguages *m_languages;
};

#endif // TRANSLATORPREFERENCES_H
