/*
    spellcheckpreferences.h

    Kopete Spell Checking plugin

    Copyright (c) 2003 by Jason Keirstead   <jason@keirstead.org>

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

#ifndef SPELLCHECKPREFERENCES_H
#define SPELLCHECKPREFERENCES_H

#include <kshortcut.h>

#include "configmodule.h"

class KSpellConfig;
class KKeyButton;
class SpellCheckPrefsUI;

class SpellCheckPreferences : public ConfigModule
{
	Q_OBJECT
	public:
		SpellCheckPreferences(const QString &pixmap,QObject *parent=0);
		virtual void save();
		virtual void reopen();
		KSpellConfig *spellConfig() { return m_spellConfig; }
		bool autoCheckEnabled() { return autoCheck; }
		const KShortcut &shortCut() { return m_shortCut; }

	signals:
		void saved();

	private:
		SpellCheckPrefsUI *preferencesDialog;
		KSpellConfig *m_spellConfig;
		KKeyButton *m_keyButton;
		KShortcut m_shortCut;
		bool autoCheck;

	private slots:
		void slotAutoCheckChanged();
		void slotShortcutChanged( const KShortcut & );

};

#endif
