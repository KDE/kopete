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
class KSpell;
class KopeteView;

class SpellCheckPlugin : public KopetePlugin
{
	Q_OBJECT

	public:
		static SpellCheckPlugin  *plugin();
		SpellCheckPlugin( QObject *parent, const char *name, const QStringList &args );
		~SpellCheckPlugin();
		virtual KActionCollection *customChatActions( KopeteMessageManager * );
		virtual KActionCollection *customToolbarActions();

	private:
		static SpellCheckPlugin* pluginStatic_;
		KActionCollection *m_actionCollection;
		KopeteView *mView;
		KopeteMessage mBuffer;
		KSpell *mSpell;

	private slots:
		void slotCheckSpelling();
		void slotSpellCheckerReady(KSpell *);
		void slotViewDestroyed();
		void slotCorrection( const QString &, const QString &, unsigned int );
		void slotSpellDone();
		//void slotSpellCheckCorrection();
};
