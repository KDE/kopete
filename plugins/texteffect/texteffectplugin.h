/***************************************************************************
                          texteffectplugin.h  -  description
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

#ifndef TextEffectPLUGIN_H
#define TextEffectPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"


class QStringList;
class QString;

class KopeteMessage;
class KopeteMetaContact;
class KopeteMessageManager;
class TextEffectConfig;

/**
  * @author Olivier Goffart
  */

class TextEffectPlugin : public KopetePlugin
{
	Q_OBJECT

public:
	static TextEffectPlugin  *plugin();

	TextEffectPlugin( QObject *parent, const char *name, const QStringList &args );
	~TextEffectPlugin();

public slots:
	void slotOutgoingMessage( KopeteMessage& msg );
	void slotSettingsChanged();

private:
	static TextEffectPlugin* pluginStatic_;
	unsigned int last_color;
	TextEffectConfig *m_config;
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

