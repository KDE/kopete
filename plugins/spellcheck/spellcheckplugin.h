/*
    spellcheckplugin.h

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
#include "kopeteplugin.h"
#include "kopetemessage.h"

class KActionCollection;
class KopeteMessageManager;
class KopeteMessage;
class KSpell;
class SpellCheckPreferences;
class SingleSpellInstance;

class SpellCheckPlugin : public KopetePlugin
{
	Q_OBJECT

	public:
		static SpellCheckPlugin  *plugin();
		SpellCheckPlugin( QObject *parent, const char *name, const QStringList &args );
		~SpellCheckPlugin();
		virtual KActionCollection *customChatActions( KopeteMessageManager * );
		KSpell *speller();
		QPtrList<SingleSpellInstance> singleSpellers;

	private:
		static SpellCheckPlugin* pluginStatic_;
		KActionCollection *m_actionCollection;
		KSpell *mSpell;
		KopeteMessage mBuffer;
		SpellCheckPreferences *mPrefs;
		bool spellCheckerReady;
		bool manualCheckInProgress;

	private slots:
		void slotPrefsSaved();
		void slotCheckSpelling();
		void slotBindToView( KopeteView * );
		void slotSpellCheckerReady(KSpell *);
		void slotCorrection( const QString &, const QString &, unsigned int );
		void slotMisspelling( const QString &, const QStringList &, unsigned int );
		void slotSpellDone( const QString & );
};
