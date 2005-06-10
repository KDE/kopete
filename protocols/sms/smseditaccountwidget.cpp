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

#include <qvgroupbox.h>
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

#include "kopeteuiglobal.h"

#include "smseditaccountwidget.h"
#include "smsactprefs.h"
#include "serviceloader.h"
#include "smsprotocol.h"
#include "smsaccount.h"

SMSEditAccountWidget::SMSEditAccountWidget(SMSProtocol *protocol, Kopete::Account *account, QWidget *parent, const char */*name*/)
	: QWidget(parent), KopeteEditAccountWidget(account)
{
	QVBoxLayout *l = new QVBoxLayout(this, QBoxLayout::Down);
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
		preferencesDialog->accountId->setDisabled(true);
		sName = account->configGroup()->readEntry("ServiceName", QString::null);
		preferencesDialog->subEnable->setChecked(account->configGroup()->readBoolEntry("SubEnable", false));
		preferencesDialog->subCode->setText(account->configGroup()->readEntry("SubCode", QString::null));
		preferencesDialog->ifMessageTooLong->setCurrentItem(SMSMsgAction(account->configGroup()->readNumEntry("MsgAction", 0)));
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
	c->writeEntry("MsgAction", preferencesDialog->ifMessageTooLong->currentItem());

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
	middleFrameLayout = new QGridLayout(preferencesDialog->middleFrame, 1, 2, 0, 6, "middleFrameLayout");
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

