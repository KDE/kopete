/*
    nowlisteningguiclient.h

    Kopete Now Listening To plugin

    Copyright (c) 2005           by Tommi Rantala <tommi.rantala@cs.helsinki.fi>
    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002-2005      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef NOWLISTENINGGUICLIENT_H
#define NOWLISTENINGGUICLIENT_H

#include <QObject>
#include <kxmlguiclient.h>

class KAction;
class NowListeningPlugin;

namespace Kopete {
	class ChatSession;
}

class NowListeningGUIClient : public QObject, public KXMLGUIClient
{
	Q_OBJECT

public:
	NowListeningGUIClient( Kopete::ChatSession* parent, NowListeningPlugin* plugin );
	virtual ~NowListeningGUIClient() {}

protected slots:
	void slotAdvertToCurrentChat();
	void slotPluginUnloaded();

private:
	Kopete::ChatSession* m_msgManager;
	KAction* m_action;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
