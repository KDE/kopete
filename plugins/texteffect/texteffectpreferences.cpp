/***************************************************************************
                          texteffectpreferences.cpp  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kcolordialog.h>
#include <kgenericfactory.h>

#include <kdeversion.h>

#include "texteffectprefs.h"
#include "texteffectpreferences.h"
#include "texteffectconfig.h"

typedef KGenericFactory<TextEffectPreferences> TextEffectPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_texteffect, TextEffectPreferencesFactory( "kcm_kopete_texteffect" )  )

TextEffectPreferences::TextEffectPreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
			: KCModule(TextEffectPreferencesFactory::instance(), parent, args)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	preferencesDialog = new TextEffectPrefs(this);
	config = new TextEffectConfig;

	connect(preferencesDialog->mColorsAdd , SIGNAL(pressed()) , this , SLOT(slotAddPressed()));
	connect(preferencesDialog->mColorsRemove , SIGNAL(pressed()) , this , SLOT(slotRemovePressed()));
	connect(preferencesDialog->mColorsUp , SIGNAL(pressed()) , this , SLOT(slotUpPressed()));
	connect(preferencesDialog->mColorsDown , SIGNAL(pressed()) , this , SLOT(slotDownPressed()));

	load();

}

TextEffectPreferences::~TextEffectPreferences()
{
	delete preferencesDialog;
	delete config;
}


void TextEffectPreferences::load()
{
	config->load();

	preferencesDialog->mColorsListBox->insertStringList(config->colors());
	preferencesDialog->m_fg->setChecked(config->colorLines());
	preferencesDialog->m_words->setChecked(config->colorWords());
	preferencesDialog->m_char->setChecked(config->colorChar());
	preferencesDialog->m_lamer->setChecked(config->lamer());
	preferencesDialog->m_casewaves->setChecked(config->waves());

	//TODO-FIXME: port this plugin to KCAutoConfigModule to make it know if the page has been modified or not
	setChanged(true);
}

void TextEffectPreferences::save()
{
	config->setColors(colors());
	config->setColorRandom(preferencesDialog->m_colorRandom->isChecked());
	config->setColorLines(preferencesDialog->m_fg->isChecked());
	config->setColorWords(preferencesDialog->m_words->isChecked());
	config->setColorChar(preferencesDialog->m_char->isChecked());

	config->setLamer(preferencesDialog->m_lamer->isChecked());
	config->setWaves(preferencesDialog->m_casewaves->isChecked());

	config->save();

//	TODO: uncomment this line when the plugin will be ported to autoconfig
//	setChanged( false );
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
	setChanged(true);
}
void TextEffectPreferences::slotRemovePressed()
{
	delete preferencesDialog->mColorsListBox->selectedItem();
	setChanged(true);
}
void TextEffectPreferences::slotUpPressed()
{
	int p=preferencesDialog->mColorsListBox->currentItem();
	if(p <= 0 )
		return;
	QListBoxItem *i=preferencesDialog->mColorsListBox->selectedItem();
	if(!i)
		return;
	preferencesDialog->mColorsListBox->setSelected(i,false);
	preferencesDialog->mColorsListBox->takeItem(i);
	preferencesDialog->mColorsListBox->insertItem(i , p-1 );
	preferencesDialog->mColorsListBox->setSelected(i,true);
	setChanged(true);
}
void TextEffectPreferences::slotDownPressed()
{
	int p=preferencesDialog->mColorsListBox->currentItem();
	if(p < 0 )
		return;
	QListBoxItem *i=preferencesDialog->mColorsListBox->selectedItem();
	if(!i)
		return;
	preferencesDialog->mColorsListBox->setSelected(i,false);
	preferencesDialog->mColorsListBox->takeItem(i);
	preferencesDialog->mColorsListBox->insertItem(i , p+1 );
	preferencesDialog->mColorsListBox->setSelected(i,true);
	setChanged(true);
}

#include "texteffectpreferences.moc"
