#include "gnupgmessagehandler.h"
#include "kopetemessageevent.h"

#include <kdebug.h>
#include <QPointer>

class GnupgMessageHandlerFactory::Private
{
public:
	Message::MessageDirection direction;
	int position;
	QPointer<QObject> target;
	const char *slot;
};

GnupgMessageHandlerFactory::GnupgMessageHandlerFactory( Message::MessageDirection direction,
	int position, QObject *target, const char *slot )
 : d( new Private )
{
	d->direction = direction;
	d->position = position;
	d->target = target;
	d->slot = slot;
}

GnupgMessageHandlerFactory::~GnupgMessageHandlerFactory()
{
	delete d;
}

MessageHandler *GnupgMessageHandlerFactory::create( ChatSession *manager, Message::MessageDirection direction )
{
	Q_UNUSED( manager )
	if ( direction != d->direction )
		return 0;
	MessageHandler *handler = new GnupgMessageHandler;
	QObject::connect( handler, SIGNAL(handle(Kopete::MessageEvent*)), d->target, d->slot );
	return handler;
}

int GnupgMessageHandlerFactory::filterPosition( ChatSession *manager, Message::MessageDirection direction )
{
	Q_UNUSED( manager )
	if ( direction != d->direction )
		return StageDoNotCreate;
	return d->position;
}

GnupgMessageHandler::GnupgMessageHandler()
{
}

GnupgMessageHandler::~GnupgMessageHandler()
{
}

void GnupgMessageHandler::handleMessage( MessageEvent *e )
{
	QPointer< MessageEvent > event = e;
	emit handle( e );
	if( event )
	{
		kDebug(14303) << "MessageEvent still there!";
		MessageHandler::handleMessage( event );
	}
	else
		kDebug(14303) << "MessageEvent destroyed!";
}

#include "cryptographymessagehandler.moc"
