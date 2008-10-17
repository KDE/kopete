/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/


#include "skypeeditaccount.h"
#include "skypeprotocol.h"
#include "skypeaccount.h"

#include <qlineedit.h>
#include <qstring.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kopeteaccountmanager.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <kdebug.h>
#include <qbuttongroup.h>
#include <qspinbox.h>

class SkypeEditAccountPrivate {
	public:
		///The protocol
		SkypeProtocol *protocol;
		///The account
		SkypeAccount *account;
};

skypeEditAccount::skypeEditAccount(SkypeProtocol *protocol, Kopete::Account *account, QWidget *parent) : QWidget(parent), KopeteEditAccountWidget(account) {
	kDebug() << k_funcinfo << endl;//some debug info

	QVBoxLayout *layout = new QVBoxLayout( this );
				kDebug(14210) ;
	QWidget *widget = new QWidget( this );
	m_preferencesWidget = new Ui::SkypeEditAccountBase();
	m_preferencesWidget->setupUi( widget );
	layout->addWidget( widget );

	d = new SkypeEditAccountPrivate();//the d pointer
	d->protocol = protocol;//I may need the protocol later

	d->account = (SkypeAccount *) account;//save the account

	//Now, check weather it is existing account or just an old one to modify
	if (account) {//it is old one
		m_preferencesWidget->excludeCheck->setChecked(account->excludeConnect());//Check, weather it should be excluded
		//LaunchGroup->setButton(d->account->launchType);//set the launch type
		m_preferencesWidget->AuthorCheck->setChecked(!d->account->author.isEmpty());//set the check box that allows you to change authorization
		if (m_preferencesWidget->AuthorCheck->isChecked())
			m_preferencesWidget->AuthorEdit->setText(d->account->author);//set the name
		m_preferencesWidget->MarkCheck->setChecked(d->account->getMarkRead());//set the get read mode
		m_preferencesWidget->HitchCheck->setChecked(d->account->getHitchHike());
		m_preferencesWidget->ScanCheck->setChecked(d->account->getScanForUnread());
		m_preferencesWidget->CallCheck->setChecked(d->account->getCallControl());
		m_preferencesWidget->PingsCheck->setChecked(d->account->getPings());
		//BusGroup->setButton(d->account->getBus());
		m_preferencesWidget->DBusCheck->setChecked(d->account->getStartDBus());
		m_preferencesWidget->LaunchSpin->setValue(d->account->getLaunchTimeout());
		m_preferencesWidget->CommandEdit->setText(d->account->getSkypeCommand());
		m_preferencesWidget->WaitSpin->setValue(d->account->getWaitBeforeConnect());
		if (d->account->closeCallWindowTimeout()) {
			m_preferencesWidget->AutoCloseCallCheck->setChecked(true);
			m_preferencesWidget->CloseTimeoutSpin->setValue(d->account->closeCallWindowTimeout());
		} else m_preferencesWidget->AutoCloseCallCheck->setChecked(false);
		m_preferencesWidget->LeaveCheck->setChecked(d->account->leaveOnExit());
		const QString &startCallCommand = d->account->startCallCommand();
		m_preferencesWidget->StartCallCommandCheck->setChecked(!startCallCommand.isEmpty());
		m_preferencesWidget->StartCallCommandEdit->setText(startCallCommand);
		m_preferencesWidget->WaitForStartCallCommandCheck->setChecked(d->account->waitForStartCallCommand());
		const QString &endCallCommand = d->account->endCallCommand();
		m_preferencesWidget->EndCallCommandCheck->setChecked(!endCallCommand.isEmpty());
		m_preferencesWidget->EndCallCommandEdit->setText(endCallCommand);
		m_preferencesWidget->OnlyLastCallCommandCheck->setChecked(d->account->endCallCommandOnlyLast());
		const QString &incomingCommand = d->account->incomingCommand();
		m_preferencesWidget->IncomingCommandCheck->setChecked(!incomingCommand.isEmpty());
		m_preferencesWidget->IncomingCommandEdit->setText(incomingCommand);
	} else {
		//KMessageBox::information(this, i18n("Please note that this version of Skype plugin is a development version and it is probable it will cause more problems than solve. You have been warned"), i18n("Version info")); - I hope it is not needed any more
	}
}

