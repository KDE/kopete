/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "smseditaccountwidget.h"
#include "smsprefs.h"
#include "serviceloader.h"
#include "smsprotocol.h"
#include "smsaccount.h"

#include <klocale.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <kmessagebox.h>
#include <qlineedit.h>

SMSEditAccountWidget::SMSEditAccountWidget(KopeteAccount *account, QWidget *parent)
	: QWidget(parent), EditAccountWidget(account)
{
	(new QVBoxLayout(this, QBoxLayout::Down))->setAutoAdd(true);

	preferencesDialog = new smsPrefsUI(this);

	service = 0L;
	configWidget = 0L;

	m_account = account;

	QString sName;
	if (m_account)
	{
		preferencesDialog->accountId->setText(m_account->accountId());

		sName = m_account->pluginData(SMSProtocol::protocol(), "ServiceName");
	}
	else
		m_account=new SMSAccount(SMSProtocol::protocol(), QString::null);

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
	if (service != 0L)
		delete service;
}

bool SMSEditAccountWidget::validateData()
{
	return true;
}

KopeteAccount* SMSEditAccountWidget::apply()
{
	m_account->setAccountId(preferencesDialog->accountId->text());

	m_account->setPluginData(SMSProtocol::protocol(), "ServiceName",
		preferencesDialog->serviceName->currentText());

	emit saved();
	return m_account;
}

void SMSEditAccountWidget::setServicePreferences(const QString& serviceName)
{
	if (service != 0L)
		delete service;

	if (configWidget != 0L)
		delete configWidget;
	
	service = ServiceLoader::loadService(serviceName, m_account);

	if ( service == 0L)
		return;
	
	connect (this, SIGNAL(saved()), service, SLOT(savePreferences()));

	configWidget = service->configureWidget(this);
	configWidget->show();
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

