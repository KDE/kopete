#ifndef SERVICELOADER_H
#define SERVICELOADER_H

#include <qstring.h>
#include "smsservice.h"

class ServiceLoader
{
public:
	static SMSService* loadService(QString name);
} ;

#endif //SERVICELOADER_H