skypeEditAccount::~skypeEditAccount() {
	kDebug() << k_funcinfo << endl;//some debug info
}

bool skypeEditAccount::validateData() {
	kDebug() << k_funcinfo << endl;//some debug info

	if (d->protocol->hasAccount() && (!account())) {//he wants to create some account witch name is already used
		KMessageBox::sorry(this, i18n("You can have only one skype account"), i18n("Wrong Information"));//Tell him to use something other
		return false;
	}

	return true;//It seems OK
}

Kopete::Account *skypeEditAccount::apply() {
	kDebug() << k_funcinfo << endl;//some debug info

	//first, I need a pointer to that account
	if (!account()) //it does not exist
		setAccount(new SkypeAccount(d->protocol, QString() ));//create a new one
	SkypeAccount *skype = static_cast<SkypeAccount *>(account());//get the account

	//set it's values
	skype->setExcludeConnect(m_preferencesWidget->excludeCheck->isChecked());//Save the "exclude from connection" setup
	skype->launchType = m_preferencesWidget->LaunchGroup->selectedId();//get the type how to launch skype
	if (m_preferencesWidget->AuthorCheck->isChecked())
		skype->author = m_preferencesWidget->AuthorEdit->text();//put there what user wrote
	else
		skype->author = "";//nothing unusual
	skype->setHitchHike(m_preferencesWidget->HitchCheck->isChecked());//save the hitch hike mode and activat ethe new value
	skype->setMarkRead(m_preferencesWidget->MarkCheck->isChecked());//set the mark read messages mode and activate it
	skype->setScanForUnread(m_preferencesWidget->ScanCheck->isChecked());
	skype->setCallControl(m_preferencesWidget->CallCheck->isChecked());
	skype->setPings(m_preferencesWidget->PingsCheck->isChecked());
	skype->setBus(m_preferencesWidget->BusGroup->selectedId());
	skype->setStartDBus(m_preferencesWidget->DBusCheck->isChecked());
	skype->setLaunchTimeout(m_preferencesWidget->LaunchSpin->value());
	skype->setSkypeCommand(m_preferencesWidget->CommandEdit->text());
	skype->setWaitBeforeConnect(m_preferencesWidget->WaitSpin->value());
	skype->setLeaveOnExit(m_preferencesWidget->LeaveCheck->isChecked());
	if (m_preferencesWidget->AutoCloseCallCheck->isChecked()) {
		skype->setCloseWindowTimeout(m_preferencesWidget->CloseTimeoutSpin->value());
	} else {
		skype->setCloseWindowTimeout(0);
	}
	if (m_preferencesWidget->StartCallCommandCheck->isChecked()) {
		skype->setStartCallCommand(m_preferencesWidget->StartCallCommandEdit->text());
	} else {
		skype->setStartCallCommand("");
	}
	skype->setWaitForStartCallCommand(m_preferencesWidget->WaitForStartCallCommandCheck->isChecked());
	if (m_preferencesWidget->EndCallCommandCheck->isChecked()) {
		skype->setEndCallCommand(m_preferencesWidget->EndCallCommandEdit->text());
	} else {
		skype->setEndCallCommand("");
	}
	if (m_preferencesWidget->IncomingCommandCheck->isChecked()) {
		skype->setIncomingCommand(m_preferencesWidget->IncomingCommandEdit->text());
	} else {
		skype->setIncomingCommand("");
	}

	skype->setEndCallCommandOnlyForLast(m_preferencesWidget->OnlyLastCallCommandCheck->isChecked());
	skype->save();//save it to config
	return skype;//return the account
}

#include "skypeeditaccount.moc"
