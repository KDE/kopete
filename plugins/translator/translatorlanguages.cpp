/*
    translatorlanguages.cpp - Kopete Translatorfish Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2003      by Matt Rogers <matt@matt.rogers.name>

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

#include "translatorlanguages.h"

#include <qstring.h>
#include <qmap.h>
#include <klocale.h>

TranslatorLanguages::TranslatorLanguages()
{
	m_sc = 0;
	m_services.insert("babelfish", "BabelFish");
	m_services.insert("google", "Google");

	// Google
	m_langs["google"].insert("null", i18n("Unknown"));
	m_langs["google"].insert("auto", i18n("Detect language"));
	m_langs["google"].insert("af", i18n("Afrikaans"));
	m_langs["google"].insert("sq", i18n("Albanian"));
	m_langs["google"].insert("ar", i18n("Arabic"));
	m_langs["google"].insert("hy", i18n("Armenian"));
	m_langs["google"].insert("az", i18n("Azerbaijani"));
	m_langs["google"].insert("az", i18n("Azerbaijani"));
	m_langs["google"].insert("eu", i18n("Basque"));
	m_langs["google"].insert("be", i18n("Belarusian"));
	m_langs["google"].insert("bg", i18n("Bulgarian"));
	m_langs["google"].insert("ca", i18n("Catalan"));
	m_langs["google"].insert("zh-CN", i18n("Chinese (Simplified)")); // For google only
	m_langs["google"].insert("zh-TW", i18n("Chinese (Traditional)")); // For google only
	m_langs["google"].insert("hr", i18n("Croatian"));
	m_langs["google"].insert("cs", i18n("Czech"));
	m_langs["google"].insert("da", i18n("Danish"));
	m_langs["google"].insert("nl", i18n("Dutch"));
	m_langs["google"].insert("en", i18n("English"));
	m_langs["google"].insert("et", i18n("Estonian"));
	m_langs["google"].insert("tl", i18n("Filipino"));
	m_langs["google"].insert("fi", i18n("Finnish"));
	m_langs["google"].insert("fr", i18n("French"));
	m_langs["google"].insert("gl", i18n("Galician"));
	m_langs["google"].insert("ka", i18n("Georgian"));
	m_langs["google"].insert("de", i18n("German"));
	m_langs["google"].insert("el", i18n("Greek"));
	m_langs["google"].insert("ht", i18n("Haitian Creole"));
	m_langs["google"].insert("iw", i18n("Hebrew"));
	m_langs["google"].insert("hi", i18n("Hindi"));
	m_langs["google"].insert("hu", i18n("Hungarian"));
	m_langs["google"].insert("is", i18n("Icelandic"));
	m_langs["google"].insert("id", i18n("Indonesian"));
	m_langs["google"].insert("ga", i18n("Irish"));
	m_langs["google"].insert("it", i18n("Italian"));
	m_langs["google"].insert("ja", i18n("Japanese"));
	m_langs["google"].insert("ko", i18n("Korean"));
	m_langs["google"].insert("lv", i18n("Latvian"));
	m_langs["google"].insert("lt", i18n("Lithuanian"));
	m_langs["google"].insert("mk", i18n("Macedonian"));
	m_langs["google"].insert("ms", i18n("Malay"));
	m_langs["google"].insert("mt", i18n("Maltese"));
	m_langs["google"].insert("no", i18n("Norwegian"));
	m_langs["google"].insert("fa", i18n("Persian"));
	m_langs["google"].insert("pl", i18n("Polish"));
	m_langs["google"].insert("pt", i18n("Portuguese"));
	m_langs["google"].insert("ro", i18n("Romanian"));
	m_langs["google"].insert("ru", i18n("Russian"));
	m_langs["google"].insert("sr", i18n("Serbian"));
	m_langs["google"].insert("sk", i18n("Slovak"));
	m_langs["google"].insert("sl", i18n("Slovenian"));
	m_langs["google"].insert("es", i18n("Spanish"));
	m_langs["google"].insert("sw", i18n("Swahili"));
	m_langs["google"].insert("sv", i18n("Swedish"));
	m_langs["google"].insert("th", i18n("Thai"));
	m_langs["google"].insert("tr", i18n("Turkish"));
	m_langs["google"].insert("uk", i18n("Ukrainian"));
	m_langs["google"].insert("ur", i18n("Urdu"));
	m_langs["google"].insert("vi", i18n("Vietnamese"));
	m_langs["google"].insert("cy", i18n("Welsh"));
	m_langs["google"].insert("yi", i18n("Yiddish"));

	// BabelFish
	m_langs["babelfish"].insert("null", i18n("Unknown"));
	m_langs["babelfish"].insert("en", i18n("English"));
	m_langs["babelfish"].insert("nl", i18n("Dutch"));
	m_langs["babelfish"].insert("fr", i18n("French"));
	m_langs["babelfish"].insert("de", i18n("German"));
	m_langs["babelfish"].insert("el", i18n("Greek"));
	m_langs["babelfish"].insert("it", i18n("Italian"));
	m_langs["babelfish"].insert("ja", i18n("Japanese"));
	m_langs["babelfish"].insert("ko", i18n("Korean"));
	m_langs["babelfish"].insert("pt", i18n("Portuguese"));
	m_langs["babelfish"].insert("ru", i18n("Russian"));
	m_langs["babelfish"].insert("es", i18n("Spanish"));
	m_langs["babelfish"].insert("zh", i18n("Chinese (Simplified)")); // for babelfish only
	m_langs["babelfish"].insert("zt", i18n("Chinese (Traditional)")); // for babelfish only

	// Google
	foreach (QString lang1, m_langs["google"].keys())
		foreach (QString lang2, m_langs["google"].keys())
			if (lang1 != lang2)
				m_supported["google"].append(lang1+"_"+lang2);

	// BabelFish
	/* Chinese to .. */
	m_supported["babelfish"].append("zh_en");
	m_supported["babelfish"].append("zh_zt");
	m_supported["babelfish"].append("zt_en");
	m_supported["babelfish"].append("zt_zh");
	/* English to .. */
	m_supported["babelfish"].append("en_zh");
	m_supported["babelfish"].append("en_zt");
	m_supported["babelfish"].append("en_nl");
	m_supported["babelfish"].append("en_fr");
	m_supported["babelfish"].append("en_de");
	m_supported["babelfish"].append("en_el");
	m_supported["babelfish"].append("en_it");
	m_supported["babelfish"].append("en_ja");
	m_supported["babelfish"].append("en_ko");
	m_supported["babelfish"].append("en_pt");
	m_supported["babelfish"].append("en_ru");
	m_supported["babelfish"].append("en_es");
	/* Dutch to ...*/
	m_supported["babelfish"].append("nl_en");
	m_supported["babelfish"].append("nl_fr");
	/* French to ... */
	m_supported["babelfish"].append("fr_nl");
	m_supported["babelfish"].append("fr_en");
	m_supported["babelfish"].append("fr_de");
	m_supported["babelfish"].append("fr_el");
	m_supported["babelfish"].append("fr_it");
	m_supported["babelfish"].append("fr_pt");
	m_supported["babelfish"].append("fr_es");
	/* German to ... */
	m_supported["babelfish"].append("de_en");
	m_supported["babelfish"].append("de_fr");
	/* Greek to ... */
	m_supported["babelfish"].append("el_en");
	m_supported["babelfish"].append("el_fr");
	/* Italian to ... */
	m_supported["babelfish"].append("it_en");
	m_supported["babelfish"].append("it_fr");

	m_supported["babelfish"].append("ja_en");
	m_supported["babelfish"].append("ko_en");
	/* Portuguese to ... */
	m_supported["babelfish"].append("pt_en");
	m_supported["babelfish"].append("pt_fr");

	m_supported["babelfish"].append("ru_en");
	/* Spanish to ... */
	m_supported["babelfish"].append("es_en");
	m_supported["babelfish"].append("es_fr");

	QMap<QString,QString>::ConstIterator i;

	for ( i = m_langs["google"].constBegin(); i != m_langs["google"].constEnd() ; ++i )
	{
		m_langIntKeyMap["google"][m_lc["google"]] = i.key();
		m_langKeyIntMap["google"][i.key()] = m_lc["google"];
		m_lc["google"]++;
	}

	for ( i = m_langs["babelfish"].constBegin(); i != m_langs["babelfish"].constEnd() ; ++i )
	{
		m_langIntKeyMap["babelfish"][m_lc["babelfish"]] = i.key();
		m_langKeyIntMap["babelfish"][i.key()] = m_lc["babelfish"];
		m_lc["babelfish"]++;
	}

	for ( i = m_services.constBegin(); i != m_services.constEnd() ; ++i )
	{
		m_servicesIntKeyMap[m_sc] = i.key();
		m_servicesKeyIntMap[i.key()] = m_sc;
		m_sc++;
	}
}
