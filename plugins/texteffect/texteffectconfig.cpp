/*
    texteffectconfig.cpp

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>
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

#include "texteffectconfig.h"

#include <qstring.h>

#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

TextEffectConfig::TextEffectConfig()
{
}

void TextEffectConfig::load()
{
	KConfigGroup config(KGlobal::config(), "TextEffect Plugin");

	mColors = config.readEntry("Colors", QStringList() );
	if(mColors.isEmpty())
	{
            mColors= defaultColorList();
	}
	mColorRandom = config.readEntry("Color Random Order", false);
	mColorLines = config.readEntry("Color change every lines", true);
	mColorWords = config.readEntry("Color change every words", false);
	mColorChar = config.readEntry("Color change every char", false);

	mLamer = config.readEntry("L4m3r", false);
	mWaves = config.readEntry("WaVeS", false);
}

QStringList TextEffectConfig::defaultColorList()
{
    
    return QString(QLatin1String("#00BBDD,#0088DD,#0000DD,#8800DD,#DD00DD,#DD0088,#DD0000,#DD8800,#DDBB00,#88BB00,#00BB00" )).split(',');
}

void TextEffectConfig::save()
{
	KConfigGroup config(KGlobal::config(), "TextEffect Plugin");

	config.writeEntry("Colors", mColors );
	config.writeEntry("Color Random Order", mColorRandom);
	config.writeEntry("Color change every lines", mColorLines);
	config.writeEntry("Color change every words", mColorWords);
	config.writeEntry("Color change every char", mColorChar);

	config.writeEntry("L4m3r", mLamer);
	config.writeEntry("WaVeS", mWaves);

	config.sync();
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
