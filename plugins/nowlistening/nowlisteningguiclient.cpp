/*
    nowlisteningguiclient.cpp

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

#include "nowlisteningguiclient.h"
#include "nowlisteningplugin.h"

#include "kopetechatsessionmanager.h"
#include "kopeteview.h"

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

NowListeningGUIClient::NowListeningGUIClient( Kopete::ChatSession *parent, NowListeningPlugin *plugin )
		: QObject(parent) , KXMLGUIClient(parent)
{
	connect(plugin, SIGNAL(readyForUnload()), SLOT(slotPluginUnloaded()));
	m_msgManager = parent;

	m_action = new KAction( i18n( "Send Media Info" ), this );
        actionCollection()->addAction( "actionSendAdvert", m_action );
	connect(m_action, SIGNAL(triggered(bool)), this, SLOT(slotAdvertToCurrentChat()));

	setXMLFile("nowlisteningchatui.rc");
}

void NowListeningGUIClient::slotAdvertToCurrentChat()
{
	kDebug( 14307 ) ;

	// Sanity check - don't crash if the plugin is unloaded and we get called.
	if (!NowListeningPlugin::plugin())
		return;

	QString message = NowListeningPlugin::plugin()->mediaPlayerAdvert();

	// We warn in a mode appropriate to the mode the user invoked the
	// plugin - GUI on menu action, in message if they typed '/media'
	if ( message.isEmpty() )
	{
		QWidget * origin = 0L;
		if ( m_msgManager && m_msgManager->view() )
			origin = m_msgManager->view()->mainWidget();
		KMessageBox::queuedMessageBox( origin, KMessageBox::Sorry,
			i18n( "None of the supported media players (Amarok, KsCD, JuK, Kaffeine, Quod Libet, or Qmmp) are playing anything." ),
			i18n( "Nothing to Send" ) );
	}
	else
	{
		//advertise  to a single chat
		if ( m_msgManager )
			NowListeningPlugin::plugin()->advertiseToChat( m_msgManager, message );
	}
}

// The plugin itself is being unloaded - so remove the GUI entry.
void NowListeningGUIClient::slotPluginUnloaded()
{
	foreach (QWidget *w, m_action->associatedWidgets())
		w->removeAction(m_action);
}

#include "nowlisteningguiclient.moc"

// vim: set noet ts=4 sts=4 sw=4:
