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
#include <QFile>
#include <QDir>
#include <kinputdialog.h>

class SkypeEditAccountPrivate {
	public:
		///The protocol
		SkypeProtocol *protocol;
		///The account
		SkypeAccount *account;
};

skypeEditAccount::skypeEditAccount(SkypeProtocol *protocol, Kopete::Account *account, QWidget *parent) : QWidget(parent), KopeteEditAccountWidget(account) {
	kDebug(SKYPE_DEBUG_GLOBAL);

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
		if ( d->account->launchType == 0 ) {//set the launch type
			widget->LaunchNeededRadio->setChecked(true);
			widget->LaunchNeverRadio->setChecked(false);
		} else if ( d->account->launchType == 1 ) {
			widget->LaunchNeededRadio->setChecked(false);
			widget->LaunchNeverRadio->setChecked(true);
		}
		widget->AuthorCheck->setChecked(!d->account->author.isEmpty());//set the check box that allows you to change authorization
		if (widget->AuthorCheck->isChecked())
			widget->AuthorEdit->setText(d->account->author);//set the name
		widget->MarkCheck->setChecked(d->account->getMarkRead());//set the get read mode
		widget->HitchCheck->setChecked(d->account->getHitchHike());
		widget->ScanCheck->setChecked(d->account->getScanForUnread());
		widget->CallCheck->setChecked(d->account->getCallControl());
		widget->PingsCheck->setChecked(d->account->getPings());
		if ( d->account->getBus() == 0 ) {
			widget->radioButton4->setChecked(true);
			widget->radioButton5->setChecked(false);
		} else if ( d->account->getBus() == 1 ) {
			widget->radioButton4->setChecked(false);
			widget->radioButton5->setChecked(true);
		}
		widget->LaunchSpin->setValue(d->account->getLaunchTimeout());
		widget->CommandEdit->setText(d->account->getSkypeCommand());
		widget->WaitSpin->setValue(d->account->getWaitBeforeConnect());
		if (d->account->closeCallWindowTimeout()) {
			widget->AutoCloseCallCheck->setChecked(true);
			widget->CloseTimeoutSpin->setValue(d->account->closeCallWindowTimeout());
		} else widget->AutoCloseCallCheck->setChecked(false);
		widget->LeaveCheck->setChecked(!d->account->leaveOnExit());
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

	connect( widget->configureSkypeClient, SIGNAL(clicked()), this, SLOT(configureSkypeClient()) );
}

skypeEditAccount::~skypeEditAccount() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	disconnect( widget->configureSkypeClient, SIGNAL(clicked()), this, SLOT(configureSkypeClient()) );
	delete widget;
	delete d;
}

bool skypeEditAccount::validateData() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (d->protocol->hasAccount() && (!account())) {//he wants to create some account witch name is already used
		KMessageBox::sorry(this, i18n("You can have only one skype account"), i18n("Wrong Information"));//Tell him to use something other
		return false;
	}

	return true;//It seems OK
}

Kopete::Account *skypeEditAccount::apply() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	//first, I need a pointer to that account
	if (!account()) //it does not exist
		setAccount(new SkypeAccount(d->protocol, "Skype" ));//create a new one
	SkypeAccount *skype = static_cast<SkypeAccount *>(account());//get the account

	//set it's values
	skype->setExcludeConnect(widget->excludeCheck->isChecked());//Save the "exclude from connection" setup

	if ( widget->LaunchNeverRadio->isChecked() )//get the type how to launch skype
		skype->launchType = 1;
	else if ( widget->LaunchNeededRadio->isChecked() )
		skype->launchType = 0;

	if (widget->AuthorCheck->isChecked())
		skype->author = widget->AuthorEdit->text();//put there what user wrote
	else
		skype->author = "";//nothing unusual

	skype->setHitchHike(widget->HitchCheck->isChecked());//save the hitch hike mode and activat ethe new value
	skype->setMarkRead(widget->MarkCheck->isChecked());//set the mark read messages mode and activate it
	skype->setScanForUnread(widget->ScanCheck->isChecked());
	skype->setCallControl(widget->CallCheck->isChecked());
	skype->setPings(widget->PingsCheck->isChecked());

	if ( widget->radioButton4->isChecked() )
		skype->setBus(0);
	else if ( widget->radioButton5->isChecked() )
		skype->setBus(1);

	skype->setLaunchTimeout(widget->LaunchSpin->value());
	skype->setSkypeCommand(widget->CommandEdit->text());
	skype->setWaitBeforeConnect(widget->WaitSpin->value());
	skype->setLeaveOnExit(!widget->LeaveCheck->isChecked());
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

