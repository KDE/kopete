#include "kopeteaway.h"

#include <qstring.h>
#include <kconfig.h>
#include <qmultilineedit.h>

#include <kopete.h>
#include "kopeteawaydialog.h"

KopeteAway *KopeteAway::instance = 0L;

KopeteAway::KopeteAway()
{
	mAwayMessage = "";
	mGlobalAway = false;
}

QString KopeteAway::message()
{
	return getInstance()->mAwayMessage;
}

void KopeteAway::show()
{
	KopeteAwayDialog awaydialog;
	awaydialog.exec();
}

KopeteAway *KopeteAway::getInstance()
{
	if (instance == 0L)
	{
		instance = new KopeteAway;
	}
	return instance;
}

bool KopeteAway::globalAway()
{
	return getInstance()->mGlobalAway;
}

void KopeteAway::setGlobalAway(bool status)
{
	getInstance()->mGlobalAway = status;	
}



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

