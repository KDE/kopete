#include "serviceloader.h"
#include "smssend.h"
#include "smsclient.h"
#include <kmessagebox.h>
#include <klocale.h>

SMSService* ServiceLoader::loadService(QString name, QString uName)
{
	SMSService* s;
	if (name == "SMSSend")
		s = new SMSSend(uName);
	else if (name == "SMSClient")
		s = new SMSClient(uName);
	else
	{
		KMessageBox::sorry(0L, QString(i18n("Could not load service %1").arg(name)), 
			i18n("Error loading service"));
		s = 0L;
	}
	
	return s;
}

QStringList ServiceLoader::services()
{
	QStringList toReturn;
	toReturn.append("SMSSend");
	toReturn.append("SMSClient");
	return toReturn;
}


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

