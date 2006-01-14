/*
    latexguiclient.h

    Kopete Latex Plugin

    Copyright (c) 2005 by Olivier Goffart <ogoffart @ kde.org>

    Kopete    (c) 2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef TRANSLATORGUICLIENT_H
#define TRANSLATORGUICLIENT_H

#include <qobject.h>
#include <kxmlguiclient.h>

#include <kio/job.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

namespace Kopete { class ChatSession; }

/**
  * @author Olivier Goffart <ogoffart @ kde.org>
  */

class LatexGUIClient : public QObject , public KXMLGUIClient
{
	Q_OBJECT

public:
	LatexGUIClient( Kopete::ChatSession *parent, const char *name=0L);
	~LatexGUIClient();

private slots:
	 void slotPreview();

private:
	Kopete::ChatSession *m_manager;
};

#endif

