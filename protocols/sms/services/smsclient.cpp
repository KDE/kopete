#include "smsclient.h"
#include "smsclientprefs.h"
#include "smsglobal.h"
#include "smscontact.h"

#include <qdir.h>
#include <qcombobox.h>
#include <klocale.h>
#include <kfile.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kdeversion.h>
#if KDE_VERSION > 305
#include <kprocio.h>
#else
#include <kprocess.h>
#endif

SMSClient::SMSClient(SMSContact* m_contact)
	: SMSService(m_contact)
{
	prefWidget = 0L;
}

SMSClient::~SMSClient()
{
}

void SMSClient::send(const KopeteMessage& msg)
{
	m_msg = msg;

	QString provider = SMSGlobal::readConfig("SMSClient", "ProviderName", m_contact);

	if (provider == QString::null)
	{
		KMessageBox::error(0L, i18n("No provider configured"), i18n("Could not send message"));
		return;
	}

	QString programName = SMSGlobal::readConfig("SMSClient", "ProgramName", m_contact);
	if (programName == QString::null)
		programName = "/usr/bin/sms_client";

#if KDE_VERSION > 305
	KProcIO* p = new KProcIO;
	p->setUseShell(true);
#else
	KShellProcess* p = new KShellProcess;
#endif

	QString message = msg.plainBody();
	QString nr = msg.to().first()->id();
	
	*p << programName;
	*p << QString("%1:%2 \"%3\"").arg(provider).arg(nr).arg(message);

	connect( p, SIGNAL(processExited(KProcess *)), this, SLOT(slotSendFinished(KProcess*)));
	connect( p, SIGNAL(receivedStdout(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));
	connect( p, SIGNAL(receivedStderr(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));

	p->start(KProcess::Block);
}

QWidget* SMSClient::configureWidget(QWidget* parent)
{
	if (prefWidget == 0L)
		prefWidget = new SMSClientPrefsUI(parent);
	
	prefWidget->configDir->setMode(KFile::Directory);
	QString configDir = SMSGlobal::readConfig("SMSClient", "ConfigDir", m_contact);
	if (configDir == QString::null)
		configDir = "/etc/sms";
	prefWidget->configDir->setURL(configDir);
	
	QString programName = SMSGlobal::readConfig("SMSClient", "ProgramName", m_contact);
	if (programName == QString::null)
		programName = "/usr/bin/sms_client";
	prefWidget->program->setURL(programName);
	
	prefWidget->provider->insertStringList(providers());

	QString pName = SMSGlobal::readConfig("SMSClient", "ProviderName", m_contact);
	for (int i=0; i < prefWidget->provider->count(); i++)
	{
		if (prefWidget->provider->text(i) == pName)
		{
			prefWidget->provider->setCurrentItem(i);
			break;
		}
	}
	
	return prefWidget;
}

void SMSClient::savePreferences()
{
	if (prefWidget != 0L)
	{
		SMSGlobal::writeConfig("SMSClient", "ProgramName", m_contact, prefWidget->program->url());
		SMSGlobal::writeConfig("SMSClient", "ConfigDir", m_contact, prefWidget->configDir->url());
		SMSGlobal::writeConfig("SMSClient", "ProviderName", m_contact, prefWidget->provider->currentText());
	}
}

QStringList SMSClient::providers()
{
	QStringList p;

	QDir d;
	d.setPath(QString("%1/services/").arg(prefWidget->configDir->url()));
	p += d.entryList("*", QDir::Files);

	return p;
}

void SMSClient::slotReceivedOutput(KProcess*, char  *buffer, int  buflen)
{
	QStringList lines = QStringList::split("\n", QString::fromLocal8Bit(buffer, buflen));
	for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
		output.append(*it);
}

void SMSClient::slotSendFinished(KProcess* p)
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

int SMSClient::maxSize()
{
	return -1;
}

#include "smsclient.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

