/*
    spellcheckpreferences.cpp

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
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qcheckbox.h>

#include <kspell.h>
#include <kkeybutton.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>

#include "spellcheckpreferences.h"
#include "spellcheckprefs.h"

SpellCheckPreferences::SpellCheckPreferences(const QString &pixmap, QObject *parent): ConfigModule( i18n("Spell Checking"), i18n("Spell Checking Plugin"), pixmap, parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	preferencesDialog = new SpellCheckPrefsUI(this);

	( new QVBoxLayout( preferencesDialog->spellCheckFrame ) )->setAutoAdd( true );
	m_spellConfig = new KSpellConfig( preferencesDialog->spellCheckFrame  );
	m_spellConfig->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ) );

	( new QVBoxLayout( preferencesDialog->shortcutFrame ) )->setAutoAdd( true );
 	m_keyButton = new KKeyButton( preferencesDialog->shortcutFrame );
	m_keyButton->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ) );

	connect( m_keyButton, SIGNAL(capturedShortcut( const KShortcut & )), this, SLOT(slotShortcutChanged( const KShortcut & )) );

	connect( preferencesDialog->autoCheck, SIGNAL(clicked()), this, SLOT(slotAutoCheckChanged()) );

	autoCheck = true;

	reopen();
}

void SpellCheckPreferences::slotAutoCheckChanged()
{
	autoCheck = preferencesDialog->autoCheck->isChecked();
}

void SpellCheckPreferences::slotShortcutChanged( const KShortcut &newcut )
{
	m_keyButton->setShortcut( newcut );
	m_shortCut = newcut;
}

void SpellCheckPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup( QString::fromLatin1("Spell Checking Plugin") );

	config->writeEntry( QString::fromLatin1("Check As You Type"), autoCheck );
	config->writeEntry( QString::fromLatin1("Shortcut Key"), m_shortCut.toString() );

	config->writeEntry( QString::fromLatin1("NoRootAffix"), m_spellConfig->noRootAffix() );
	config->writeEntry( QString::fromLatin1("RunTogether"), m_spellConfig->runTogether() );
	config->writeEntry( QString::fromLatin1("Dictionary"), m_spellConfig->dictionary() );
	config->writeEntry( QString::fromLatin1("Encoding"), m_spellConfig->encoding() );
	config->writeEntry( QString::fromLatin1("Client"), m_spellConfig->client() );

	config->sync();

	reopen();

	emit( saved() );
}

void SpellCheckPreferences::reopen()
{
	KConfig *config = KGlobal::config();
	config->setGroup( QString::fromLatin1("Spell Checking Plugin") );

	autoCheck = config->readBoolEntry( QString::fromLatin1("Check As You Type"), autoCheck );
	KShortcut newCut = KShortcut( config->readEntry( QString::fromLatin1("Shortcut Key"), QString::fromLatin1("CTRL+ALT+S") ) );
	if( !newCut.isNull() )
		slotShortcutChanged( newCut );

	m_spellConfig-> setNoRootAffix( config->readBoolEntry( QString::fromLatin1("NoRootAffix"), false ) );
	m_spellConfig->setRunTogether( config->readBoolEntry( QString::fromLatin1("RunTogether"), false ) );
	QString dict = config->readEntry( QString::fromLatin1("Dictionary") );
	if( !dict.isNull() && !dict.isEmpty() )
		m_spellConfig->setDictionary( dict );

	m_spellConfig->setEncoding( config->readNumEntry( QString::fromLatin1("Encoding"), 0 ) );
	m_spellConfig->setClient( config->readNumEntry( QString::fromLatin1("Client"), 0 ) );

	preferencesDialog->autoCheck->setChecked( autoCheck );
}

#include "spellcheckpreferences.moc"
