#ifndef SMSSENDPROVIDER_H
#define SMSSENDPROVIDER_H

#include "kopetemessage.h"
#include "smssendarg.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>

class KProcess;
class SMSContact;

class SMSSendProvider : public QObject
{
	Q_OBJECT
public:
	SMSSendProvider(const QString& providerName, const QString& prefixValue, SMSContact* contact, QObject* parent = 0, const char* name = 0);
	~SMSSendProvider();

	int count();
	const QString& name(int i);
	const QString& value(int i);
	const QString& description(int i);

	void save(QPtrList<SMSSendArg>& args);
	void send(const KopeteMessage& msg);

	int maxSize();
private slots:
	void slotReceivedOutput(KProcess*, char  *buffer, int  buflen);
	void slotSendFinished(KProcess*);
private:
	QStringList names;
	QStringList descriptions;
	QStringList values;

	int messagePos;
	int telPos;
	int m_maxSize;

	QString provider;
	QString prefix;
	QStringList output;

	SMSContact* m_contact;

	KopeteMessage m_msg;

	bool canSend;
signals:
	void messageSent(const KopeteMessage& msg);
} ;

#endif //SMSSENDPROVIDER_H
