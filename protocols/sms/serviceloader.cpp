#include "serviceloader.h"
#include "spray.h"
#include <kmessagebox.h>

SMSService* ServiceLoader::loadService(QString name)
{
	SMSService* s;
	if (name == "Spray")
		s = new Spray;
	else
	{
		KMessageBox::sorry(0L, QString("No service configured"), 
			"Error loading service");
		s = 0L;
	}
	
	return s;
}
