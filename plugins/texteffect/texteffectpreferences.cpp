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

#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <klistbox.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kcolordialog.h>

#include "texteffectprefs.h"
#include "texteffectpreferences.h"


TextEffectPreferences::TextEffectPreferences(const QString &pixmap,QObject *parent)
							: ConfigModule(i18n("TextEffect"),i18n("TextEffect Plugin"),pixmap,parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	preferencesDialog = new TextEffectPrefs(this);

	connect(preferencesDialog->mColorsAdd , SIGNAL(pressed()) , this , SLOT(slotAddPressed()));
	connect(preferencesDialog->mColorsRemove , SIGNAL(pressed()) , this , SLOT(slotRemovePressed()));
	connect(preferencesDialog->mColorsUp , SIGNAL(pressed()) , this , SLOT(slotUpPressed()));
	connect(preferencesDialog->mColorsDown , SIGNAL(pressed()) , this , SLOT(slotDownPressed()));

	reopen();

}

TextEffectPreferences::~TextEffectPreferences()
{
}


void TextEffectPreferences::reopen()
{
	KGlobal::config()->setGroup("TextEffect Plugin");

	preferencesDialog->mColorsListBox->clear();
	QStringList colors=KGlobal::config()->readListEntry("Colors");
	if(colors.isEmpty())
	{
		colors=QStringList::split( ",", "#00BBDD,#0088DD,#0000DD,#8800DD,#DD00DD,#DD0088,#DD0000,#DD8800,#DDBB00,#88BB00,#00BB00" );
	}
	preferencesDialog->mColorsListBox->insertStringList(colors);

	preferencesDialog->m_colorRandom->setChecked( KGlobal::config()->readBoolEntry( "Color Random Order" , false ) );
	preferencesDialog->m_fg->setChecked( KGlobal::config()->readBoolEntry( "Color change every lines" , true ) ) ;
	preferencesDialog->m_words->setChecked( KGlobal::config()->readBoolEntry( "Color change every words" , false ) );
	preferencesDialog->m_char->setChecked( KGlobal::config()->readBoolEntry( "Color change every char" , false ) );

	preferencesDialog->m_lamer->setChecked( KGlobal::config()->readBoolEntry( "L4m3r" , false ) );
	preferencesDialog->m_casewaves->setChecked( KGlobal::config()->readBoolEntry( "WaVeS" , false ) );
}

void TextEffectPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup("TextEffect Plugin");

	config->writeEntry("Colors", colors() );
	config->writeEntry("Color Random Order", preferencesDialog->m_colorRandom->isChecked());
	config->writeEntry("Color change every lines", preferencesDialog->m_fg->isChecked());
	config->writeEntry("Color change every words", preferencesDialog->m_words->isChecked());
	config->writeEntry("Color change every char", preferencesDialog->m_char->isChecked());

	config->writeEntry("L4m3r", preferencesDialog->m_lamer->isChecked());
	config->writeEntry("WaVeS", preferencesDialog->m_casewaves->isChecked());

	config->sync();
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

bool TextEffectPreferences::color_lines()
{
	return preferencesDialog->m_fg->isChecked();
}

bool TextEffectPreferences::color_words()
{
	return preferencesDialog->m_words->isChecked();
}

bool TextEffectPreferences::color_char()
{
	return preferencesDialog->m_char->isChecked();
}

bool TextEffectPreferences::color_random()
{
	return preferencesDialog->m_colorRandom->isChecked();
}


bool TextEffectPreferences::lamer()
{
	return preferencesDialog->m_lamer->isChecked();
}

bool TextEffectPreferences::waves()
{
	return preferencesDialog->m_casewaves->isChecked();
}





void TextEffectPreferences::slotAddPressed()
{
	QColor myColor;
	if( KColorDialog::getColor( myColor ) == KColorDialog::Accepted )
	{
		preferencesDialog->mColorsListBox->insertItem(myColor.name());
	}
}
void TextEffectPreferences::slotRemovePressed()
{
	delete preferencesDialog->mColorsListBox->selectedItem();
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
}

#include "texteffectpreferences.moc"
