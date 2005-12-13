/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qcombobox.h>
#include <qlayout.h>

#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kconfigbase.h>

#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include "smsclient.h"
#include "smsclientprefs.h"
#include "smsprotocol.h"

SMSClient::SMSClient(Kopete::Account* account)
	: SMSService(account)
{
	prefWidget = 0L;
}

SMSClient::~SMSClient()
{
}

void SMSClient::setWidgetContainer(QWidget* parent, QGridLayout* layout)
{
	kdWarning( 14160 ) << k_funcinfo << "ml: " << layout << ", " << "mp: " << parent << endl;
	m_parent = parent;
	m_layout = layout;
	QWidget *configWidget = configureWidget(parent);
	layout->addMultiCellWidget(configWidget, 0, 1, 0, 1);
	configWidget->show();
}

void SMSClient::send(const Kopete::Message& msg)
{
	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be non-zero!!)" << endl;
	if (!m_account) return;

	m_msg = msg;
	
	KConfigGroup* c = m_account->configGroup();
	QString provider = c->readEntry(QString("%1:%2").arg("SMSClient").arg("ProviderName"), QString::null);

	if (provider.isNull())
	{
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("No provider configured"), i18n("Could Not Send Message"));
		return;
	}

	QString programName = c->readEntry(QString("%1:%2").arg("SMSClient").arg("ProgramName"). QString::null);
	if (programName.isNull())
		programName = "/usr/bin/sms_client";

	KProcess* p = new KProcess;

	QString message = msg.plainBody();
	QString nr = msg.to().first()->contactId();

	*p << programName;
	*p << provider + ":" + nr;
	*p << message;

	QObject::connect(p, SIGNAL(processExited(KProcess *)), this, SLOT(slotSendFinished(KProcess*)));
	QObject::connect(p, SIGNAL(receivedStdout(KProcess*, char*, int)), this, SLOT(slotReceivedOutput(KProcess*, char*, int)));
	QObject::connect(p, SIGNAL(receivedStderr(KProcess*, char*, int)), this, SLOT(slotReceivedOutput(KProcess*, char*, int)));

	p->start(KProcess::Block, KProcess::AllOutput);
}

QWidget* SMSClient::configureWidget(QWidget* parent)
{
	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be ok if zero!!)" << endl;

	if (prefWidget == 0L)
		prefWidget = new SMSClientPrefsUI(parent);

	prefWidget->configDir->setMode(KFile::Directory);
	QString configDir;
	if (m_account)
		configDir = m_account->configGroup()->readEntry(QString("%1:%2").arg("SMSClient").arg("ConfigDir"), QString::null);
	if (configDir.isNull())
		configDir = "/etc/sms";
	prefWidget->configDir->setURL(configDir);

	QString programName;
	if (m_account)
		programName = m_account->configGroup()->readEntry(QString("%1:%2").arg("SMSClient").arg("ProgramName"),
		                                                  QString::null);
	if (programName.isNull())
		programName = "/usr/bin/sms_client";
	prefWidget->program->setURL(programName);

	prefWidget->provider->insertStringList(providers());

	if (m_account)
	{
		QString pName = m_account->configGroup()->readEntry(QString("%1:%2").arg("SMSClient").arg("ProviderName"));
		for (int i=0; i < prefWidget->provider->count(); i++)
		{
			if (prefWidget->provider->text(i) == pName)
			{
				prefWidget->provider->setCurrentItem(i);
				break;
			}
		}
	}

	return prefWidget;
}

void SMSClient::savePreferences()
{
	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be work if zero!!)" << endl;

	if (prefWidget != 0L && m_account != 0L)
	{
		KConfigGroup* c = m_account->configGroup();

		c->writeEntry(QString("%1:%2").arg("SMSClient").arg("ProgramName"), prefWidget->program->url());
		c->writeEntry(QString("%1:%2").arg("SMSClient").arg("ConfigDir"), prefWidget->configDir->url());
		c->writeEntry(QString("%1:%2").arg("SMSClient").arg("ProviderName"), prefWidget->provider->currentText());
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
		emit messageSent(m_msg);
	else
		emit messageNotSent(m_msg, output.join("\n"));
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

