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
#include <qintdict.h>
#include <qstring.h>

#include <kio/job.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class QStringList;
class QString;
class KListAction;

class KopeteMessage;
class KopeteMetaContact;

class TranslatorPreferences;

/**
  * @author Duncan Mac-Vicar Prett   <duncan@kde.org>
  *
  * Kopete Translator Plugin
  *
  */

class TranslatorPlugin : public KopetePlugin
{
	Q_OBJECT

public:
	static  TranslatorPlugin  *plugin();

	TranslatorPlugin( QObject *parent, const char *name, const QStringList &args );
	~TranslatorPlugin();

	enum TranslateMode
	{
		DontTranslate	= 0,
		ShowOriginal 	= 1,
		JustTranslate	= 2,
		ShowDialog   	= 3
	};


	/***************************************************************************
	 *   Re-implementation of KopetePlugin class methods                       *
	 ***************************************************************************/

	void init();
	bool unload();

	bool serialize( KopeteMetaContact *metaContact, QStringList &strList) const;
	void deserialize( KopeteMetaContact *metaContact, const QStringList& data );

	virtual KActionCollection *customContextMenuActions(KopeteMetaContact*);

	/***************************************************************************
	 *   Plugin's API (used by preferences)                                    *
	 ***************************************************************************/

	const QString& languageName( const QString &key )
	{ return m_langs[key]; };

	const int languageIndex	( const QString &key )
	{ return m_langKeyIntMap[key]; };

	const QString& languageKey( const int index )
	{ return  m_langIntKeyMap[index]; };

	const QMap<QString,QString>& languagesMap()
	{ return m_langs; };

	const QMap<QString,QString>& servicesMap()
	{ return m_services; };

	const QStringList& supported( const QString &servicekey)
	{ return m_supported[servicekey]; };

	const int serviceIndex	( const QString &key )
	{ return m_servicesKeyIntMap[key]; };

	const QString& serviceKey( const int index )
	{ return  m_servicesIntKeyMap[index]; };

public slots:

	void slotIncomingMessage( KopeteMessage& msg );
	void slotOutgoingMessage( KopeteMessage& msg );
	void slotDataReceived ( KIO::Job *, const QByteArray &data);
	void slotJobDone ( KIO::Job *);
	void slotSetLanguage();

protected:

	void translateMessage( KopeteMessage &, const QString &, const QString & );

	void googleTranslateMessage( KopeteMessage &, const QString &, const QString & );
	void babelTranslateMessage( KopeteMessage &, const QString &, const QString & );


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

	/* int to lang key and viceversa*/
	QMap<int, QString> m_langIntKeyMap;
	QMap<QString, int> m_langKeyIntMap;

	/* int to lang key and viceversa*/
	QMap<int, QString> m_servicesIntKeyMap;
	QMap<QString, int> m_servicesKeyIntMap;

	/* Lang counter */
	int m_lc;
	/* Service counter */
	int m_sc;

	/* Translator plugin Preferences */
	TranslatorPreferences *m_prefs;

	QMap< KIO::Job *, QCString> m_data;
	QMap< KIO::Job *, bool> m_completed;

	KActionCollection* m_actionCollection;
	KListAction* m_actionLanguage;
	KopeteMetaContact *m_currentMetaContact;

	static TranslatorPlugin* pluginStatic_;

private: // Private methods
  /** No descriptions */
  void sendTranslation(KopeteMessage &msg, const QString &translated);
};

#endif

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

