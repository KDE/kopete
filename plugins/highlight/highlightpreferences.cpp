/*
    highlightpreferences.cpp  -  description

    Copyright (c) 2003      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include "highlightpreferences.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <QVBoxLayout>
#include <QPointer>


#include <kcombobox.h>
#include <klineedit.h>
#include <kparts/componentfactory.h>
#include <klocale.h>
#include <k3listview.h>
#include <kgenericfactory.h>
#include <kcolorbutton.h>
#include <kinputdialog.h>
#include <kurlrequester.h>
#include <kregexpeditorinterface.h>
#include <kdebug.h>
#include <knotifyconfigwidget.h>

#include "filter.h"
#include "highlightplugin.h"
#include "highlightconfig.h"
#include "ui_highlightprefsbase.h"

K_PLUGIN_FACTORY(HighlightPreferencesFactory, registerPlugin<HighlightPreferences>();)
K_EXPORT_PLUGIN(HighlightPreferencesFactory( "kcm_kopete_highlight" ))

HighlightPreferences::HighlightPreferences(QWidget *parent, const QVariantList &args)
							: KCModule(HighlightPreferencesFactory::componentData(), parent, args)
{
	donttouch=true;

	preferencesDialog.setupUi(this);

	m_config = new HighlightConfig;

	connect(preferencesDialog.m_list , SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)) , this , SLOT(slotCurrentFilterChanged()));
	connect(preferencesDialog.m_list , SIGNAL(itemDoubleClicked(QListWidgetItem*)) , this , SLOT(slotRenameFilter()));
	connect(preferencesDialog.m_add , SIGNAL(pressed()) , this , SLOT(slotAddFilter()));
	connect(preferencesDialog.m_remove , SIGNAL(pressed()) , this , SLOT(slotRemoveFilter()));
	connect(preferencesDialog.m_rename , SIGNAL(pressed()) , this , SLOT(slotRenameFilter()));
	connect(preferencesDialog.m_editregexp , SIGNAL(pressed()) , this , SLOT(slotEditRegExp()));

	//Maybe here i should use a slot per widget, but i am too lazy
	connect(preferencesDialog.m_case , SIGNAL(stateChanged(int)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_regexp , SIGNAL(stateChanged(int)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_setImportance , SIGNAL(stateChanged(int)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_setBG , SIGNAL(stateChanged(int)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_setFG , SIGNAL(stateChanged(int)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_search , SIGNAL(textChanged(QString)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_notifications , SIGNAL(pressed()) , this , SLOT(slotConfigureNotifications()));
	connect(preferencesDialog.m_raise , SIGNAL(stateChanged(int)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_search , SIGNAL(textChanged(QString)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_importance , SIGNAL(activated(int)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_FG , SIGNAL(changed(QColor)) , this , SLOT(slotSomethingHasChanged()));
	connect(preferencesDialog.m_BG , SIGNAL(changed(QColor)) , this , SLOT(slotSomethingHasChanged()));

	load();
	donttouch=false;
}

HighlightPreferences::~HighlightPreferences()
{
	delete m_config;
}

void HighlightPreferences::load()
{
	m_config->load();
	donttouch=true;
	preferencesDialog.m_list->clear();
	bool first=true;
	foreach( Filter *f, m_config->filters() )
	{
		QListWidgetItem* lvi= new QListWidgetItem(preferencesDialog.m_list);
		lvi->setText(f->displayName);
		lvi->setData(Qt::UserRole,  qVariantFromValue(f) );
		if(first)
			preferencesDialog.m_list->setCurrentItem(lvi);
		first=false;
	}
	donttouch=false;
	slotCurrentFilterChanged();
	emit KCModule::changed(false);
}

void HighlightPreferences::save()
{
	m_config->save();
	emit KCModule::changed(false);
}


void HighlightPreferences::slotCurrentFilterChanged()
{
	donttouch=true;
	Filter *current=selectedItem();
	if(!current)
	{
		preferencesDialog.m_search->setEnabled(false);
		preferencesDialog.m_case->setEnabled(false);
		preferencesDialog.m_regexp->setEnabled(false);
		preferencesDialog.m_importance->setEnabled(false);
		preferencesDialog.m_setImportance->setEnabled(false);
		preferencesDialog.m_BG->setEnabled(false);
		preferencesDialog.m_setBG->setEnabled(false);
		preferencesDialog.m_FG->setEnabled(false);
		preferencesDialog.m_setFG->setEnabled(false);
		preferencesDialog.m_raise->setEnabled(false);
		preferencesDialog.m_editregexp->setEnabled(false);
		preferencesDialog.m_rename->setEnabled(false);
		preferencesDialog.m_remove->setEnabled(false);
		preferencesDialog.m_notifications->setEnabled(false);
		donttouch=false;
		return;
	}
	
	preferencesDialog.m_rename->setEnabled(true);
	preferencesDialog.m_remove->setEnabled(true);
	preferencesDialog.m_notifications->setEnabled(true);	
	preferencesDialog.m_search->setEnabled(true);
	preferencesDialog.m_case->setEnabled(true);
	preferencesDialog.m_regexp->setEnabled(true);
	preferencesDialog.m_setImportance->setEnabled(true);
	preferencesDialog.m_setBG->setEnabled(true);
	preferencesDialog.m_setFG->setEnabled(true);


	preferencesDialog.m_search->setText(current->search);
	preferencesDialog.m_case->setChecked(current->caseSensitive);
	preferencesDialog.m_regexp->setChecked(current->isRegExp);
	preferencesDialog.m_editregexp->setEnabled(current->isRegExp);
	preferencesDialog.m_importance->setCurrentIndex(current->importance);
	preferencesDialog.m_setImportance->setChecked(current->setImportance);
	preferencesDialog.m_importance->setEnabled(current->setImportance);
	preferencesDialog.m_BG->setColor(current->BG);
	preferencesDialog.m_setBG->setChecked(current->setBG);
	preferencesDialog.m_BG->setEnabled(current->setBG);
	preferencesDialog.m_FG->setColor(current->FG);
	preferencesDialog.m_setFG->setChecked(current->setFG);
	preferencesDialog.m_FG->setEnabled(current->setFG);
	preferencesDialog.m_raise->setChecked(current->raiseView);

	donttouch=false;
}

void HighlightPreferences::slotAddFilter()
{
	Filter *filtre=m_config->newFilter();
	QListWidgetItem* lvi= new QListWidgetItem(preferencesDialog.m_list);
	lvi->setText(filtre->displayName );
	lvi->setData( Qt::UserRole, qVariantFromValue(filtre) );
	preferencesDialog.m_list->setCurrentItem(lvi);
}

void HighlightPreferences::slotRemoveFilter()
{
	QListWidgetItem *lvi=preferencesDialog.m_list->currentItem();
	if(!lvi)
		return;
	Filter *current=qvariant_cast<Filter*>(lvi->data(Qt::UserRole));
	if(!current)
		return;

	delete lvi;
	m_config->removeFilter(current);
	emit KCModule::changed(true);
}

void HighlightPreferences::slotRenameFilter()
{
	QListWidgetItem *lvi=preferencesDialog.m_list->currentItem();
	if(!lvi)
		return;
	Filter *current=qvariant_cast<Filter*>(lvi->data(Qt::UserRole));
	if(!current)
		return;

	bool ok;
	const QString newname = KInputDialog::getText(
		i18n( "Rename Filter" ), i18n( "Please enter the new name for the filter:" ), current->displayName, &ok );
	if( !ok )
		return;
	current->displayName=newname;
	lvi->setText(newname);
	emit KCModule::changed(true);
}


void HighlightPreferences::slotSomethingHasChanged()
{
	if(donttouch)
		return;
	Filter *current=selectedItem();
	if(!current)
		return;

	current->search=preferencesDialog.m_search->text();
	current->caseSensitive=preferencesDialog.m_case->isChecked();
	current->isRegExp=preferencesDialog.m_regexp->isChecked();
	preferencesDialog.m_editregexp->setEnabled(current->isRegExp);
	current->importance=preferencesDialog.m_importance->currentIndex();
	current->setImportance=preferencesDialog.m_setImportance->isChecked();
	preferencesDialog.m_importance->setEnabled(current->setImportance);
	current->BG=preferencesDialog.m_BG->color();
	current->setBG=preferencesDialog.m_setBG->isChecked();
	preferencesDialog.m_BG->setEnabled(current->setBG);
	current->FG=preferencesDialog.m_FG->color();
	current->setFG=preferencesDialog.m_setFG->isChecked();
	preferencesDialog.m_FG->setEnabled(current->setFG);
	current->raiseView=preferencesDialog.m_raise->isChecked();

	emit KCModule::changed(true);
}

void HighlightPreferences::slotEditRegExp()
{
	// FIXME: Port editorDialog->qt_cast
 	QDialog *editorDialog = KServiceTypeTrader::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor" );
 	if ( editorDialog )
 	{
 		// kdeutils was installed, so the dialog was found fetch the editor interface
 		KRegExpEditorInterface *editor = qobject_cast<KRegExpEditorInterface*>( editorDialog );
 		Q_ASSERT( editor ); // This should not fail!
 		// now use the editor.
 		editor->setRegExp(preferencesDialog.m_search->text());
 
 		// Finally exec the dialog
 		if(editorDialog->exec() == QDialog::Accepted )
 		{
 			preferencesDialog.m_search->setText(editor->regExp());
 		}
 
 	}
 	else
 	{
 		// Don't offer the dialog.
 	}
}

Filter * HighlightPreferences::selectedItem()
{
	QListWidgetItem *lvi=preferencesDialog.m_list->currentItem();
	if(!lvi)
		return 0L;
	return qvariant_cast<Filter*>(lvi->data(Qt::UserRole));

}

void HighlightPreferences::slotConfigureNotifications()
{
	Filter *current=selectedItem();
	if(!current)
		return;

	
	QPointer <KDialog> dialog=new KDialog(this);
	KNotifyConfigWidget *w=new KNotifyConfigWidget(this);
	dialog->setMainWidget(w);
	
	connect(dialog,SIGNAL(applyClicked()),w,SLOT(save()));
	connect(dialog,SIGNAL(okClicked()),w,SLOT(save()));
	connect(w,SIGNAL(changed(bool)) , dialog , SLOT(enableButtonApply(bool)));

	w->setApplication(QString(), "class" , current->className() );
	dialog->exec();
	delete dialog;
}



#include "highlightpreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:


