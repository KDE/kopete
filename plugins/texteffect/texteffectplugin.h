/*
    texteffectplugin.h  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef TextEffectPLUGIN_H
#define TextEffectPLUGIN_H

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
class TextEffectConfig;

/**
  * @author Olivier Goffart
  */

class TextEffectPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	static TextEffectPlugin  *plugin();

	TextEffectPlugin( QObject *parent, const QVariantList &args );
	~TextEffectPlugin();

public slots:
	void slotOutgoingMessage( Kopete::Message& msg );
	void slotSettingsChanged();

private:
	static TextEffectPlugin* pluginStatic_;
	int last_color;
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

