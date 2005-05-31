
#include <qmap.h>

#include <kmessagebox.h>
#include <knotifyclient.h>

#include <klocale.h>

#include "knotification.h"
#include "kopeteerrornotifier.h"
#include "kopeteuiglobal.h"

namespace Kopete
{
namespace ErrorNotifier
{

void notify( ErrorType sev, const QString &caption, const QString &message, const QString explaination, const QString debugInfo)
{
	switch (sev)
	{
		case TypeInfo:

		break;
		case TypeWarning:
		/*
		KNotification *notification = KNotification::userEvent( message, QPixmap(), KopeteSystemTray::systemTray(), i18n("Details"), KNotifyClient::PassivePopup, KNotifyClient::Default, QString::null, QString::null, QString::null, KNotification::RaiseWidgetOnActivation|KNotification::CloseOnTimeout|KNotification::CloseWhenWidgetActivated );
		ErrorNotification errorN;
		errorN.caption = caption;
		errorN.message = message;
		errorN.explaination = explaination;
		errorN.picture = QPixmap();
		errorN.debugInfo = debugInfo;
		errorHandler::self()->registerNotification(notification, errorN);
		*/
		break;

		case TypeError:
		//KMessageBox::queuedDetailedError( Kopete::UI::Global::mainWidget(), message, explaination, caption);
		break;

		case TypeFatal:
		//KMessageBox::queuedDetailedError( Kopete::UI::Global::mainWidget(), message, explaination, caption);
		break;
	}
}

ErrorHandler* ErrorHandler::s_self = 0L;

class ErrorHandler::Private
{
	public:
	QMap<KNotification*, ErrorNotification> events;
};

ErrorHandler::ErrorHandler()
{
	d = new Private;
}

ErrorHandler::~ErrorHandler()
{
	delete d;
}

ErrorHandler* ErrorHandler::self()
{
	if (!s_self)
		s_self = new ErrorHandler();
	
	return s_self;
}

void ErrorHandler::registerNotification(KNotification* event, ErrorNotification error)
{
	
}

} // end ns ErrorNotifier
} // end ns Kopete