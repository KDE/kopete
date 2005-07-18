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

skypeEditAccount::skypeEditAccount(SkypeProtocol *protocol, Kopete::Account *account, QWidget *parent) : SkypeEditAccountBase(parent), KopeteEditAccountWidget(account) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d = new SkypeEditAccountPrivate();//the d pointer
	d->protocol = protocol;//I may need the protocol later

	d->account = (SkypeAccount *) account;//save the account

	//Now, check weather it is existing account or just an old one to modify
	if (account) {//it is old one
		excludeCheck->setChecked(account->excludeConnect());//Check, weather it should be excluded
		LaunchGroup->setButton(d->account->launchType);//set the launch type
		AuthorCheck->setChecked(!d->account->author.isEmpty());//set the check box that allows you to change authorization
		if (AuthorCheck->isChecked())
			AuthorEdit->setText(d->account->author);//set the name
		MarkCheck->setChecked(d->account->getMarkRead());//set the get read mode
		HitchCheck->setChecked(d->account->getHitchHike());
		ScanCheck->setChecked(d->account->getScanForUnread());
		CallCheck->setChecked(d->account->getCallControl());
		PingsCheck->setChecked(d->account->getPings());
		BusGroup->setButton(d->account->getBus());
		DBusCheck->setChecked(d->account->getStartDBus());
		LaunchSpin->setValue(d->account->getLaunchTimeout());
		CommandEdit->setText(d->account->getSkypeCommand());
		WaitSpin->setValue(d->account->getWaitBeforeConnect());
		if (d->account->closeCallWindowTimeout()) {
			AutoCloseCallCheck->setChecked(true);
			CloseTimeoutSpin->setValue(d->account->closeCallWindowTimeout());
		} else AutoCloseCallCheck->setChecked(false);
		LeaveCheck->setChecked(d->account->leaveOnExit());
		const QString &startCallCommand = d->account->startCallCommand();
		StartCallCommandCheck->setChecked(!startCallCommand.isEmpty());
		StartCallCommandEdit->setText(startCallCommand);
		WaitForStartCallCommandCheck->setChecked(d->account->waitForStartCallCommand());
		const QString &endCallCommand = d->account->endCallCommand();
		EndCallCommandCheck->setChecked(!endCallCommand.isEmpty());
		EndCallCommandEdit->setText(endCallCommand);
		OlnlyLastCallCommandCheck->setChecked(d->account->endCallCommandOnlyLast());
	} else {
		//TODO Make this unneeded :)
		KMessageBox::information(this, i18n("Please note that this version of Skype plugin is a development version and it is probable it will cause more problems than solve. You have been warned"), i18n("Version info"));
	}
}

skypeEditAccount::~skypeEditAccount() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
}

bool skypeEditAccount::validateData() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (d->protocol->hasAccount() && (!account())) {//he wants to create some account witch name is already used
		KMessageBox::sorry(this, i18n("You can have only one skype account"), i18n("Wrong information"));//Tell him to use something other
		return false;
	}

	return true;//It seems OK
}

Kopete::Account *skypeEditAccount::apply() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	//first, I need a pointer to that account
	if (!account()) //it does not exist
		setAccount(new SkypeAccount(d->protocol));//create a new one
	SkypeAccount *skype = static_cast<SkypeAccount *>(account());//get the account

	//set it's values
	skype->setExcludeConnect(excludeCheck->isChecked());//Save the "exclude from connection" setup
	skype->launchType = LaunchGroup->selectedId();//get the type how to launch skype
	if (AuthorCheck->isChecked())
		skype->author = AuthorEdit->text();//put there what user wrote
	else
		skype->author = "";//nothing unusual
	skype->setHitchHike(HitchCheck->isChecked());//save the hitch hike mode and activat ethe new value
	skype->setMarkRead(MarkCheck->isChecked());//set the mark read messages mode and activate it
	skype->setScanForUnread(ScanCheck->isChecked());
	skype->setCallControl(CallCheck->isChecked());
	skype->setPings(PingsCheck->isChecked());
	skype->setBus(BusGroup->selectedId());
	skype->setStartDBus(DBusCheck->isChecked());
	skype->setLaunchTimeout(LaunchSpin->value());
	skype->setSkypeCommand(CommandEdit->text());
	skype->setWaitBeforeConnect(WaitSpin->value());
	skype->setLeaveOnExit(LeaveCheck->isChecked());
	if (AutoCloseCallCheck->isChecked()) {
		skype->setCloseWindowTimeout(CloseTimeoutSpin->value());
	} else {
		skype->setCloseWindowTimeout(0);
	}
	if (StartCallCommandCheck->isChecked()) {
		skype->setStartCallCommand(StartCallCommandEdit->text());
	} else {
		skype->setStartCallCommand("");
	}
	skype->setWaitForStartCallCommand(WaitForStartCallCommandCheck->isChecked());
	if (EndCallCommandCheck->isChecked()) {
		skype->setEndCallCommand(EndCallCommandEdit->text());
	} else {
		skype->setEndCallCommand("");
	}
	skype->setEndCallCommandOnlyForLast(OlnlyLastCallCommandCheck->isChecked());
	skype->save();//save it to config
	return skype;//return the account
}

#include "skypeeditaccount.moc"
