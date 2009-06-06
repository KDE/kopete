/*
    translatorguiclient.h

    Kopete Translator Plugin

    Copyright (c) 2003 by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

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
  * @author Olivier Goffart <ogoffart@kde.org>
  */

class TranslatorGUIClient : public QObject , public KXMLGUIClient
{
	Q_OBJECT

public:
	explicit TranslatorGUIClient( Kopete::ChatSession *parent );
	~TranslatorGUIClient();

private slots:
	 void slotTranslateChat();
	 void messageTranslated(const QVariant&);

private:
	Kopete::ChatSession *m_manager;
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

