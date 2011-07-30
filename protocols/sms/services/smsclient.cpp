/*
  smsclient.cpp  -  SMS Plugin

  Copyright (c) 2003      by Richard Lärkäng        <nouseforaname@home.se>
  Copyright (c) 2003      by Gav Wood               <gav@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "smsclient.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLayout>

#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kconfigbase.h>
#include <kconfiggroup.h>
#include <k3process.h>
#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

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
	kWarning( 14160 ) << "ml: " << layout << ", " << "mp: " << parent;
	m_parent = parent;
	m_layout = layout;
	QWidget *configWidget = configureWidget(parent);
	layout->addWidget(configWidget, 0, 0, 1, 1);
	configWidget->show();
}

void SMSClient::send(const Kopete::Message& msg)
{
	kWarning( 14160 ) << "m_account = " << m_account << " (should be non-zero!!)";
	if (!m_account) return;

	m_msg = msg;
	
	KConfigGroup* c = m_account->configGroup();
	QString provider = c->readEntry(QString("%1:%2").arg("SMSClient").arg("ProviderName"), QString());

	if (provider.isNull())
	{
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("No provider configured"), i18n("Could Not Send Message"));
		return;
	}

	QString programName = c->readEntry(QString("%1:%2").arg("SMSClient").arg("ProgramName"), QString());
	if (programName.isNull())
		programName = "/usr/bin/sms_client";

	K3Process* p = new K3Process;

	QString message = msg.plainBody();
	QString nr = msg.to().first()->contactId();

	*p << programName;
	*p << provider + ':' + nr;
	*p << message;

	QObject::connect(p, SIGNAL(processExited(K3Process*)), this, SLOT(slotSendFinished(K3Process*)));
	QObject::connect(p, SIGNAL(receivedStdout(K3Process*,char*,int)), this, SLOT(slotReceivedOutput(K3Process*,char*,int)));
	QObject::connect(p, SIGNAL(receivedStderr(K3Process*,char*,int)), this, SLOT(slotReceivedOutput(K3Process*,char*,int)));

	p->start(K3Process::Block, K3Process::AllOutput);
}

QWidget* SMSClient::configureWidget(QWidget* parent)
{
	kWarning( 14160 ) << "m_account = " << m_account << " (should be ok if zero!!)";

	if (prefWidget == 0L)
		prefWidget = new SMSClientPrefsUI(parent);

	prefWidget->configDir->setMode(KFile::Directory);
	QString configDir;
	if (m_account)
		configDir = m_account->configGroup()->readEntry(QString("%1:%2").arg("SMSClient").arg("ConfigDir"), QString());
	if (configDir.isNull())
		configDir = "/etc/sms";
	prefWidget->configDir->setUrl(configDir);

	QString programName;
	if (m_account)
		programName = m_account->configGroup()->readEntry(QString("%1:%2").arg("SMSClient").arg("ProgramName"),
		                                                  QString());
	if (programName.isNull())
		programName = "/usr/bin/sms_client";
	prefWidget->program->setUrl(programName);

	prefWidget->provider->addItems(providers());

	if (m_account)
	{
		QString pName = m_account->configGroup()->readEntry(QString("%1:%2").arg("SMSClient").arg("ProviderName"), QString());
		for (int i=0; i < prefWidget->provider->count(); i++)
		{
			if (prefWidget->provider->itemText(i) == pName)
			{
				prefWidget->provider->setCurrentIndex(i);
				break;
			}
		}
	}

	return prefWidget;
}

void SMSClient::savePreferences()
{
	kWarning( 14160 ) << "m_account = " << m_account << " (should be work if zero!!)";

	if (prefWidget != 0L && m_account != 0L)
	{
		KConfigGroup* c = m_account->configGroup();

		c->writeEntry(QString("%1:%2").arg("SMSClient").arg("ProgramName"), prefWidget->program->url().url());
		c->writeEntry(QString("%1:%2").arg("SMSClient").arg("ConfigDir"), prefWidget->configDir->url().url());
		c->writeEntry(QString("%1:%2").arg("SMSClient").arg("ProviderName"), prefWidget->provider->currentText());
	}
}

QStringList SMSClient::providers()
{
	QStringList p;

	QDir d;
	d.setPath(QString("%1/services/").arg(prefWidget->configDir->url().url()));
	p += d.entryList(QStringList(QLatin1String("*")), QDir::Files);

	return p;
}

void SMSClient::slotReceivedOutput(K3Process*, char  *buffer, int  buflen)
{
	QStringList lines = QString::fromLocal8Bit(buffer, buflen).split('\n');
	for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
		output.append(*it);
}

void SMSClient::slotSendFinished(K3Process* p)
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
	m_description = i18n("<qt>SMSClient is a program for sending SMS with the modem. The program can be found on <a href=\"%1\">%1</a></qt>", url);
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

