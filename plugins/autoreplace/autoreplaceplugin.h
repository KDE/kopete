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

namespace Kopete {
	class Message;
	class MetaContact;
	class ChatSession;
	class SimpleMessageHandlerFactory;
}

class AutoReplaceConfig;

class AutoReplacePlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	static AutoReplacePlugin *plugin();

	AutoReplacePlugin( QObject *parent, const char *name, const QStringList &args );
	~AutoReplacePlugin();

private slots:
	void slotAboutToSend( Kopete::Message &msg );

	void slotSettingsChanged();

private:
	static AutoReplacePlugin * pluginStatic_;
	Kopete::SimpleMessageHandlerFactory *m_inboundHandler;

	AutoReplaceConfig *m_prefs;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

