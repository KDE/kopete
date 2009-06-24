/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>
    Copyright (C) 2008-2009 Pali Rohár <pali.rohar@gmail.com>

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
	QWidget *w = new QWidget( this );
	widget = new Ui::SkypeEditAccountBase();
	widget->setupUi( w );
	layout->addWidget( w );

	d = new SkypeEditAccountPrivate();//the d pointer
	d->protocol = protocol;//I may need the protocol later

	d->account = (SkypeAccount *) account;//save the account

	//Now, check weather it is existing account or just an old one to modify
	if (account) {//it is old one
		widget->excludeCheck->setChecked(account->excludeConnect());//Check, weather it should be excluded
		//LaunchGroup->setButton(d->account->launchType);//set the launch type
		widget->AuthorCheck->setChecked(!d->account->author.isEmpty());//set the check box that allows you to change authorization
		if (widget->AuthorCheck->isChecked())
			widget->AuthorEdit->setText(d->account->author);//set the name
		widget->MarkCheck->setChecked(d->account->getMarkRead());//set the get read mode
		widget->HitchCheck->setChecked(d->account->getHitchHike());
		widget->ScanCheck->setChecked(d->account->getScanForUnread());
		widget->CallCheck->setChecked(d->account->getCallControl());
		widget->PingsCheck->setChecked(d->account->getPings());
		//BusGroup->setButton(d->account->getBus());
		widget->LaunchSpin->setValue(d->account->getLaunchTimeout());
		widget->CommandEdit->setText(d->account->getSkypeCommand());
		widget->WaitSpin->setValue(d->account->getWaitBeforeConnect());
		if (d->account->closeCallWindowTimeout()) {
			widget->AutoCloseCallCheck->setChecked(true);
			widget->CloseTimeoutSpin->setValue(d->account->closeCallWindowTimeout());
		} else widget->AutoCloseCallCheck->setChecked(false);
		widget->LeaveCheck->setChecked(d->account->leaveOnExit());
		const QString &startCallCommand = d->account->startCallCommand();
		widget->StartCallCommandCheck->setChecked(!startCallCommand.isEmpty());
		widget->StartCallCommandEdit->setText(startCallCommand);
		widget->WaitForStartCallCommandCheck->setChecked(d->account->waitForStartCallCommand());
		const QString &endCallCommand = d->account->endCallCommand();
		widget->EndCallCommandCheck->setChecked(!endCallCommand.isEmpty());
		widget->EndCallCommandEdit->setText(endCallCommand);
		widget->OnlyLastCallCommandCheck->setChecked(d->account->endCallCommandOnlyLast());
		const QString &incomingCommand = d->account->incomingCommand();
		widget->IncomingCommandCheck->setChecked(!incomingCommand.isEmpty());
		widget->IncomingCommandEdit->setText(incomingCommand);
	} else {
		//KMessageBox::information(this, i18n("Please note that this version of Skype plugin is a development version and it is probable it will cause more problems than solve. You have been warned"), i18n("Version info")); - I hope it is not needed any more
	}
}

skypeEditAccount::~skypeEditAccount() {
	kDebug() << k_funcinfo << endl;//some debug info

	delete widget;
	delete d;
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
		setAccount(new SkypeAccount(d->protocol, "Skype" ));//create a new one
	SkypeAccount *skype = static_cast<SkypeAccount *>(account());//get the account

	//set it's values
	skype->setExcludeConnect(widget->excludeCheck->isChecked());//Save the "exclude from connection" setup
	skype->launchType = widget->LaunchGroup->selectedId();//get the type how to launch skype
	if (widget->AuthorCheck->isChecked())
		skype->author = widget->AuthorEdit->text();//put there what user wrote
	else
		skype->author = "";//nothing unusual
	skype->setHitchHike(widget->HitchCheck->isChecked());//save the hitch hike mode and activat ethe new value
	skype->setMarkRead(widget->MarkCheck->isChecked());//set the mark read messages mode and activate it
	skype->setScanForUnread(widget->ScanCheck->isChecked());
	skype->setCallControl(widget->CallCheck->isChecked());
	skype->setPings(widget->PingsCheck->isChecked());
	skype->setBus(widget->BusGroup->selectedId());
	skype->setLaunchTimeout(widget->LaunchSpin->value());
	skype->setSkypeCommand(widget->CommandEdit->text());
	skype->setWaitBeforeConnect(widget->WaitSpin->value());
	skype->setLeaveOnExit(widget->LeaveCheck->isChecked());
	if (widget->AutoCloseCallCheck->isChecked()) {
		skype->setCloseWindowTimeout(widget->CloseTimeoutSpin->value());
	} else {
		skype->setCloseWindowTimeout(0);
	}
	if (widget->StartCallCommandCheck->isChecked()) {
		skype->setStartCallCommand(widget->StartCallCommandEdit->text());
	} else {
		skype->setStartCallCommand("");
	}
	skype->setWaitForStartCallCommand(widget->WaitForStartCallCommandCheck->isChecked());
	if (widget->EndCallCommandCheck->isChecked()) {
		skype->setEndCallCommand(widget->EndCallCommandEdit->text());
	} else {
		skype->setEndCallCommand("");
	}
	if (widget->IncomingCommandCheck->isChecked()) {
		skype->setIncomingCommand(widget->IncomingCommandEdit->text());
	} else {
		skype->setIncomingCommand("");
	}

	skype->setEndCallCommandOnlyForLast(widget->OnlyLastCallCommandCheck->isChecked());
	skype->save();//save it to config
	return skype;//return the account
}

#include "skypeeditaccount.moc"
