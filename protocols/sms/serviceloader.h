#ifndef SERVICELOADER_H
#define SERVICELOADER_H

#include <qstring.h>
#include "smsservice.h"

class ServiceLoader
{
public:
	static SMSService* loadService(QString name, QString uName);
} ;

#endif //SERVICELOADER_H
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

