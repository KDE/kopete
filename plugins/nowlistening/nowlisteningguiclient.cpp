#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>

#include "kopetemessagemanager.h"

#include "nowlisteningplugin.h"
#include "nowlisteningguiclient.h"

NowListeningGUIClient::NowListeningGUIClient( KopeteMessageManager *parent )
		: QObject(parent) , KXMLGUIClient(parent)
{
	m_msgManager = parent;
	KAction *actionSendAdvert = new KAction( i18n("Send Media Info"), 0, this,
			SLOT( slotAdvertToCurrentChat() ), actionCollection(), "actionSendAdvert" );
	setXMLFile("nowlisteningchatui.rc");
}

void NowListeningGUIClient::slotAdvertToCurrentChat()
{
	kdDebug(14307) << k_funcinfo << endl;
	QString message = NowListeningPlugin::plugin()->allPlayerAdvert();

	if ( !message.isEmpty() )
	{
		//advertise  to a single chat
		if ( m_msgManager )
			NowListeningPlugin::plugin()->advertiseToChat( m_msgManager, message );
	}
}


