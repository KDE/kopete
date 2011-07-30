/*
  smseditaccountwidget.cpp  -  SMS Plugin Edit Account Widget

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

#include "smseditaccountwidget.h"

#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <kconfigbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krestrictedline.h>
#include <kconfiggroup.h>

#include "kopeteuiglobal.h"

#include "smsactprefs.h"
#include "serviceloader.h"
#include "smsprotocol.h"
#include "smsaccount.h"

SMSEditAccountWidget::SMSEditAccountWidget(SMSProtocol *protocol, Kopete::Account *account, QWidget *parent)
	: QWidget(parent), KopeteEditAccountWidget(account)
{
	QVBoxLayout *l = new QVBoxLayout(this);
	preferencesDialog = new smsActPrefsUI(this);
	l->addWidget(preferencesDialog);

	service = 0L;
	configWidget = 0L;
	middleFrameLayout = 0L;

	m_protocol = protocol;

	QString sName;
	if (account)
	{
		preferencesDialog->accountId->setText(account->accountId());
		//Disable changing the account ID for now
		//FIXME: Remove this when we can safely change the account ID (Matt)
		preferencesDialog->accountId->setReadOnly(true);
		sName = account->configGroup()->readEntry("ServiceName", QString());
		preferencesDialog->subEnable->setChecked(account->configGroup()->readEntry("SubEnable", false));
		preferencesDialog->subCode->setText(account->configGroup()->readEntry("SubCode", QString()));
		preferencesDialog->ifMessageTooLong->setCurrentIndex(SMSMsgAction(account->configGroup()->readEntry("MsgAction", 0)));
	}

	preferencesDialog->serviceName->addItems(ServiceLoader::services());

	connect (preferencesDialog->serviceName, SIGNAL(activated(QString)),
		this, SLOT(setServicePreferences(QString)));
	connect (preferencesDialog->descButton, SIGNAL(clicked()),
		this, SLOT(showDescription()));


	for (int i=0; i < preferencesDialog->serviceName->count(); i++)
	{
		if (preferencesDialog->serviceName->itemText(i) == sName)
		{
			preferencesDialog->serviceName->setCurrentIndex(i);
			break;
		}
	}
	setServicePreferences(preferencesDialog->serviceName->currentText());
}

SMSEditAccountWidget::~SMSEditAccountWidget()
{
	delete service;
}

bool SMSEditAccountWidget::validateData()
{
	return true;
}

Kopete::Account* SMSEditAccountWidget::apply()
{
	if (!account())
		setAccount( new SMSAccount( m_protocol, preferencesDialog->accountId->text() ) );

	if (service)
		service->setAccount(account());
	
	KConfigGroup *c = account()->configGroup();
	c->writeEntry("ServiceName", preferencesDialog->serviceName->currentText());
	c->writeEntry("SubEnable", preferencesDialog->subEnable->isChecked() ? "true" : "false");
	c->writeEntry("SubCode", preferencesDialog->subCode->text());
	c->writeEntry("MsgAction", preferencesDialog->ifMessageTooLong->currentIndex());

	emit saved();
	return account();
}

void SMSEditAccountWidget::setServicePreferences(const QString& serviceName)
{
	delete service;
	delete configWidget;

	service = ServiceLoader::loadService(serviceName, account());

	if (service == 0L)
		return;

	connect (this, SIGNAL(saved()), service, SLOT(savePreferences()));

	delete middleFrameLayout;
	middleFrameLayout = new QGridLayout(preferencesDialog->middleFrame);
	middleFrameLayout->setObjectName("middleFrameLayout");
	middleFrameLayout->setSpacing(6);
	middleFrameLayout->setMargin(0);
	service->setWidgetContainer(preferencesDialog->middleFrame, middleFrameLayout);
}

void SMSEditAccountWidget::showDescription()
{
	SMSService* s = ServiceLoader::loadService(preferencesDialog->serviceName->currentText(), 0L);

	QString d = s->description();

	KMessageBox::information(Kopete::UI::Global::mainWidget(), d, i18n("Description"));
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

