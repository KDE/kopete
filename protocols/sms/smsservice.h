#ifndef SMSSERVICE_H
#define SMSSERVICE_H

#include <qstring.h>
#include <qwidget.h>
#include <qobject.h>

class SMSService : public QObject
{
public:
	SMSService();
	virtual ~SMSService();

	virtual bool send(QString nr, QString message) = 0;
	virtual QWidget* configureWidget(QWidget* parent) = 0;
	virtual void savePreferences() = 0;
} ;

#endif //SMSSERVICE_H
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

