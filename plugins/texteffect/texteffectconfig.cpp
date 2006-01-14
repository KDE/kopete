/*
    texteffectconfig.cpp

    Copyright (c) 2003      by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2003      by Matt Rogers           <matt@matt.rogers.name>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qstring.h>

#include <kglobal.h>
#include <kconfig.h>

#include "texteffectconfig.h"

TextEffectConfig::TextEffectConfig()
{
	load();
}

void TextEffectConfig::load()
{
	KConfig *config = KGlobal::config();
	config->setGroup("TextEffect Plugin");

	mColors = config->readListEntry("Colors");
	if(mColors.isEmpty())
	{
            mColors= defaultColorList();
	}
	mColorRandom = config->readBoolEntry("Color Random Order", false);
	mColorLines = config->readBoolEntry("Color change every lines", true);
	mColorWords = config->readBoolEntry("Color change every words", false);
	mColorChar = config->readBoolEntry("Color change every char", false);

	mLamer = config->readBoolEntry("L4m3r", false);
	mWaves = config->readBoolEntry("WaVeS", false);
}

QStringList TextEffectConfig::defaultColorList()
{
    return QStringList::split( ",", "#00BBDD,#0088DD,#0000DD,#8800DD,#DD00DD,#DD0088,#DD0000,#DD8800,#DDBB00,#88BB00,#00BB00" );
}

void TextEffectConfig::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup("TextEffect Plugin");

	config->writeEntry("Colors", mColors );
	config->writeEntry("Color Random Order", mColorRandom);
	config->writeEntry("Color change every lines", mColorLines);
	config->writeEntry("Color change every words", mColorWords);
	config->writeEntry("Color change every char", mColorChar);

	config->writeEntry("L4m3r", mLamer);
	config->writeEntry("WaVeS", mWaves);

	config->sync();
}

QStringList TextEffectConfig::colors() const
{
	return mColors;
}

bool TextEffectConfig::colorRandom() const
{
	return mColorRandom;
}

bool TextEffectConfig::colorWords() const
{
	return mColorWords;
}

bool TextEffectConfig::colorLines() const
{
	return mColorLines;
}

bool TextEffectConfig::colorChar() const
{
	return mColorChar;
}

bool TextEffectConfig::lamer() const
{
	return mLamer;
}

bool TextEffectConfig::waves() const
{
	return mWaves;
}

void TextEffectConfig::setColors(const QStringList &newColors)
{
	mColors = newColors;
}

void TextEffectConfig::setColorWords(bool newWords)
{
	mColorWords = newWords;
}

void TextEffectConfig::setColorLines(bool newLines)
{
	mColorLines = newLines;
}

void TextEffectConfig::setColorRandom(bool newRandom)
{
	mColorRandom = newRandom;
}

void TextEffectConfig::setColorChar(bool newChar)
{
	mColorChar = newChar;
}

void TextEffectConfig::setLamer(bool newLamers)
{
	mLamer = newLamers;
}

void TextEffectConfig::setWaves(bool newWaves)
{
	mWaves = newWaves;
}
