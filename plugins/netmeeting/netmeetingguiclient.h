/*
    netmeetingguiclient.h

    Kopete NetMeeting Plugin

    Copyright (c) 2003 by Olivier Goffart <ogoffart @ kde.org>

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

namespace Kopete { class ChatSession; }
class MSNChatSession;
class NetMeetingPlugin;

/**
  * @author Olivier Goffart <ogoffart @ kde.org>
  */

class NetMeetingGUIClient : public QObject , public KXMLGUIClient
{
	Q_OBJECT

public:
	NetMeetingGUIClient( MSNChatSession *parent, const char *name=0L);
	~NetMeetingGUIClient();

private slots:
	void slotStartInvitation();

private:
	MSNChatSession *m_manager;
	NetMeetingPlugin *m_plugin;
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

