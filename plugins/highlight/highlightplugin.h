/***************************************************************************
                          highlightplugin.h  -  description
                             -------------------
    begin                : mar 14 2003
    copyright            : (C) 2003 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HighlightPLUGIN_H
#define HighlightPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class QStringList;
class QString;
class QTimer;

namespace Kopete { class Message; }
namespace Kopete { class MetaContact; }
namespace Kopete { class ChatSession; }

class HighlightConfig;
class Filter;

/**
  * @author Olivier Goffart
  */

class HighlightPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	static HighlightPlugin  *plugin();

	HighlightPlugin( QObject *parent, const char *name, const QStringList &args );
	~HighlightPlugin();

public slots:
	void slotIncomingMessage( Kopete::Message& msg );
	void slotSettingsChanged();


private:
	static HighlightPlugin* pluginStatic_;
	HighlightConfig *m_config;
};

#endif
