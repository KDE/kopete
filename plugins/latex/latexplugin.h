/*
    latexplugin.h

    Kopete Latex Plugin

    Copyright (c) 2004 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2001-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef LATEXPLUGIN_H
#define LATEXPLUGIN_H

#include <qobject.h>
#include <qstring.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class QStringList;
class QString;

namespace Kopete { class Message; }

/**
  * @author Duncan Mac-Vicar Prett
  */

class LatexPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	static LatexPlugin  *plugin();

	LatexPlugin( QObject *parent, const char *name, const QStringList &args );
	~LatexPlugin();

public slots:
	void slotHandleLatex( Kopete::Message& msg );
	void slotSettingsChanged();


private:
	static LatexPlugin* s_pluginStatic;
	QString m_convScript;
	bool mMagickNotFoundShown;
};

#endif
