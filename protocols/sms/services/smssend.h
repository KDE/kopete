#ifndef SMSSEND_H
#define SMSSEND_H

#include "smsservice.h"
#include <qobject.h>
#include <qmap.h>

class SMSSendPrefsUI;
class QListViewItem;

class SMSSend : public SMSService
{
	Q_OBJECT
public:
	SMSSend(QString userName);
	~SMSSend();

	void send(const KopeteMessage& msg);
	QWidget* configureWidget(QWidget* parent);

public slots:
	void savePreferences();

private slots:
	void saveProviderPreferences();
	void setOptions(const QString& name);
	void showDescription();
	void changeOption(QListViewItem* i);

private:
	SMSSendPrefsUI* prefWidget;
	QMap<QString, QString> descriptions;
	QStringList providers();
} ;

#endif //SMSSEND_H
