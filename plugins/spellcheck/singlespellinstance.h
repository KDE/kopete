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
#include <qobject.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qregexp.h>

class KSpell;
class KopeteView;
class QTextEdit;
class SpellCheckPreferences;

typedef QMap<QString,QStringList> ReplacementMap;

class SingleSpellInstance : public QObject
{
	Q_OBJECT

	public:
		SingleSpellInstance( KopeteView*, SpellCheckPreferences* );
		~SingleSpellInstance();

	private:
		KopeteView *mView;
		QTextEdit *t;
		QRegExp mBound;
		QStringList *words;
		ReplacementMap mReplacements;
		bool spellCheckerReady;
		KSpell *mSpell;

	private slots:
		void slotSpellCheckerReady(KSpell *);
		void slotMisspelling( const QString &, const QStringList &, unsigned int );
		void slotUpdateTextEdit();
		void slotViewDestroyed();

	protected:
		virtual bool eventFilter( QObject *watched, QEvent *e );
};
