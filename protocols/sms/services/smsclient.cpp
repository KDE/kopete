#include "smsclient.h"
#include "smsclientprefs.h"
#include "smscontact.h"
#include "smsprotocol.h"
#include "kopeteaccount.h"

#include <qdir.h>
#include <qcombobox.h>
#include <klocale.h>
#include <kfile.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kdeversion.h>
#include <kprocess.h>

SMSClient::SMSClient(KopeteAccount* account)
	: SMSService(account)
{
	prefWidget = 0L;
}

SMSClient::~SMSClient()
{
}

void SMSClient::send(const KopeteMessage& msg)
{
	m_msg = msg;

	QString provider = m_account->pluginData(SMSProtocol::protocol(), QString("%1:%2").arg("SMSClient").arg("ProviderName"));

	if (provider.isNull())
	{
		KMessageBox::error(0L, i18n("No provider configured"), i18n("Could Not Send Message"));
		return;
	}

	QString programName = m_account->pluginData(SMSProtocol::protocol(), QString("%1:%2").arg("SMSClient").arg("ProgramName"));
	if (programName.isNull())
		programName = "/usr/bin/sms_client";

	KProcess* p = new KProcess;

	QString message = msg.plainBody();
	QString nr = msg.to().first()->contactId();
	
	*p << programName;
	*p << provider + ":" + nr;
    *p << message;

	connect( p, SIGNAL(processExited(KProcess *)), this, SLOT(slotSendFinished(KProcess*)));
	connect( p, SIGNAL(receivedStdout(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));
	connect( p, SIGNAL(receivedStderr(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));

	p->start(KProcess::Block, KProcess::AllOutput);
}

QWidget* SMSClient::configureWidget(QWidget* parent)
{
	if (prefWidget == 0L)
		prefWidget = new SMSClientPrefsUI(parent);
	
	prefWidget->configDir->setMode(KFile::Directory);
	QString configDir = m_account->pluginData(SMSProtocol::protocol(), QString("%1:%2").arg("SMSClient").arg("ConfigDir"));
	if (configDir.isNull())
		configDir = "/etc/sms";
	prefWidget->configDir->setURL(configDir);
	
	QString programName = m_account->pluginData(SMSProtocol::protocol(), QString("%1:%2").arg("SMSClient").arg("ProgramName"));
	if (programName.isNull())
		programName = "/usr/bin/sms_client";
	prefWidget->program->setURL(programName);
	
	prefWidget->provider->insertStringList(providers());

	QString pName = m_account->pluginData(SMSProtocol::protocol(), QString("%1:%2").arg("SMSClient").arg("ProviderName"));
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
		m_account->setPluginData(SMSProtocol::protocol(), QString("%1:%2").arg("SMSClient").arg("ProgramName"), prefWidget->program->url());
		m_account->setPluginData(SMSProtocol::protocol(), QString("%1:%2").arg("SMSClient").arg("ConfigDir"), prefWidget->configDir->url());
		m_account->setPluginData(SMSProtocol::protocol(), QString("%1:%2").arg("SMSClient").arg("ProviderName"), prefWidget->provider->currentText());
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
		KMessageBox::information(0L, i18n("Message sent"), output.join("\n"), i18n("Message Sent"));

		emit messageSent(m_msg);
	}
	else
	{
		KMessageBox::detailedError(0L, i18n("Something went wrong when sending message"), output.join("\n"),
				i18n("Could Not Send Message"));
	}
}

int SMSClient::maxSize()
{
	return 160;
}

const QString& SMSClient::description()
{
	QString url = "http://www.smsclient.org";
	m_description = i18n("<qt>SMSClient is a program for sending SMS with the modem. The program can be found on <a href=\"%1\">%1</a></qt>").arg(url).arg(url);
	return m_description;
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

