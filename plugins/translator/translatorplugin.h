/*
    translatorplugin.h

    Kopete Translatorfish Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef BABELFISHPLUGIN_H
#define BABELFISHPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qcstring.h>

#include <kio/job.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class QStringList;
class QString;

class KopeteMessage;
class KopeteMetaContact;

class TranslatorPreferences;

/*
BabelFish supported (DON'T REMOVE)

en_zh English to Chinese
en_fr English to French
en_de English to German
en_it English to Italian
en_ja English to Japanese
en_ko English to Korean
en_pt English to Portuguese
en_es English to Spanish
zh_en Chinese to English
fr_en French to English
fr_de French to German
de_en German to English
de_fr German to French
it_en Italian to English
ja_en Japanese to English
ko_en Korean to English
pt_en Portuguese to English
ru_en Russian to English
es_en Spanish to English
*/

class TranslatorPlugin : public KopetePlugin
{
	Q_OBJECT

public:
    static  TranslatorPlugin  *plugin();

	TranslatorPlugin( QObject *parent, const char *name, const QStringList &args );
	~TranslatorPlugin();

	void init();
	bool unload();

	bool serialize( KopeteMetaContact *metaContact, QStringList &strList) const;
	void deserialize( KopeteMetaContact *metaContact, const QStringList& data );

	const QString& languageName( const QString &key )
	{ return m_langs[key]; };

	const QMap<QString,QString>& languagesMap()
	{ return m_langs; };

	const QMap<QString,QString>& servicesMap()
	{ return m_services; };

	const QStringList& supported( const QString &servicekey)
	{ return m_supported[servicekey]; };
    
public slots:

	void slotIncomingMessage( KopeteMessage& msg );
	void slotOutgoingMessage( KopeteMessage& msg );
	void slotDataReceived ( KIO::Job *, const QByteArray &data);
	void slotJobDone ( KIO::Job *);

protected:

	void translateMessage( KopeteMessage& msg, const QString &, const QString & );

private:
	/* Known Languages key -> desc ie: en -> English */
	QMap< QString, QString> m_langs;
    /* Known Services key -> desc ie: en -> English */
	QMap< QString, QString> m_services;
	/* Supported translations per service, src_dst format ( ie: en_es )*/
	QMap< QString, QStringList > m_supported;

	/* Each person language */
	QMap<const KopeteMetaContact*, QString> m_langMap;

   	/* My language for each metacontact */
    QMap<const KopeteMetaContact*, QString> m_myLangMap;

	/* My default language */
	QString m_myLang;

	/* Translator plugin Preferences */
	TranslatorPreferences *m_prefs;

	QMap< KIO::Job *, QCString> m_data;
	QMap< KIO::Job *, bool> m_completed;

	static TranslatorPlugin* pluginStatic_;

};

#endif


