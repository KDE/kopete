#include "kopeteaway.h"

#include <qstring.h>
#include <kglobal.h>
#include <kconfig.h>
#include <qmultilineedit.h>

#include <kopete.h>
#include "kopeteawaydialog.h"

KopeteAway::KopeteAway()
{
	config = KGlobal::config();
	config->setGroup("");
	mAwayMessage = config->readEntry ( "AwayMessage", "I'm currently away" );
}

QString KopeteAway::message()
{
	return mAwayMessage;
}

void KopeteAway::show()
{
	KopeteAwayDialog awaydialog;
	awaydialog.mleMessage->setText(mAwayMessage);
	awaydialog.exec();
}
