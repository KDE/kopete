#ifndef SERVICELOADER_H
#define SERVICELOADER_H

#include "smsservice.h"
#include <qstring.h>
#include <qstringlist.h>

class SMSContact;

class ServiceLoader
{
public:
	static SMSService* loadService(const QString& name, SMSContact* contact);
	static QStringList services();
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

