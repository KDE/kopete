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

class TranslatorPlugin : public KopetePlugin
{
	Q_OBJECT

public:
	TranslatorPlugin( QObject *parent, const char *name, const QStringList &args );
	~TranslatorPlugin();

	void init();
	bool unload();

	bool serialize( KopeteMetaContact *metaContact,
			QStringList &strList) const;
	void deserialize( KopeteMetaContact *metaContact, const QStringList& data );

public slots:
	void slotIncomingMessage( KopeteMessage& msg );
	void slotOutgoingMessage( KopeteMessage& msg );
	void slotDataReceived ( KIO::Job *, const QByteArray &data);
	void slotJobDone ( KIO::Job *);

protected:
	void translateMessage( KopeteMessage& msg, const QString &, const QString & );
private:
	QMap<const KopeteMetaContact*, QString> m_langMap;

	QMap< KIO::Job *, QCString> m_data;
	QMap< KIO::Job *, bool> m_completed;

	QString myLang;

	TranslatorPreferences *m_prefs;
};

#endif


