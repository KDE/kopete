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
#include <kconfiggroup.h>

//TODO: Use KConfigXT
AutoReplaceConfig::AutoReplaceConfig()
{
	load();
}

// reload configuration reading it from kopeterc
void AutoReplaceConfig::load()
{
	KConfigGroup config(KGlobal::config(), "AutoReplace Plugin");

	QStringList wordsList = config.readEntry( "WordsToReplace", QStringList() );
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
	for ( QStringList::ConstIterator it = wordsList.constBegin(); it != wordsList.constEnd(); ++it )
	{
		k = *it;
		++it;
		if( it == wordsList.constEnd() )
			break;
		v = *it;
		m_map.insert( k, v );
	}

	m_autoreplaceIncoming = config.readEntry( "AutoReplaceIncoming" , false );
	m_autoreplaceOutgoing = config.readEntry( "AutoReplaceOutgoing" , true );
	m_addDot              = config.readEntry( "DotEndSentence" , false );
	m_upper               = config.readEntry( "CapitalizeBeginningSentence" , false );
}

QStringList AutoReplaceConfig::defaultAutoReplaceList()
{
    return i18nc( "list_of_words_to_replace",
			"ur,your,r,are,u,you,theres,there is,arent,are not,dont,do not" ).split( ',', QString::SkipEmptyParts );
}

void AutoReplaceConfig::loadDefaultAutoReplaceList()
{
    const QStringList wordsList = defaultAutoReplaceList();
    m_map.clear();
    QString k, v;
    for ( QStringList::ConstIterator it = wordsList.constBegin(); it != wordsList.constEnd(); ++it )
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

void AutoReplaceConfig::setAutoReplaceIncoming(bool enabled)
{
	m_autoreplaceIncoming = enabled;
}

void AutoReplaceConfig::setAutoReplaceOutgoing(bool enabled)
{
	m_autoreplaceOutgoing = enabled;
}

void AutoReplaceConfig::setDotEndSentence(bool enabled)
{
	m_addDot = enabled;
}

void AutoReplaceConfig::setCapitalizeBeginningSentence(bool enabled)
{
	m_upper = enabled;
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
	KConfigGroup config(KGlobal::config(), "AutoReplace Plugin" );

	QStringList newWords;
	WordsToReplace::ConstIterator it;
	for ( it = m_map.constBegin(); it != m_map.constEnd(); ++it )
	{
		newWords.append( it.key() );
		newWords.append( it.value() );
	}

	config.writeEntry( "WordsToReplace", newWords );

	config.writeEntry( "AutoReplaceIncoming" , m_autoreplaceIncoming );
	config.writeEntry( "AutoReplaceOutgoing" , m_autoreplaceOutgoing );
	config.writeEntry( "DotEndSentence" , m_addDot );
	config.writeEntry( "CapitalizeBeginningSentence" , m_upper );

	config.sync();
}

// vim: set noet ts=4 sts=4 sw=4:

