#ifndef NOWLISTENINGGUICLIENT_H
#define NOWLISTENINGGUICLIENT_H

#include <qobject.h>
#include <kxmlguiclient.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class NowListeningGUIClient : public QObject , public KXMLGUIClient
{
Q_OBJECT
	public:
		NowListeningGUIClient( KopeteMessageManager *parent );

	protected slots:
		void slotAdvertToCurrentChat();

	private:
		KopeteMessageManager* m_msgManager;
};

#endif
