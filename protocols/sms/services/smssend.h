#ifndef SMSSEND_H
#define SMSSEND_H

#include "smsservice.h"
#include "smssendarg.h"
#include <qobject.h>
#include <qmap.h>

class SMSSendPrefsUI;
class QListViewItem;

class SMSSend : public SMSService
{
	Q_OBJECT
public:
	SMSSend(SMSContact* contact);
	~SMSSend();

	void send(const KopeteMessage& msg);
	QWidget* configureWidget(QWidget* parent);

	int maxSize();
	QString description();

public slots:
	void savePreferences();

private slots:
	void setOptions(const QString& name);
signals:
	void messageSent(const KopeteMessage&);

private:
	SMSSendPrefsUI* prefWidget;
	QPtrList<SMSSendArg> args;
	QStringList providers();
} ;

#endif //SMSSEND_H
