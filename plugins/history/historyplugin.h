/*
    historyplugin.h

    Copyright (c) 2003 by Olivier Goffart             <ogoffart@tiscalinet.be>
              (c) 2003 by Stefan Gehn                 <metz AT gehn.net>
    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef HISTORYPLUGIN_H
#define HISTORYPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include "kopeteplugin.h"

#include "kopetemessage.h"

namespace Kopete { class Message; }
class KopeteView;
class KActionCollection;
namespace Kopete { class MetaContact; }
namespace Kopete { class MessageManager; }
class HistoryPreferences;
class HistoryGUIClient;

/**
  * @author Olivier Goffart
  */
class HistoryPlugin : public Kopete::Plugin
{
	Q_OBJECT
	public:
		HistoryPlugin( QObject *parent, const char *name, const QStringList &args );
		~HistoryPlugin();

		/**
		 * convert the Kopete 0.6 / 0.5 history to the new format
		 */
		static void convertOldHistory();
		/**
		 * return true if an old history has been detected, and no new ones
		 */
		static bool detectOldHistory();

	private slots:
		void slotMessageDisplayed(Kopete::Message &msg);
		void slotViewCreated( KopeteView* );
		void slotViewHistory();
		void slotKMMClosed( Kopete::MessageManager* );
		void slotSettingsChanged();

	private:
		QMap<Kopete::MessageManager*,HistoryGUIClient*> m_loggers;
		Kopete::Message m_lastmessage;
};

#endif


