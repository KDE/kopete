#ifndef SMSSERVICE_H
#define SMSSERVICE_H

#include <qstring.h>
#include <qwidget.h>

class SMSService
{
public:
	virtual bool send(QString nr, QString message);
	virtual QWidget* configureWidget();
} ;

#endif //SMSSERVICE_H
