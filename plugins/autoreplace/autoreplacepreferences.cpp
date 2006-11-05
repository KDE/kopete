/***************************************************************************
                          autoreplacepreferences.cpp  -  description
                             -------------------
    begin                : 20030426
    copyright            : (C) 2003 by Roberto Pariset
    email                : victorheremita@fastwebnet.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlistview.h>

#include <klocale.h>
#include <klineedit.h>
#include <kglobal.h>
#include <kgenericfactory.h>
#include <kautoconfig.h>

#include "autoreplaceprefs.h"
#include "autoreplacepreferences.h"
#include "autoreplaceconfig.h"

typedef KGenericFactory<AutoReplacePreferences> AutoReplacePreferencesFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_kopete_autoreplace, AutoReplacePreferencesFactory( "kcm_kopete_autoreplace" ) )

AutoReplacePreferences::AutoReplacePreferences( QWidget *parent, const char * /* name */, const QStringList &args )
: KCAutoConfigModule( AutoReplacePreferencesFactory::instance(), parent, args )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new AutoReplacePrefsUI( this );

	// creates table columns (avoids new columns every time)
	preferencesDialog->m_list->addColumn( i18n("Text" ) );
	preferencesDialog->m_list->addColumn( i18n("Replacement" ) );
	preferencesDialog->m_list->header()->setStretchEnabled( true , 1 );
	
	// connect SIGNALS/SLOTS
	connect( preferencesDialog->m_add, SIGNAL(pressed()),
		SLOT( slotAddCouple()) );
	connect( preferencesDialog->m_edit, SIGNAL(pressed()),
		SLOT( slotEditCouple()) );
	connect( preferencesDialog->m_remove, SIGNAL(pressed()),
		SLOT(slotRemoveCouple()) );
	connect( preferencesDialog->m_list, SIGNAL(selectionChanged()),
		SLOT(slotSelectionChanged()) );
	connect( preferencesDialog->m_key, SIGNAL(textChanged ( const QString & )),
		SLOT( slotEnableAddEdit( const QString & )) ); 

	m_wordListChanged = false;

	// Sentence options and which messages to apply autoreplace to
	// are managed by KCMAutoConfigModule.  The list of replacements
	// itself is manually read/written as KCMAutoConfigModule doesn't support it.
	autoConfig()->ignoreSubWidget( preferencesDialog->replacementsGroup );
	setMainWidget( preferencesDialog, "AutoReplace Plugin" );
	
	m_config = new AutoReplaceConfig;
	load();
}

AutoReplacePreferences::~AutoReplacePreferences()
{
	delete m_config;
}

// reload configuration reading it from kopeterc
void AutoReplacePreferences::load()
{
	m_config->load();

	// Removes and deletes all the items in this list view and triggers an update
	preferencesDialog->m_list->clear();

	// show keys/values on gui
	AutoReplaceConfig::WordsToReplace::Iterator it;
	AutoReplaceConfig::WordsToReplace map = m_config->map();
	for ( it = map.begin(); it != map.end(); ++it )
	{
		// notice: insertItem is called automatically by the constructor
		new QListViewItem( preferencesDialog->m_list, it.key(), it.data() );
	}

	m_wordListChanged = false;
	KCAutoConfigModule::load();
}

// save list to kopeterc and creates map out of it
void AutoReplacePreferences::save()
{
	// make a list reading all values from gui
	AutoReplaceConfig::WordsToReplace newWords;
	for ( QListViewItem * i = preferencesDialog->m_list->firstChild(); i != 0; i = i->nextSibling() )
		newWords[ i->text( 0 ) ] = i->text( 1 );

	// save the words list
	m_config->setMap( newWords );
	m_config->save();

	m_wordListChanged = false;
	KCAutoConfigModule::save();
}

// read m_key m_value, create a QListViewItem
void AutoReplacePreferences::slotAddCouple()
{
	QString k = preferencesDialog->m_key->text();
	QString v = preferencesDialog->m_value->text();
	if ( !k.isEmpty() && !k.isNull() && !v.isEmpty() && !v.isNull() )
	{
		QListViewItem * lvi;
		QListViewItem * oldLvi = 0;
		// see if we are replacing an existing entry
		if ( ( oldLvi = preferencesDialog->m_list->findItem( k, 0 ) ) )
			delete oldLvi;
		lvi = new QListViewItem( preferencesDialog->m_list, k, v );
		// Triggers a size, geometry and content update
		// during the next iteration of the event loop
		preferencesDialog->m_list->triggerUpdate();
		// select last added
		preferencesDialog->m_list->setSelected( lvi, true );
	}

	m_wordListChanged = true;
	slotWidgetModified();
}

// edit the selected item
void AutoReplacePreferences::slotEditCouple()
{
	QString k = preferencesDialog->m_key->text();
	QString v = preferencesDialog->m_value->text();
	QListViewItem * lvi;
	if ( ( lvi = preferencesDialog->m_list->selectedItem() ) && !k.isEmpty() && !k.isNull() && !v.isEmpty() && !v.isNull() )
	{
		lvi->setText( 0, k );
		lvi->setText( 1, v );
		preferencesDialog->m_list->triggerUpdate();
		m_wordListChanged = true;
		slotWidgetModified();
	}
}

// Returns a pointer to the selected item if the list view is in
// Single selection mode and an item is selected
void AutoReplacePreferences::slotRemoveCouple()
{
	delete preferencesDialog->m_list->selectedItem();

	m_wordListChanged = true;
	slotWidgetModified();
}

void AutoReplacePreferences::slotEnableAddEdit( const QString & keyText )
{
	preferencesDialog->m_add->setEnabled( !keyText.isEmpty() );
	preferencesDialog->m_edit->setEnabled( !keyText.isEmpty() && preferencesDialog->m_list->selectedItem() );
}

void AutoReplacePreferences::slotSelectionChanged()
{
	QListViewItem *selection = 0;
	if ( ( selection = preferencesDialog->m_list->selectedItem() ) )
	{
		// enable the remove button
		preferencesDialog->m_remove->setEnabled( true );
		// put the selection contents into the text entry widgets so they can be edited
		preferencesDialog->m_key->setText( selection->text( 0 ) );
		preferencesDialog->m_value->setText( selection->text( 1 ) );
	}
	else
	{
		preferencesDialog->m_remove->setEnabled( false );
		preferencesDialog->m_key->clear();
		preferencesDialog->m_value->clear();
	}
}

void AutoReplacePreferences::slotWidgetModified()
{
	emit KCModule::changed( m_wordListChanged || autoConfig()->hasChanged() );
}

void AutoReplacePreferences::defaults()
{
    KCAutoConfigModule::defaults();
    preferencesDialog->m_list->clear();
    m_config->loadDefaultAutoReplaceList();
    AutoReplaceConfig::WordsToReplace::Iterator it;
    AutoReplaceConfig::WordsToReplace map = m_config->map();
    for ( it = map.begin(); it != map.end(); ++it )
    {
        // notice: insertItem is called automatically by the constructor
        new QListViewItem( preferencesDialog->m_list, it.key(), it.data() );
    }
    m_wordListChanged = true;
    slotWidgetModified();
}

#include "autoreplacepreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:

