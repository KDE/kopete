/*
    autoreplaceconfig.h

    Copyright (c) 2003      by Roberto Pariset       <victorheremita@fastwebnet.it>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

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

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

#ifndef AutoReplaceConfig_H
#define AutoReplaceConfig_H

class AutoReplaceConfig
{
public:
	AutoReplaceConfig();

	void save();
	void load();

	typedef QMap<QString, QString> WordsToReplace;

	WordsToReplace map() const;
	bool autoReplaceIncoming() const;
	bool autoReplaceOutgoing() const;
	bool dotEndSentence() const;
	bool capitalizeBeginningSentence() const;

	void setAutoReplaceIncoming(bool enabled);
	void setAutoReplaceOutgoing(bool enabled);
	void setDotEndSentence(bool enabled);
	void setCapitalizeBeginningSentence(bool enabled);

	void setMap( const WordsToReplace &w );
    QStringList defaultAutoReplaceList();
    void loadDefaultAutoReplaceList();

private:
	WordsToReplace m_map;

	bool m_autoreplaceIncoming;
	bool m_autoreplaceOutgoing;
	bool m_addDot;
	bool m_upper;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

