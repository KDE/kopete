/***************************************************************************
                          cryptographyplugin.h  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CryptographyPLUGIN_H
#define CryptographyPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class QStringList;
class QString;
class QTimer;
class KListAction;

class KopeteMessage;
class KopeteMetaContact;
class KopeteMessageManager;

class CryptographyConfig;

/**
  * @author Olivier Goffart
  */

class CryptographyPlugin : public KopetePlugin
{
	Q_OBJECT

public:
	static CryptographyPlugin  *plugin();
	static QCString cachedPass();
	static void setCachedPass(const QCString &pass);
	static bool passphraseHandling();

	CryptographyPlugin( QObject *parent, const char *name, const QStringList &args );
	~CryptographyPlugin();

public slots:

	void slotIncomingMessage( KopeteMessage& msg );
	void slotOutgoingMessage( KopeteMessage& msg );

private slots:

	void slotSelectContactKey();
	void slotForgetCachedPass();

private:
	static CryptographyPlugin* pluginStatic_;
	QCString m_cachedPass;
	QTimer *m_cachedPass_timer;
	CryptographyConfig *m_config;

	//cache messages for showing
	QMap<QString, QString> m_cachedMessages;
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

