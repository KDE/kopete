/*
    nowlisteningguiclient.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>

	Purpose:
	This class abstracts the interface to Noatun by
	implementing NLMediaPlayer

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopetemessagemanager.h"
#include "kopeteview.h"

#include "nowlisteningplugin.h"
#include "nowlisteningguiclient.h"

NowListeningGUIClient::NowListeningGUIClient( KopeteMessageManager *parent )
		: QObject(parent) , KXMLGUIClient(parent)
{
	m_msgManager = parent;
	new KAction( i18n( "Send Media Info" ), 0, this,
			SLOT( slotAdvertToCurrentChat() ), actionCollection(), "actionSendAdvert" );
	setXMLFile("nowlisteningchatui.rc");
}

void NowListeningGUIClient::slotAdvertToCurrentChat()
{
	kdDebug( 14307 ) << k_funcinfo << endl;
	QString message = NowListeningPlugin::plugin()->allPlayerAdvert();

	// We warn in a mode appropriate to the mode the user invoked the plugin - GUI on menu action, in message if they typed '/media'
	if ( message.isEmpty() )
    {
		QWidget * origin = 0L;
		if ( m_msgManager && m_msgManager->view() )
			origin = m_msgManager->view()->mainWidget();
		KMessageBox::queuedMessageBox( origin, KMessageBox::Sorry,
							i18n( "None of the supported media players (KsCD, JuK, amaroK, Noatun or Kaffeine) are playing anything." ),
							i18n( "Nothing to Send" ) );
    }
    else
	{
		//advertise  to a single chat
		if ( m_msgManager )
			NowListeningPlugin::plugin()->advertiseToChat( m_msgManager, message );
	}
}



#include "nowlisteningguiclient.moc"
