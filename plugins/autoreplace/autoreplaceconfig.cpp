/*
    autoreplaceconfig.cpp

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

#include "autoreplaceconfig.h"

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

AutoReplaceConfig::AutoReplaceConfig()
{
	load();
}

// reload configuration reading it from kopeterc
void AutoReplaceConfig::load()
{
	KConfig *config = KGlobal::config();
	config->setGroup( "AutoReplace Plugin" );

	QStringList wordsList = config->readListEntry( "WordsToReplace" );
	if( wordsList.isEmpty() )
	{
		// basic list, key/value
		// a list based on i18n should be provided, i.e. for italian
		// "qsa,qualcosa,qno,qualcuno" remember UTF-8 accents
            wordsList = defaultAutoReplaceList();
	}
	
	// we may be reloading after removing an entry from the list
	m_map.clear();
	QString k, v;
	for ( QStringList::Iterator it = wordsList.begin(); it != wordsList.end(); ++it )
	{
		k = *it;
		++it;
		if( it == wordsList.end() )
			break;
		v = *it;
		m_map.insert( k, v );
	}

	m_autoreplaceIncoming = config->readBoolEntry( "AutoReplaceIncoming" , false );
	m_autoreplaceOutgoing = config->readBoolEntry( "AutoReplaceOutgoing" , true );
	m_addDot              = config->readBoolEntry( "DotEndSentence" , false );
	m_upper               = config->readBoolEntry( "CapitalizeBeginningSentence" , false );
}

QStringList AutoReplaceConfig::defaultAutoReplaceList()
{
    return QStringList::split( ",", i18n( "list_of_words_to_replace",
			"ur,your,r,are,u,you,theres,there is,arent,are not,dont,do not" ) );
}

void AutoReplaceConfig::loadDefaultAutoReplaceList()
{
    QStringList wordsList = defaultAutoReplaceList();
    m_map.clear();
    QString k, v;
    for ( QStringList::Iterator it = wordsList.begin(); it != wordsList.end(); ++it )
    {
        k = *it;
        v = *( ++it );
        m_map.insert( k, v );
    }
}


bool AutoReplaceConfig::autoReplaceIncoming() const
{
	return m_autoreplaceIncoming;
}

bool AutoReplaceConfig::autoReplaceOutgoing() const
{
	return m_autoreplaceOutgoing;
}

bool AutoReplaceConfig::dotEndSentence() const
{
	return m_addDot;
}

bool AutoReplaceConfig::capitalizeBeginningSentence() const
{
	return m_upper;
}

void AutoReplaceConfig::setMap( const WordsToReplace &w )
{
	m_map = w;
}

AutoReplaceConfig::WordsToReplace AutoReplaceConfig::map() const
{
	return m_map;
}

void AutoReplaceConfig::save()
{
	KConfig * config = KGlobal::config();
	config->setGroup( "AutoReplace Plugin" );

	QStringList newWords;
	WordsToReplace::Iterator it;
	for ( it = m_map.begin(); it != m_map.end(); ++it )
	{
		newWords.append( it.key() );
		newWords.append( it.data() );
	}

	config->writeEntry( "WordsToReplace", newWords );

	config->sync();
}

// vim: set noet ts=4 sts=4 sw=4:

