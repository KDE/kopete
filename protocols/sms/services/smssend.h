#ifndef SMSSEND_H
#define SMSSEND_H

#include "smsservice.h"
#include "smssendarg.h"
#include <qobject.h>
#include <qmap.h>

class SMSSendProvider;
class SMSSendPrefsUI;
class QListViewItem;

class SMSSend : public SMSService
{
	Q_OBJECT
public:
	SMSSend(KopeteAccount* account);
	~SMSSend();

	virtual void setAccount(KopeteAccount* account);

	void send(const KopeteMessage& msg);
	QWidget* configureWidget(QWidget* parent);

	int maxSize();
	const QString& description();

public slots:
	void savePreferences();

private slots:
	void setOptions(const QString& name);
	void loadProviders(const QString& prefix);
signals:
	void messageSent(const KopeteMessage&);

private:
	SMSSendProvider* m_provider;
	SMSSendPrefsUI* prefWidget;
	QPtrList<SMSSendArg> args;
	QString m_description;
} ;

#endif //SMSSEND_H
