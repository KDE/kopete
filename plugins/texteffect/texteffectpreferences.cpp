/*
    texteffectpreferences.cpp  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include "texteffectpreferences.h"

#include <qstring.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <QVBoxLayout>

#include <klocale.h>
#include <kcolordialog.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include <kdeversion.h>

#include "ui_texteffectprefs.h"
#include "texteffectconfig.h"

K_PLUGIN_FACTORY(TextEffectPreferencesFactory, registerPlugin<TextEffectPreferences>();)
K_EXPORT_PLUGIN(TextEffectPreferencesFactory( "kcm_kopete_texteffect" ))

TextEffectPreferences::TextEffectPreferences(QWidget *parent,
                                             const QVariantList &args)
	: KCModule(TextEffectPreferencesFactory::componentData(), parent, args)
{
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget *w = new QWidget;
	kDebug( 14310 ) << "Creating preferences dialog";
	preferencesDialog = new Ui::TextEffectPrefs;
	preferencesDialog->setupUi( w );
	l->addWidget( w );

	kDebug( 14310 ) << "Creating config object";

	config = new TextEffectConfig;

	kDebug( 14310 ) << "Setting up connections";

	connect(preferencesDialog->mColorsAdd , SIGNAL(pressed()) ,
			this , SLOT(slotAddPressed()));

	connect(preferencesDialog->mColorsRemove , SIGNAL(pressed()) ,
			this , SLOT(slotRemovePressed()));

	connect(preferencesDialog->mColorsUp , SIGNAL(pressed()) ,
			this , SLOT(slotUpPressed()));

	connect(preferencesDialog->mColorsDown , SIGNAL(pressed()) ,
			this , SLOT(slotDownPressed()));

	// Connect up all the check boxes
	connect( preferencesDialog->m_lamer, SIGNAL(clicked()),
			 this, SLOT(slotSettingChanged()) );
	connect( preferencesDialog->m_casewaves, SIGNAL(clicked()),
			 this, SLOT(slotSettingChanged()) );

	connect( preferencesDialog->m_colorRandom, SIGNAL(clicked()),
			 this, SLOT(slotSettingChanged()) );
	connect( preferencesDialog->m_fg, SIGNAL(clicked()),
			 this, SLOT(slotSettingChanged()) );
	connect( preferencesDialog->m_char, SIGNAL(clicked()),
			 this, SLOT(slotSettingChanged()) );
	connect( preferencesDialog->m_words, SIGNAL(clicked()),
			 this, SLOT(slotSettingChanged()) );

	//setMainWidget( preferencesDialog, "Text Effect Plugin" );
}

TextEffectPreferences::~TextEffectPreferences()
{
	delete preferencesDialog;
	delete config;
}


void TextEffectPreferences::load()
{
	kDebug( 14310 ) << "ENTER";

	config->load();

	preferencesDialog->mColorsListBox->insertStringList(config->colors());
	preferencesDialog->m_fg->setChecked(config->colorLines());
	preferencesDialog->m_words->setChecked(config->colorWords());
	preferencesDialog->m_char->setChecked(config->colorChar());
	preferencesDialog->m_lamer->setChecked(config->lamer());
	preferencesDialog->m_casewaves->setChecked(config->waves());


	// Call parent's save method
	KCModule::load();

	// Indicate that we have not changed ^_^
	emit changed( false );

	kDebug( 14310 ) << "EXIT";

}

void TextEffectPreferences::save()
{
	kDebug() << "ENTER";
	// Save the settings
	config->setColors(colors());
	config->setColorRandom(preferencesDialog->m_colorRandom->isChecked());
	config->setColorLines(preferencesDialog->m_fg->isChecked());
	config->setColorWords(preferencesDialog->m_words->isChecked());
	config->setColorChar(preferencesDialog->m_char->isChecked());

	config->setLamer(preferencesDialog->m_lamer->isChecked());
	config->setWaves(preferencesDialog->m_casewaves->isChecked());

	config->save();

	// Notify the plugin that the settings have changed
	//TextEffectPlugin::plugin()->slotSettingsChanged();

	// Call parent's save method
	KCModule::save();

	// Indicate that we have not changed ^_^
	emit changed( false );
	kDebug() << "EXIT";
}

QStringList TextEffectPreferences::colors()
{
	QStringList ret;
	for(unsigned int f=0; f<preferencesDialog->mColorsListBox->count() ; f++)
	{
		ret.append(preferencesDialog->mColorsListBox->text(f));
	}
	return ret;
}

void TextEffectPreferences::slotAddPressed()
{
	QColor myColor;
	if( KColorDialog::getColor( myColor ) == KColorDialog::Accepted )
	{
		preferencesDialog->mColorsListBox->insertItem(myColor.name());
	}

	// Indicate that something has changed
	slotSettingChanged();

}
void TextEffectPreferences::slotRemovePressed()
{
	delete preferencesDialog->mColorsListBox->selectedItem();
	// Indicate that something has changed
	slotSettingChanged();
}


void TextEffectPreferences::slotUpPressed()
{
	int p=preferencesDialog->mColorsListBox->currentItem();
	if(p <= 0 )
		return;
	Q3ListBoxItem *i=preferencesDialog->mColorsListBox->selectedItem();
	if(!i)
		return;
	preferencesDialog->mColorsListBox->setSelected(i,false);
	preferencesDialog->mColorsListBox->takeItem(i);
	preferencesDialog->mColorsListBox->insertItem(i , p-1 );
	preferencesDialog->mColorsListBox->setSelected(i,true);

	// Indicate that something has changed
	slotSettingChanged();

}
void TextEffectPreferences::slotDownPressed()
{
	int p=preferencesDialog->mColorsListBox->currentItem();
	if(p < 0 )
		return;
	Q3ListBoxItem *i=preferencesDialog->mColorsListBox->selectedItem();
	if(!i)
		return;
	preferencesDialog->mColorsListBox->setSelected(i,false);
	preferencesDialog->mColorsListBox->takeItem(i);
	preferencesDialog->mColorsListBox->insertItem(i , p+1 );
	preferencesDialog->mColorsListBox->setSelected(i,true);

	// Indicate that something has changed
	slotSettingChanged();
}



void TextEffectPreferences::slotSettingChanged()
{
	kDebug() << "Called"
			  << endl;
	// Indicate that our settings have changed
    emit changed( true );
}

void TextEffectPreferences::defaults()
{
    preferencesDialog->mColorsListBox->clear();
    preferencesDialog->mColorsListBox->insertStringList(config->defaultColorList());
    preferencesDialog->m_fg->setChecked(false);
    preferencesDialog->m_words->setChecked(false);
    preferencesDialog->m_char->setChecked(false);
    preferencesDialog->m_lamer->setChecked(false);
    preferencesDialog->m_casewaves->setChecked(false);
    preferencesDialog->m_colorRandom->setChecked( false );
    emit changed( true );
}

#include "texteffectpreferences.moc"
