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

#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qmap.h>
#include <qstring.h>
#include <qlistview.h>

#include <klocale.h>
#include <klineedit.h>
#include <keditlistbox.h>
#include <kconfig.h>
#include <kglobal.h>

#include "autoreplaceprefs.h"
#include "autoreplacepreferences.h"


AutoReplacePreferences::AutoReplacePreferences(	const QString &pixmap,QObject *parent ) :
		ConfigModule(i18n("AutoReplace"),i18n("AutoReplace Plugin"),pixmap,parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new AutoReplacePrefsUI( this );

	// creates table columns (avoids new columns every time)
	preferencesDialog->m_list->addColumn( i18n("text") );
	preferencesDialog->m_list->addColumn( i18n("replacement") );

	// connect SIGNALS/SLOTS
	connect( preferencesDialog->m_add, SIGNAL(pressed()),
		SLOT( slotAddCouple()) );
	connect( preferencesDialog->m_remove, SIGNAL(pressed()),
		SLOT(slotRemoveCouple()) );
	/* connect( preferencesDialog->m_list, SIGNAL(selectionChanged()),
		SLOT(slotEnableRemove()) );
	connect( preferencesDialog->m_value, SIGNAL(textChanged ( const QString & )),
		SLOT( slotEnableAdd()) ); */

	reopen();
}

AutoReplacePreferences::~AutoReplacePreferences()
{
}

// reload configuration reading it from kopeterc
void AutoReplacePreferences::reopen()
{
	KGlobal::config()->setGroup("AutoReplace Plugin");
	// read the list of words to replace
	wordsList = KGlobal::config()->readListEntry("WordsToReplace");
	if( wordsList.isEmpty() )
	{
		// basic list, key/value
		// a list based on i18n should be provided, i.e. for italian
		// "qsa,qualcosa,qno,qualcuno" remember UTF-8 accents
		wordsList = QStringList::split( ",", i18n( "list_of_words_to_replace", 
			"ur,your,r,are,u,you,theres,there is,arent,are not,dont,do not" ) );
	}

	// Removes and deletes all the items in this list view and triggers an update
	preferencesDialog->m_list->clear();

	// show keys/values on gui
	QStringList::Iterator it=wordsList.begin();
	QString k, v;
	while( it!=wordsList.end() ) {
		// notice: insertItem is called automatically by the constructor
		k = *it;
		v = *(++it);
		new QListViewItem( preferencesDialog->m_list, k, v );
		++it;
	}

	// checkboxes
	autoreplaceIncoming = KGlobal::config()->readBoolEntry( "AutoReplaceIncoming" , false );
	autoreplaceOutgoing = KGlobal::config()->readBoolEntry( "AutoReplaceOutgoing" , true );
	addDot = KGlobal::config()->readBoolEntry( "DotEndSentence" , false );
	upper = KGlobal::config()->readBoolEntry( "CapitalizeBeginningSentence" , false );
	preferencesDialog->m_cb_incoming->setChecked( autoreplaceIncoming );
	preferencesDialog->m_cb_outgoing->setChecked( autoreplaceOutgoing );
	preferencesDialog->m_cb_dot->setChecked( addDot );
	preferencesDialog->m_cb_upper->setChecked( upper );

	// MAP NEEDS TO BE FIRST CREATED HERE
	QStringList::Iterator itl = wordsList.begin();
	while( itl != wordsList.end() ) {
		k = *itl;
		v = *(++itl);
		map.insert( k, v );
		++itl;
	}
}

// save list to kopeterc and creates map out of it
void AutoReplacePreferences::save()
{
	KConfig * config = KGlobal::config();
	config->setGroup( "AutoReplace Plugin" );

	// make a list reading all values from gui
	QStringList newWords = QStringList();
	QListViewItem * i = preferencesDialog->m_list->firstChild();
	while( i!=0 ) {
		newWords += i->text(0);
		newWords += i->text(1);
		i = i->nextSibling();
	}

	// save the words list
	config->writeEntry("WordsToReplace", newWords);

	// save checkboxes
	autoreplaceIncoming = preferencesDialog->m_cb_incoming->isChecked();
	autoreplaceOutgoing = preferencesDialog->m_cb_outgoing->isChecked();
	addDot = preferencesDialog->m_cb_dot->isChecked();
	upper = preferencesDialog->m_cb_upper->isChecked();
	config->writeEntry("AutoReplaceIncoming", autoreplaceIncoming);
	config->writeEntry("AutoReplaceOutgoing", autoreplaceOutgoing);
	config->writeEntry("DotEndSentence", addDot);
	config->writeEntry("CapitalizeBeginningSentence", upper);

	// save all config to kopeterc
	config->sync();

	// create map out of a list
	QString k, v;
	map.clear();
	QStringList::Iterator itl = newWords.begin();
	while( itl != newWords.end() ) {
		k = *itl;
		v = *(++itl);
		map.insert( k, v );
		++itl;
	}

}

// read m_key m_value, create a QListViewItem
void AutoReplacePreferences::slotAddCouple()
{
	QString k = preferencesDialog->m_key->text();
	QString v = preferencesDialog->m_value->text();
	if( !k.isEmpty() && !k.isNull() && !v.isEmpty() && !v.isNull() ) {
		QListViewItem * lvi =
			new QListViewItem( preferencesDialog->m_list, k, v );
		// clear k and v [only if added]
		preferencesDialog->m_key->clear();
		preferencesDialog->m_value->clear();
		// Triggers a size, geometry and content update
		// during the next iteration of the event loop
		preferencesDialog->m_list->triggerUpdate();
		// select last added
		preferencesDialog->m_list->setSelected(lvi, true);
	}
}

// Returns a pointer to the selected item if the list view is in
// Single selection mode and an item is selected
void AutoReplacePreferences::slotRemoveCouple()
{
	QListViewItem * lvi = preferencesDialog->m_list->selectedItem();
      	if(lvi)
		delete lvi;
}
/*
void AutoReplacePreferences::slotEnableAdd()
{
	//preferencesDialog->m_add
}

void AutoReplacePreferences::slotEnableRemove()
{
	preferencesDialog->m_remove->setEnabled( true );
}
*/
#include "autoreplacepreferences.moc"
