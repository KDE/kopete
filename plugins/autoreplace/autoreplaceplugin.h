/***************************************************************************
                          autoreplaceplugin.h  -  description
                             -------------------
    begin                : 20030425
    copyright            : (C) 2003 by Roberto Pariset
    email                : victorheremita@fastwebnet.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AutoReplacePLUGIN_H
#define AutoReplacePLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>
#include <qregexp.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class KopeteMessage;
class KopeteMetaContact;
class KopeteMessageManager;
class AutoReplacePreferences;


class AutoReplacePlugin : public KopetePlugin
{
	Q_OBJECT

public:
	static AutoReplacePlugin  * plugin();

	AutoReplacePlugin( QObject *parent, const char *name, const QStringList &args );
	~AutoReplacePlugin();

public slots:
	void slotAutoReplaceOutgoingMessage( KopeteMessage & msg );
	void slotAutoReplaceIncomingMessage( KopeteMessage & msg );
	void slotAddDot( KopeteMessage & msg );
	void slotCapitolize( KopeteMessage & msg );

private:
	void autoReplaceMessage( KopeteMessage & msg );
	static AutoReplacePlugin * pluginStatic_;
	AutoReplacePreferences * m_prefs;
	QMap<QString, QString> map;
};

#endif
