/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *   copyright: (C) 2003 Grzegorz Jaskiewicz <gj@pointblue.com.pl>       *
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

#include <qvgroupbox.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <krestrictedline.h>

#include "smseditaccountwidget.h"
#include "smsactprefs.h"
#include "serviceloader.h"
#include "smsprotocol.h"
#include "smsaccount.h"

SMSEditAccountWidget::SMSEditAccountWidget(SMSProtocol *protocol, KopeteAccount *account, QWidget *parent, const char */*name*/)
	: QWidget(parent), EditAccountWidget(account)
{
	QVBoxLayout *l = new QVBoxLayout(this, QBoxLayout::Down);
	preferencesDialog = new smsActPrefsUI(this);
	l->addWidget(preferencesDialog);

	service = 0L;
	configWidget = 0L;
	middleFrameLayout = 0L;

	m_account = account;
	m_protocol = protocol;

	QString sName;
	if (m_account)
	{
		preferencesDialog->accountId->setText(m_account->accountId());
		//Disable changing the account ID for now
		//FIXME: Remove this when we can safely change the account ID (Matt)
		preferencesDialog->accountId->setDisabled(true);
		sName = m_account->pluginData(protocol, "ServiceName");
		preferencesDialog->subEnable->setChecked(m_account->pluginData(protocol, "SubEnable") == "true");
		preferencesDialog->subCode->setText(m_account->pluginData(protocol, "SubCode"));
		preferencesDialog->ifMessageTooLong->setCurrentItem((SMSMsgAction)m_account->pluginData(protocol, "MsgAction").toInt());
	}

	preferencesDialog->serviceName->insertStringList(ServiceLoader::services());

	connect (preferencesDialog->serviceName, SIGNAL(activated(const QString &)),
		this, SLOT(setServicePreferences(const QString &)));
	connect (preferencesDialog->descButton, SIGNAL(clicked()),
		this, SLOT(showDescription()));


	for (int i=0; i < preferencesDialog->serviceName->count(); i++)
	{
		if (preferencesDialog->serviceName->text(i) == sName)
		{
			preferencesDialog->serviceName->setCurrentItem(i);
			break;
		}
	}
	setServicePreferences(preferencesDialog->serviceName->currentText());
}

SMSEditAccountWidget::~SMSEditAccountWidget()
{
//	if (service != 0L)
		delete service;
}

bool SMSEditAccountWidget::validateData()
{
	return true;
}

KopeteAccount* SMSEditAccountWidget::apply()
{
	if (m_account)
		m_account->setAccountId(preferencesDialog->accountId->text());
	else
		m_account = new SMSAccount(m_protocol, preferencesDialog->accountId->text());

	if (service)
		service->setAccount(m_account);

	m_account->setPluginData(m_protocol, "ServiceName", preferencesDialog->serviceName->currentText());
	m_account->setPluginData(m_protocol, "SubEnable", preferencesDialog->subEnable->isChecked() ? "true" : "false");
	m_account->setPluginData(m_protocol, "SubCode", preferencesDialog->subCode->text());
	m_account->setPluginData(m_protocol, "MsgAction", QString().setNum((int)(preferencesDialog->ifMessageTooLong->currentItem())));

	emit saved();
	return m_account;
}

void SMSEditAccountWidget::setServicePreferences(const QString& serviceName)
{
//	if (service != 0L)
		delete service;

//	if (configWidget != 0L)
		delete configWidget;

	service = ServiceLoader::loadService(serviceName, m_account);

	if (service == 0L)
		return;

	connect (this, SIGNAL(saved()), service, SLOT(savePreferences()));

//	if (middleFrameLayout != 0L)
		delete middleFrameLayout;
	middleFrameLayout = new QGridLayout(preferencesDialog->middleFrame, 1, 2, 0, 6, "middleFrameLayout");
	service->setWidgetContainer(preferencesDialog->middleFrame, middleFrameLayout);
}

void SMSEditAccountWidget::showDescription()
{
	SMSService* s = ServiceLoader::loadService(preferencesDialog->serviceName->currentText(), 0L);

	QString d = s->description();

	KMessageBox::information(0L, d, i18n("Description"));
}

#include "smseditaccountwidget.moc"

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

