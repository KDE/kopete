#ifndef SMSSERVICE_H
#define SMSSERVICE_H

#include <qstring.h>

class SMSService
{
public:
	virtual bool send(QString nr, QString message);
} ;

#endif //SMSSERVICE_H
