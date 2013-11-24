/*
    highlightplugin.h  -  description

    Copyright (c) 2003      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef HighlightPLUGIN_H
#define HighlightPLUGIN_H

#include <QVariantList>
#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class QStringList;
class QString;

namespace Kopete { class Message; }
namespace Kopete { class MetaContact; }
namespace Kopete { class ChatSession; }

class HighlightConfig;

/**
  * @author Olivier Goffart
  */

class HighlightPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	static HighlightPlugin  *plugin();

	HighlightPlugin( QObject *parent, const QVariantList &args );
	~HighlightPlugin();

public slots:
	void slotIncomingMessage( Kopete::Message& msg );
	void slotSettingsChanged();


private:
	static HighlightPlugin* pluginStatic_;
	HighlightConfig *m_config;
};

#endif
