#include "smssendprovider.h"
#include "smsglobal.h"
#include "smscontact.h"

#include <kdeversion.h>
#if KDE_VERSION > 305
#include <kprocio.h>
#else
#include <kprocess.h>
#endif
#include <qregexp.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

SMSSendProvider::SMSSendProvider(QString providerName, QString prefixValue, SMSContact* contact, QObject* parent, const char *name)
	: QObject( parent, name )
{

	provider = providerName;
	prefix = prefixValue;
	m_contact = contact;

	messagePos = -1;
	telPos = -1;

	optionsLoaded = false;

#if KDE_VERSION > 305
	KProcIO* p = new KProcIO;
	p->setUseShell(true);
#else
	KShellProcess* p = new KShellProcess;
#endif

	*p << QString("%1/bin/smssend").arg(prefix) << provider << "-help";
	connect( p, SIGNAL(processExited(KProcess *)), this, SLOT(slotOptionsFinished(KProcess*)));
	connect( p, SIGNAL(receivedStdout(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));
	connect( p, SIGNAL(receivedStderr(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));
	output.clear();
	p->start(KProcess::Block);
}

SMSSendProvider::~SMSSendProvider()
{

}

QListViewItem* SMSSendProvider::listItem(KListView* parent, int pos)
{
	while (optionsLoaded == false)
		;

	if ( telPos == pos || messagePos == pos)
		return 0L;
	else
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
			SMSGlobal::deleteConfig(group, p->text(0), m_contact);
		else
			SMSGlobal::writeConfig(group, p->text(0), m_contact, p->text(1));
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

void SMSSendProvider::send(const KopeteMessage& msg)
{
	while (optionsLoaded == false)
		;

	m_msg = msg;

	QString message = msg.plainBody();
	QString nr = msg.to().first()->id();

	if (canSend = false)
		return;

	values[messagePos] = message;
	values[telPos] = nr;

	QString args = "\"" + values.join("\" \"") + "\"";

#if KDE_VERSION > 305
	KProcIO* p = new KProcIO;
	p->setUseShell(true);
#else
	KShellProcess* p = new KShellProcess;
#endif

	*p << QString("%1/bin/smssend").arg(prefix) << provider << args;
	output.clear();
	connect( p, SIGNAL(processExited(KProcess *)), this, SLOT(slotSendFinished(KProcess*)));
	connect( p, SIGNAL(receivedStdout(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));
	connect( p, SIGNAL(receivedStderr(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));

	p->start(KProcess::Block);
}

void SMSSendProvider::slotSendFinished(KProcess* p)
{
	if (p->exitStatus() == 0)
	{
		KMessageBox::information(0L, i18n("Message sent"), output.join("\n"), i18n("Message sent"));

		emit messageSent(m_msg);
	}
	else
	{
		KMessageBox::detailedError(0L, i18n("Something went wrong when sending message"), output.join("\n"),
				i18n("Could not send message"));
	}
}

void SMSSendProvider::slotOptionsFinished(KProcess* p)
{
	QString n = "  ([^ ]*) ";
	QString valueInfo = "(.*)"; // Should be changed later to match "(info abut the format)"
	QString valueDesc = "/\\* (.*) \\*/";

	QRegExp r = n + valueInfo + valueDesc;
	QString group = QString("SMSSend-%1").arg(provider);

	for (unsigned i=0; i < output.count(); i++)
	{
		QString tmp = output[i];

		int pos = r.search(tmp);
		if (pos > -1)
		{
			QString name = r.cap(1);

			names.append(name);
			
			if (name == "Message")
				messagePos = values.count();
			else if (name == "message")
				messagePos = values.count();
			else if (name == "nachricht")
				messagePos = values.count();
			else if (name == "Msg")
				messagePos = values.count();
			else if (name == "Mensagem")
				messagePos = values.count();
			else if (name == "Tel")
				telPos = values.count();
			else if (name == "Number")
				telPos = values.count();
			else if (name == "number")
				telPos = values.count();
			else if (name == "TelNum")
				telPos = values.count();
			else if (name == "Recipient")
				telPos = values.count();
			else if (name == "Tel1")
				telPos = values.count();
			else if (name == "To")
				telPos = values.count();
			else if (name == "nummer")
				telPos = values.count();
			else if (name == "telefone")
				telPos = values.count();
			else if (name == "ToPhone")
				telPos = values.count();

			descriptions.append(r.cap(3));
			rules.append("");
			values.append(SMSGlobal::readConfig(group, r.cap(1), m_contact));
			QRegExp max("\\(Max size ([^)]*)\\)");

			max.search(r.cap(2));
			m_maxSize = QString(max.cap(1)).toInt();

		}
	}

	optionsLoaded = true;

	if ( messagePos == -1 )
	{
		canSend = false;
		KMessageBox::error(0L, i18n("Could not determine which argument which should contain the message"),
			i18n("Could not send message"));
		return;
	}
	if ( telPos == -1 )
	{
		canSend = false;

		KMessageBox::error(0L, i18n("Could not determine which argument which should contain the number"),
			i18n("Could not send message"));
		return;
	}

	canSend = true;

	delete p;
}

void SMSSendProvider::slotReceivedOutput(KProcess*, char  *buffer, int  buflen)
{
	QStringList lines = QStringList::split("\n", QString::fromLocal8Bit(buffer, buflen));
	for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
		output.append(*it);
}

int SMSSendProvider::maxSize()
{
	return m_maxSize;
}

#include "smssendprovider.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

