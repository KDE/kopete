/*
    translatorlanguages.cpp

    Kopete Translatorfish Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett       <duncan@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart      <ogoffart @ kde.org>
    Copyright (c) 2003 by Matt Rogers                <matt@matt.rogers.name>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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
#include <qmap.h>
#include <klocale.h>

#include "translatorlanguages.h"

TranslatorLanguages::TranslatorLanguages()
{
	m_lc = 0;
	m_sc = 0;
	m_services.insert("babelfish", "BabelFish");
	m_services.insert("google", "Google");

	m_langs.insert("null", i18n("Unknown"));
	m_langs.insert("en", i18n("English"));
	m_langs.insert("zh", i18n("Chinese"));
	m_langs.insert("fr", i18n("French"));
	m_langs.insert("de", i18n("German"));
	m_langs.insert("it", i18n("Italian"));
	m_langs.insert("ja", i18n("Japanese"));
	m_langs.insert("ko", i18n("Korean"));
	m_langs.insert("pt", i18n("Portuguese"));
	m_langs.insert("ru", i18n("Russian"));
	m_langs.insert("es", i18n("Spanish"));

	/* English to .. */
	m_supported["babelfish"].append("en_zh");
	m_supported["babelfish"].append("en_fr");
	m_supported["babelfish"].append("en_de");
	m_supported["babelfish"].append("en_it");
	m_supported["babelfish"].append("en_ja");
	m_supported["babelfish"].append("en_ko");
	m_supported["babelfish"].append("en_pt");
	m_supported["babelfish"].append("en_es");
	/* Chinese to .. */
	m_supported["babelfish"].append("zh_en");
	/* French to ... */
	m_supported["babelfish"].append("fr_en");
	m_supported["babelfish"].append("fr_de");
	/* German to ... */
	m_supported["babelfish"].append("de_en");
	m_supported["babelfish"].append("de_fr");

	m_supported["babelfish"].append("it_en");
	m_supported["babelfish"].append("ja_en");
	m_supported["babelfish"].append("ko_en");
	m_supported["babelfish"].append("pt_en");
	m_supported["babelfish"].append("ru_en");
	m_supported["babelfish"].append("es_en");

	/* Google Service */
	m_supported["google"].append("en_de");
	m_supported["google"].append("en_es");
	m_supported["google"].append("en_fr");
	m_supported["google"].append("en_it");
	m_supported["google"].append("en_pt");
	m_supported["google"].append("de_en");
	m_supported["google"].append("de_fr");
	m_supported["google"].append("es_en");
	m_supported["google"].append("fr_en");
	m_supported["google"].append("fr_de");
	m_supported["google"].append("it_en");
	m_supported["google"].append("pt_en");

	QMap<QString,QString>::ConstIterator i;

	for ( i = m_langs.begin(); i != m_langs.end() ; ++i )
	{
		m_langIntKeyMap[m_lc] = i.key();
		m_langKeyIntMap[i.key()] = m_lc;
		m_lc++;
	}

	for ( i = m_services.begin(); i != m_services.end() ; ++i )
	{
		m_servicesIntKeyMap[m_sc] = i.key();
		m_servicesKeyIntMap[i.key()] = m_sc;
		m_sc++;
	}
}
