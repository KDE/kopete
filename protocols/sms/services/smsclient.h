#ifndef SMSCLIENT_H
#define SMSCLIENT_H

#include "smsservice.h"
#include "kopetemessage.h"

#include <qobject.h>
#include <qstringlist.h>

class SMSClientPrefsUI;
class SMSContact;
class QListViewItem;
class KProcess;

class SMSClient : public SMSService
{
	Q_OBJECT
public:
	SMSClient(KopeteAccount* account);
	~SMSClient();

	void send(const KopeteMessage& msg);
	QWidget* configureWidget(QWidget* parent);

	int maxSize();
	const QString& description();

public slots:
	void savePreferences();

private slots:
	void slotReceivedOutput(KProcess*, char  *buffer, int  buflen);
	void slotSendFinished(KProcess* p);
signals:
	void messageSent(const KopeteMessage &);

private:
	SMSClientPrefsUI* prefWidget;
	QStringList providers();
	QStringList output;

	KopeteMessage m_msg;

	QString m_description;
} ;

#endif //SMSCLIENT_H
