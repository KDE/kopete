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

#define QT3_SUPPORT

#include "autoreplacepreferences.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <QTreeWidget>

#include <klocale.h>
#include <QLineEdit>
#include <kglobal.h>
#include <kgenericfactory.h>

#include "ui_autoreplaceprefs.h"
#include "autoreplaceconfig.h"

K_PLUGIN_FACTORY(AutoReplacePreferencesFactory, registerPlugin<AutoReplacePreferences>();)


// TODO: Use KConfigXT
AutoReplacePreferences::AutoReplacePreferences( QWidget *parent, const QVariantList &args )
: KCModule( parent, args )
{
	QStringList headerList = (QStringList() << i18n("Text") << i18n("Replacement"));
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;
	preferencesDialog = new Ui::AutoReplacePrefsUI;
	preferencesDialog->setupUi( w );
	l->addWidget( w );

	// creates table columns (avoids new columns every time)
	preferencesDialog->m_list->setColumnCount(2);
	preferencesDialog->m_list->setHeaderLabels(headerList);
	preferencesDialog->m_list->header()->setResizeMode(QHeaderView::Stretch);
	preferencesDialog->m_list->setSelectionMode(QAbstractItemView::SingleSelection);
	preferencesDialog->m_list->header()->setSortIndicatorShown(true);
	
	// connect SIGNALS/SLOTS
	connect( preferencesDialog->m_add, SIGNAL(pressed()),
		SLOT(slotAddCouple()) );
	connect( preferencesDialog->m_edit, SIGNAL(pressed()),
		SLOT(slotEditCouple()) );
	connect( preferencesDialog->m_remove, SIGNAL(pressed()),
		SLOT(slotRemoveCouple()) );
	connect( preferencesDialog->m_list, SIGNAL(selectionChanged()),
		SLOT(slotSelectionChanged()) );
	connect( preferencesDialog->m_key, SIGNAL(textChanged(QString)),
		SLOT(slotEnableAddEdit(QString)) ); 

	connect( preferencesDialog->AutoReplaceIncoming, SIGNAL(toggled(bool)),
		SLOT(slotWidgetModified()) ); 
	connect( preferencesDialog->AutoReplaceOutgoing, SIGNAL(toggled(bool)),
		SLOT(slotWidgetModified()) ); 
	connect( preferencesDialog->DotEndSentence, SIGNAL(toggled(bool)),
		SLOT(slotWidgetModified()) ); 
	connect( preferencesDialog->CapitalizeBeginningSentence, SIGNAL(toggled(bool)),
		SLOT(slotWidgetModified()) ); 

	//setMainWidget( preferencesDialog->gb_options, "AutoReplace Plugin" );

	m_config = new AutoReplaceConfig;
}

AutoReplacePreferences::~AutoReplacePreferences()
{
	delete m_config;
	delete preferencesDialog;
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
		QStringList args = (QStringList() << it.key() << it.value());
		// notice: insertItem is called automatically by the constructor
		new QTreeWidgetItem( preferencesDialog->m_list, args );
	}

	preferencesDialog->AutoReplaceIncoming->setChecked(m_config->autoReplaceIncoming());
	preferencesDialog->AutoReplaceOutgoing->setChecked(m_config->autoReplaceOutgoing());
	preferencesDialog->DotEndSentence->setChecked(m_config->dotEndSentence());
	preferencesDialog->CapitalizeBeginningSentence->setChecked(m_config->capitalizeBeginningSentence());
}

// save list to kopeterc and creates map out of it
void AutoReplacePreferences::save()
{
	// make a list reading all values from gui
	AutoReplaceConfig::WordsToReplace newWords;
	for (int i = 0; i < preferencesDialog->m_list->topLevelItemCount(); ++i) {
		QTreeWidgetItem *item = preferencesDialog->m_list->topLevelItem(i);
		newWords[item->text(0)] = item->text(1);
	}
	// save the words list
	m_config->setMap( newWords );

	m_config->setAutoReplaceIncoming(preferencesDialog->AutoReplaceIncoming->isChecked());
	m_config->setAutoReplaceOutgoing(preferencesDialog->AutoReplaceOutgoing->isChecked());
	m_config->setDotEndSentence(preferencesDialog->DotEndSentence->isChecked());
	m_config->setCapitalizeBeginningSentence(preferencesDialog->CapitalizeBeginningSentence->isChecked());

	m_config->save();
}

// read m_key m_value, create a QListViewItem
void AutoReplacePreferences::slotAddCouple()
{
	QString k = preferencesDialog->m_key->text();
	QString v = preferencesDialog->m_value->text();
	QStringList args = (QStringList() << k << v);
	if ( !k.isEmpty() && !k.isNull() && !v.isEmpty() && !v.isNull() )
	{
		QTreeWidgetItem * lvi;
		QTreeWidgetItem * oldLvi = 0;
		// see if we are replacing an existing entry
		QList<QTreeWidgetItem *> oldLvi_List = preferencesDialog->m_list->findItems( k, Qt::MatchExactly, 0 );
		oldLvi = oldLvi_List.first();		//SelectionMode::SingleSelection
                delete oldLvi;
		lvi = new QTreeWidgetItem( preferencesDialog->m_list, args );
		// Triggers a size, geometry and content update
		// during the next iteration of the event loop
		preferencesDialog->m_list->update();
		// select last added
		lvi->setSelected(true);
	}

	slotWidgetModified();
}

// edit the selected item
void AutoReplacePreferences::slotEditCouple()
{
	const QString k = preferencesDialog->m_key->text();
	const QString v = preferencesDialog->m_value->text();
	QTreeWidgetItem * lvi;
	if ( ( lvi = preferencesDialog->m_list->currentItem() ) && !k.isEmpty() && !k.isNull() && !v.isEmpty() && !v.isNull() )
	{
		lvi->setText( 0, k );
		lvi->setText( 1, v );
		preferencesDialog->m_list->update();
		slotWidgetModified();
	}
}

// Returns a pointer to the selected item if the list view is in
// Single selection mode and an item is selected
void AutoReplacePreferences::slotRemoveCouple()
{
	delete preferencesDialog->m_list->currentItem();

	slotWidgetModified();
}

void AutoReplacePreferences::slotEnableAddEdit( const QString & keyText )
{
	preferencesDialog->m_add->setEnabled( !keyText.isEmpty() );
	preferencesDialog->m_edit->setEnabled( !keyText.isEmpty() && preferencesDialog->m_list->currentItem() );
}

void AutoReplacePreferences::slotSelectionChanged()
{
	QTreeWidgetItem *selection = 0;
	if ( ( selection = preferencesDialog->m_list->currentItem() ) )
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
	emit KCModule::changed( true );
}

void AutoReplacePreferences::defaults()
{
    preferencesDialog->m_list->clear();
    m_config->loadDefaultAutoReplaceList();
    AutoReplaceConfig::WordsToReplace::Iterator it;
    AutoReplaceConfig::WordsToReplace map = m_config->map();
    for ( it = map.begin(); it != map.end(); ++it )
    {
        QStringList args = (QStringList() << it.key() << it.value());
        // notice: insertItem is called automatically by the constructor
        new QTreeWidgetItem( preferencesDialog->m_list, args );
    }

	preferencesDialog->AutoReplaceIncoming->setChecked(false);
	preferencesDialog->AutoReplaceOutgoing->setChecked(true);
	preferencesDialog->DotEndSentence->setChecked(false);
	preferencesDialog->CapitalizeBeginningSentence->setChecked(false);

    slotWidgetModified();
}

#include "autoreplacepreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:

