#ifndef SMSSERVICE_H
#define SMSSERVICE_H

#include <qstring.h>
#include <qwidget.h>

class SMSService
{
public:
	virtual bool send(QString nr, QString message) = 0;
	virtual QWidget* configureWidget(QWidget* parent) = 0;
	virtual void savePreferences() = 0;
} ;

#endif //SMSSERVICE_H
