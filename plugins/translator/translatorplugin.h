/*
    translatorplugin.h

    Kopete Translatorfish Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart <ogoffart@kde.org>

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

#ifndef TRANSLATORPLUGIN_H
#define TRANSLATORPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include <kio/job.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class KSelectAction;

namespace Kopete { class Message; }
namespace Kopete { class MetaContact; }
namespace Kopete { class ChatSession; }

class TranslatorGUIClient;
class TranslatorLanguages;

/**
 * @author Duncan Mac-Vicar Prett   <duncan@kde.org>
 *
 * Kopete Translator Plugin
 */
class TranslatorPlugin : public Kopete::Plugin
{
	Q_OBJECT

friend class TranslatorGUIClient;

public:
	static  TranslatorPlugin  *plugin();

	TranslatorPlugin( QObject *parent, const QVariantList &args );
	~TranslatorPlugin();

	enum TranslateMode
	{
		DontTranslate	= 0,
		ShowOriginal 	= 1,
		JustTranslate	= 2,
		ShowDialog   	= 3
	};

private slots:
	void slotIncomingMessage( Kopete::Message& msg );
	void slotOutgoingMessage( Kopete::Message& msg );
	void slotDataReceived ( KIO::Job *, const QByteArray &data);
	void slotJobDone ( KJob *);
	void slotSetLanguage();
	void slotSelectionChanged(bool);
	void slotNewKMM(Kopete::ChatSession *);
	void loadSettings();

public:
	QString translateMessage( const QString &, const QString &, const QString & );
	void translateMessage( const QString &, const QString &, const QString & , QObject * , const char*);

protected:
	QString googleTranslateMessage( const QString &, const QString &, const QString & );
	QString babelTranslateMessage(const QString &, const QString &, const QString & );

private:

	QMap< KIO::Job *, QByteArray> m_data;
	QMap< KIO::Job *, bool> m_completed;

	KSelectAction* m_actionLanguage;

	static TranslatorPlugin* pluginStatic_;
	TranslatorLanguages *m_languages;

	//Settings
	QString m_myLang;
	QString m_service;
	unsigned int m_outgoingMode;
	unsigned int m_incomingMode;

private:
	void sendTranslation(Kopete::Message &msg, const QString &translated);
};

#endif // TRANSLATORPLUGIN_H

// vim: set noet ts=4 sts=4 sw=4:
