#ifndef SMSSEND_H
#define SMSSEND_H

#include <qobject.h>
#include <qmap.h>
#include <qlabel.h>

#include <klineedit.h>

#include "smsservice.h"

class SMSSendProvider;
class SMSSendPrefsUI;
class QListViewItem;
class QGridLayout;

class SMSSend : public SMSService
{
	Q_OBJECT
public:
	SMSSend(KopeteAccount* account);
	~SMSSend();

	virtual void setAccount(KopeteAccount* account);

	void send(const KopeteMessage& msg);
	void setWidgetContainer(QWidget* parent, QGridLayout* container);

	int maxSize();
	const QString& description();

public slots:
	void savePreferences();

private slots:
	void setOptions(const QString& name);
	void loadProviders(const QString& prefix);
//signals:
//	void messageSent(const KopeteMessage&);

private:
	QGridLayout *settingsBoxLayout;
	SMSSendProvider* m_provider;
	SMSSendPrefsUI* prefWidget;
	QPtrList<KLineEdit> args;
	QPtrList<QLabel> labels;
	QString m_description;
} ;

#endif //SMSSEND_H
