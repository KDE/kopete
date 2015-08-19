#ifndef GNUPGMESSAGEHANDLER_H
#define GNUPGMESSAGEHANDLER_H

#include "kopetemessagehandler.h"
#include "cryptography_export.h"
using namespace Kopete;

class GNUPG_EXPORT GnupgMessageHandlerFactory : public MessageHandlerFactory
{
public:

	GnupgMessageHandlerFactory( Message::MessageDirection direction, int position,
	                             QObject *target, const char *slot );
	~GnupgMessageHandlerFactory();
	
	MessageHandler *create( ChatSession *manager, Message::MessageDirection direction );
	int filterPosition( ChatSession *manager, Message::MessageDirection direction );
	
private:
	class Private;
	Private *d;
};

class GnupgMessageHandler : public MessageHandler
{
	Q_OBJECT
public:
	GnupgMessageHandler();
	~GnupgMessageHandler();
	
	void handleMessage( MessageEvent *event );
	
Q_SIGNALS:
	void handle( Kopete::MessageEvent *event );

private:
};

#endif