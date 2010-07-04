/*
    translatorlanguages.h

    Kopete Translatorfish Translator plugin

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

#ifndef TRANSLATORLANGUAGES_H_
#define TRANSLATORLANGUAGES_H_

#include <qmap.h>
#include <qstringlist.h>

class QString;


class TranslatorLanguages
{
public:
	TranslatorLanguages();

	/***************************************************************************
	 *   Language APIs                                                         *
	 ***************************************************************************/

	const QString& languageName( const QString &servicekey, const QString &key )
	{ return m_langs[servicekey][key]; }

	int languageIndex	( const QString &servicekey, const QString &key ) const
	{ return m_langKeyIntMap[servicekey][key]; }

	const QString& languageKey( const QString &servicekey, const int index )
	{ return  m_langIntKeyMap[servicekey][index]; }

	const QMap<QString,QString>& languagesMap( const QString &servicekey )
	{ return m_langs[servicekey]; }

	const QMap<QString,QString>& servicesMap()
	{ return m_services; }

	const QStringList& supported( const QString &servicekey )
	{ return m_supported[servicekey]; }

	int serviceIndex	( const QString &key ) const
	{ return m_servicesKeyIntMap[key]; }

	const QString& serviceKey( const int index )
	{ return  m_servicesIntKeyMap[index]; }

	int numLanguages( const QString &servicekey )	{ return m_lc[servicekey]; }

	int numServices()	{ return m_sc; }

private:

	/* Known Languages key per service -> desc ie: google,en -> English */
	QMap<QString, QMap< QString, QString> > m_langs;

	/* Known Services key -> desc ie: en -> English */
	QMap< QString, QString> m_services;

	/* Supported translations per service, src_dst format ( ie: en_es )*/
	QMap< QString, QStringList > m_supported;

	/* int to lang key per service and viceversa*/
	QMap<QString, QMap<int, QString> > m_langIntKeyMap;
	QMap<QString, QMap<QString, int> > m_langKeyIntMap;

	/* int to services key and viceversa*/
	QMap<int, QString> m_servicesIntKeyMap;
	QMap<QString, int> m_servicesKeyIntMap;

	/* Lang counter per service */
	QMap <QString,int> m_lc;
	/* Service counter */
	int m_sc;

};

#endif
