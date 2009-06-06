/*
    texteffectconfig.h

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

#ifndef TEXTEFFECTCONFIG_H
#define TEXTEFFECTCONFIG_H

#include <QStringList>

class TextEffectConfig
{
public:
	TextEffectConfig();

	void load();
	void save();

	//accessor functions
	QStringList colors() const;
	bool colorLines() const;
	bool colorWords() const;
	bool colorChar() const;
	bool colorRandom() const;
	bool lamer() const;
	bool waves() const;

	void setColors(const QStringList &newColors = QStringList());
	void setColorLines(bool newLines);
	void setColorChar(bool newChar);
	void setColorWords(bool newWords);
	void setColorRandom(bool newRandom);
	void setLamer(bool newLamer);
	void setWaves(bool newWaves);
    QStringList defaultColorList();


private:
	QStringList mColors;
	bool mColorLines;
	bool mColorWords;
	bool mColorChar;
	bool mColorRandom;
	bool mLamer;
	bool mWaves;

};

#endif
