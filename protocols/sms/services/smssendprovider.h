#ifndef SMSSENDPROVIDER_H
#define SMSSENDPROVIDER_H

#include "kopetemessage.h"

#include <qstring.h>
#include <qstringlist.h>

class KListView;
class QListViewItem;
class KProcess;
class SMSContact;

class SMSSendProvider : public QObject
{
	Q_OBJECT
public:
	SMSSendProvider(QString providerName, QString prefixValue, SMSContact* contact, QObject* parent = 0, const char* name = 0);
	~SMSSendProvider();

	QListViewItem* listItem(KListView* parent, int pos);
	void save(KListView* data);
	void showDescription(QString name);
	int count();
	void send(const KopeteMessage& msg);

	int maxSize();
private slots:
	void slotReceivedOutput(KProcess*, char  *buffer, int  buflen);
	void slotSendFinished(KProcess*);
	void slotOptionsFinished(KProcess*);
private:
	QStringList names;
	QStringList descriptions;
	QStringList rules;
	QStringList values;

	int messagePos;
	int telPos;
	int m_maxSize;

	QString provider;
	QString prefix;
	QStringList output;

	SMSContact* m_contact;

	bool optionsLoaded;

	KopeteMessage m_msg;

	bool canSend;
signals:
	void messageSent(const KopeteMessage& msg);
} ;

#endif //SMSSENDPROVIDER_H