void skypeEditAccount::configureSkypeClient() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if ( ! account() )
		setAccount(new SkypeAccount(d->protocol, "Skype"));

	QByteArray authAppName = ( static_cast <SkypeAccount *> (account()) )->author.toUtf8();

	if ( authAppName.isEmpty() )
		authAppName = "Kopete";

	QString skypeUser = ( static_cast <SkypeAccount *> (account()) )->getMyselfSkypeName();

	if ( skypeUser.isEmpty() )
		skypeUser = KInputDialog::getText(i18n("Configure Skype client"), i18n("Please enter your skype user name"));

	if ( skypeUser.isEmpty() ) {
		KMessageBox::error(this, i18n("You must enter your skype user name"), i18n("Skype protocol"));
		return;
	}

	const QString &skypeDir = QString("%1/.Skype").arg(QDir::homePath());
	QDir dir(skypeDir);

	if ( ! dir.exists(skypeDir) )
		dir.mkpath(skypeDir);

	const QString &sharedPath = QString("%1/shared.xml").arg(skypeDir);
	QFile sharedFile(sharedPath);

	if ( ! sharedFile.open(QIODevice::WriteOnly) ) {
		kDebug(SKYPE_DEBUG_GLOBAL) << "Cant create/open file" << sharedPath;
		KMessageBox::error(this, i18n("Cannot create/open file %1 for configuring the Skype client.", sharedPath), i18n("Skype protocol"));
		return;
	}

	sharedFile.reset();
	sharedFile.write("<?xml version=\"1.0\"?>\n<config version=\"1.0\" serial=\"9\" timestamp=\"0\">\n  <UI>\n    <Installed>2</Installed>\n    <Language>en</Language>\n    <SavePassword>1</SavePassword>\n    <StartMinimized>1</StartMinimized>\n  </UI>\n</config>\n"); //This only works with Linux Skype Client versions: 2.0.0.72 2.1.0.47 2.1.0.81
	sharedFile.close();

	const QString &userDir = QString("%1/%2").arg(skypeDir).arg(skypeUser);
	if ( ! dir.exists(userDir) )
		dir.mkpath(userDir);

	const QString &configPath = QString("%1/config.xml").arg(userDir);
	QFile configFile(configPath);

	if ( ! configFile.open(QIODevice::WriteOnly) ) {
		kDebug(SKYPE_DEBUG_GLOBAL) << "Cant create/open file" << configPath;
		KMessageBox::error(this, i18n("Cannot create/open file %1 for configuring the Skype client.", configPath), i18n("Skype protocol"));
		return;
	}

	configFile.reset();
	configFile.write("<?xml version=\"1.0\"?>\n<config version=\"1.0\" serial=\"9\" timestamp=\"0\">\n  <UI>\n    <API>\n      <Authorizations>" + authAppName + "</Authorizations>\n    </API>\n    <Notifications>\n      <Enable>\n        <Birthday>0</Birthday>\n        <CallAnswered>1</CallAnswered>\n        <CallBusy>1</CallBusy>\n        <CallFailed>1</CallFailed>\n        <CallHangup>1</CallHangup>\n        <CallHold>1</CallHold>\n        <CallMissed>1</CallMissed>\n        <CallRemoteHangup>1</CallRemoteHangup>\n        <CallResume>1</CallResume>\n        <CallRingingIn>1</CallRingingIn>\n        <CallRingingOut>1</CallRingingOut>\n        <ChatIncoming>0</ChatIncoming>\n        <ChatIncomingInitial>0</ChatIncomingInitial>\n        <ChatJoined>0</ChatJoined>\n        <ChatOutgoing>0</ChatOutgoing>\n        <ChatParted>0</ChatParted>\n        <ContactAdded>0</ContactAdded>\n        <ContactAuthRequest>0</ContactAuthRequest>\n        <ContactDeleted>0</ContactDeleted>\n        <ContactOffline>0</ContactOffline>\n        <ContactOnline>0</ContactOnline>\n        <SkypeLogin>0</SkypeLogin>\n        <SkypeLoginFailed>0</SkypeLoginFailed>\n        <SkypeLogout>0</SkypeLogout>\n        <TransferComplete>1</TransferComplete>\n        <TransferFailed>1</TransferFailed>\n        <VoicemailReceived>1</VoicemailReceived>\n        <VoicemailSent>1</VoicemailSent>\n      </Enable>\n    </Notifications>\n    <Notify>\n      <Call>0</Call>\n    </Notify>\n  </UI>\n</config>\n"); //This only works with Linux Skype Client versions: 2.0.0.72 2.1.0.47 2.1.0.81
	configFile.close();

	KMessageBox::information(this, i18n("Process has completed.\nSkype is now configured for Kopete.\nYou must restart the Skype client for changes to take effect."), i18n("Skype protocol"));
}

#include "skypeeditaccount.moc"
