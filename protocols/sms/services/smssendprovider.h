#ifndef SMSSENDPROVIDER_H
#define SMSSENDPROVIDER_H

#include <qstring.h>
#include <qstringlist.h>

class KListView;
class QListViewItem;

class SMSSendProvider
{
public:
	SMSSendProvider(QString providerName, QString prefixValue);
	~SMSSendProvider();

	QListViewItem* listItem(KListView* parent, int pos);
	void save(KListView* data);
	void showDescription(QString name);
	int count();
	bool send(QString nr, QString mess);
private:
	QStringList names;
	QStringList descriptions;
	QStringList rules;
	QStringList values;

	QString provider;
	QString prefix;

	bool canSend;
} ;

#endif //SMSSENDPROVIDER_H
