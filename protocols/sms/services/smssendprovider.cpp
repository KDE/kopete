#include "smssendprovider.h"
#include "smsglobal.h"

#include <kdeversion.h>
#if KDE_VERSION > 305
	#include <kprocio.h>
#endif
#include <qregexp.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <klocale.h>

SMSSendProvider::SMSSendProvider(QString providerName, QString prefixValue, QString userName)
{
	QString n = "  ([^ ]*) ";
	QString valueInfo = ".*"; // Should be changed later to match "(info abut the format)"
	QString valueDesc = "(/\\* )(.*)( \\*/)";

	provider = providerName;
	prefix = prefixValue;
	uName = userName;

	QRegExp r = n + valueInfo + valueDesc;

	QString group = QString("SMSSend-%1").arg(provider);

#if KDE_VERSION > 305
	KProcIO* p = new KProcIO;
	p->setUseShell(true);
#else
	KMessageBox::error(0L, "Can't send messages in KDE 3.0 yet", "Could not send message");
	return;
#endif
	*p << QString("%1/bin/smssend").arg(prefix) << provider << "-help";
	p->start(KProcess::Block);

	QString tmp;
	bool nameFound = false;
	bool nrFound = false;
	while ( p->readln(tmp) != -1)
	{
		int pos = r.search(tmp);
		if (pos > -1)
		{
			names.append(r.cap(1));
			
			if (r.cap(1) == "Message")
				nameFound = true;
			if (r.cap(1) == "Tel")
				nrFound = true;

			descriptions.append(r.cap(3));
			rules.append("");
			values.append(SMSGlobal::readConfig(group, r.cap(1), uName));
		}
	}

	if ( !nameFound )
	{
		canSend = false;
		KMessageBox::error(0L, i18n("Could not determine which argument which should contain the message"),
			i18n("Could not send message"));
		return;
	}
	if ( !nrFound )
	{
		canSend = false;
		KMessageBox::error(0L, i18n("Could not determine which argument which should contain the number"),
			i18n("Could not send message"));
		return;
	}

	canSend = true;

	delete p;
}

SMSSendProvider::~SMSSendProvider()
{

}

QListViewItem* SMSSendProvider::listItem(KListView* parent, int pos)
{
	return new KListViewItem(parent, names[pos], values[pos]);
}

void SMSSendProvider::save(KListView* data)
{
	QListViewItem* p;
	QString group = QString("SMSSend-%1").arg(provider);

	for (int i=0; i < data->childCount(); i++)
	{
		p = data->itemAtIndex(i);
		if (p->text(1) == "")
			SMSGlobal::deleteConfig(group, p->text(0), uName);
		else
			SMSGlobal::writeConfig(group, p->text(0), uName, p->text(1));
	}
}

void SMSSendProvider::showDescription(QString name)
{
	int pos = names.findIndex(name);
	if (pos > -1)
		KMessageBox::information(0L, descriptions[pos], name);
}

int SMSSendProvider::count()
{
	return names.count();
}

bool SMSSendProvider::send(QString nr, QString message)
{
	if (canSend = false)
		return false;

	int pos = names.findIndex(QString("Message"));
	values[pos] = message;
	pos = names.findIndex(QString("Tel"));
	values[pos] = nr;

	QString args = "\"" + values.join("\" \"") + "\"";

#if KDE_VERSION > 305
	KProcIO* p = new KProcIO;
	p->setUseShell(true);
#else
	KMessageBox::error(0L, "Can't send messages in KDE 3.0 yet", "Could not send message");
	return false;
#endif
	*p << QString("%1/bin/smssend").arg(prefix) << provider << args;
	p->start(KProcess::Block);
	if (p->normalExit())
	{
		if (p->exitStatus() == 0)
		{
			QStringList msg;
			QString tmp;
			while ( p->readln(tmp) != -1)
				 msg.append(tmp);
		
			KMessageBox::informationList(0L, i18n("Message sent"), msg, i18n("Message sent"));
			return true;
		}
		else
		{
			QString eMsg, tmp;
			while ( p->readln(tmp) != -1)
				eMsg += tmp + "\n";

			KMessageBox::detailedError(0L, i18n("Something went wrong when sending message"), eMsg,
				i18n("Could not send message"));
			return false;
		}
	}
	return false;
}




/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

