/*
    perlplugin.cpp

    Kopete Perl Scriping plugin

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

#include "perlscriptprefs.h"
#include "perlscriptprefsbase.h"
#include "addscriptdialog.h"

#include <qlayout.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <ktexteditor/editor.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <ktexteditor/document.h>

PerlScriptPreferences::PerlScriptPreferences(const QString &pixmap, QObject *parent): ConfigModule( i18n("Perl Scripting"), i18n("Perl Scripting Plugin"), pixmap, parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new PerlScriptPrefsUI(this);
	scriptsLoaded = 0;
	
	connect( preferencesDialog->addButton, SIGNAL(pressed()), this, SLOT(slotNewScript()) );
	connect( preferencesDialog->removeButton, SIGNAL(pressed()), this, SLOT(slotRemoveScript()) );
	connect( preferencesDialog->scriptView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(slotSelectionChanged(QListViewItem *)) );
}

PerlScriptPreferences::~PerlScriptPreferences()
{
}

void PerlScriptPreferences::slotNewScript()
{
	
	AddScriptDialog *dialog = new AddScriptDialog();
	dialog->show();
	connect( dialog, SIGNAL(scriptChosen(const QString &, const QString &, const QString &)), this, SLOT(slotAddScript( const QString &, const QString &, const QString & )) );
}

void PerlScriptPreferences::slotRemoveScript()
{
	QListViewItem *selectedScript = preferencesDialog->scriptView->currentItem();
	m_scripts.remove( selectedScript->text(2) );
	preferencesDialog->scriptView->takeItem( selectedScript );
	save();
	emit( scriptRemoved( selectedScript->text(2) ) );
	delete selectedScript;
}

void PerlScriptPreferences::slotAddScript( const QString &scriptPath, const QString &scriptName, const QString &scriptDesc, bool init )
{
	if( !m_scripts.contains( scriptPath ) )
	{
		QListViewItem *newScript = new QListViewItem( preferencesDialog->scriptView, scriptName, scriptDesc, scriptPath );
		emit( scriptAdded( scriptPath, scriptName, scriptDesc ) );
		m_scripts.append( scriptPath );
		if( !init )
		{
			save();
			preferencesDialog->scriptView->setSelected( newScript, true );
		}
	}
	else
	{
		KMessageBox::error( this, i18n("This script has already been loaded"), i18n("Script already loaded") );
	}
}

void PerlScriptPreferences::slotSelectionChanged( QListViewItem *selectedScript )
{
	preferencesDialog->editDocument->openURL( KURL( selectedScript->text(2) ) );
}

void PerlScriptPreferences::reopen()
{
	KConfig *config = KGlobal::config();
	config->setGroup( QString::fromLatin1("Perl Scripting Plugin") );
	m_scripts.clear();
	QString keyBase = QString::fromLatin1("Script");
	scriptsLoaded = 0;
	QString scriptNum = keyBase + QString::number(0);
	preferencesDialog->scriptView->clear();
	
	while( config->hasKey( scriptNum + QString::fromLatin1(" Name") ) )
	{
		kdDebug(14010) << k_funcinfo << "Loading " << scriptNum << endl;
		QString scriptName = config->readEntry( scriptNum + QString::fromLatin1(" Name") );
		QString scriptDesc = config->readEntry( scriptNum + QString::fromLatin1(" Desc") );
		QString scriptPath = config->readEntry( scriptNum + QString::fromLatin1(" Path") );
		
		slotAddScript( scriptPath, scriptName, scriptDesc, true );
		scriptsLoaded++;
		scriptNum = keyBase + QString::number(scriptsLoaded);
	}
	
	preferencesDialog->scriptView->setSelected( preferencesDialog->scriptView->firstChild(), true );
}

void PerlScriptPreferences::save()
{
	preferencesDialog->editDocument->save();
	emit( scriptModified( preferencesDialog->scriptView->currentItem()->text(2) ) );
	
	scriptsLoaded = 0;
	
	KConfig *config = KGlobal::config();
	config->deleteGroup( QString::fromLatin1("Perl Scripting Plugin") );
	config->setGroup( QString::fromLatin1("Perl Scripting Plugin") );
	QString keyBase = QString::fromLatin1("Script");
	
	QListViewItem *script = preferencesDialog->scriptView->firstChild();
	while( script )
	{
		QString scriptNum = keyBase + QString::number( scriptsLoaded );
		config->writeEntry( scriptNum + QString::fromLatin1(" Name"), script->text(0) );
		config->writeEntry( scriptNum + QString::fromLatin1(" Desc"), script->text(1) );
		config->writeEntry( scriptNum + QString::fromLatin1(" Path"), script->text(2) );
		scriptsLoaded++;
		script = script->nextSibling();
	}
	
	config->sync();
}
