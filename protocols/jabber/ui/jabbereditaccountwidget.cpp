
/***************************************************************************
                   jabberaccountwidget.cpp  -  Account widget for Jabber
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           Based on code by Olivier Goffart <ogoffart@kde.org>
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabbereditaccountwidget.h"
#include <kdebug.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlabel.h>

#include <kconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <klineedit.h>
#include <kopetepassword.h>
#include <kopetepasswordedaccount.h>

//#include <solid/devicenotifier.h>
//#include <solid/device.h>

#include "kopetecontact.h"

#include "kopeteuiglobal.h"
#include "kopetepasswordwidget.h"

#include "jabberclient.h"
#include "jabberregisteraccount.h"
#include "dlgjabberchangepassword.h"
#include "privacydlg.h"

#include "xmpp.h"

#ifdef JINGLE_SUPPORT
//FIXME:Should be replaced by Solid.
#include "alsaio.h"
#endif

JabberEditAccountWidget::JabberEditAccountWidget (JabberProtocol * proto, JabberAccount * ident, QWidget * parent)
						: QWidget(parent), DlgJabberEditAccountWidget(), KopeteEditAccountWidget (ident)
{
	setupUi(this);

	m_protocol = proto;

	connect (mID, SIGNAL (textChanged(QString)), this, SLOT (updateServerField()));
	connect (cbCustomServer, SIGNAL (toggled(bool)), this, SLOT (updateServerField()));

	connect (cbUseSSL, SIGNAL (toggled(bool)), this, SLOT (sslToggled(bool)));

	connect (btnChangePassword, SIGNAL (clicked()), this, SLOT (slotChangePasswordClicked()));
	
	connect (privacyListsButton, SIGNAL (clicked()), this, SLOT (slotPrivacyListsClicked()) );

	connect (cbAdjustPriority, SIGNAL (toggled(bool)), this, SLOT (awayPriorityToggled(bool)));
	
#ifdef JINGLE_SUPPORT
	checkAudioDevices();
#else
	/*Remove the Jingle tab*/
	for (int i = 0; i < tabWidget10->count(); i++)
	{
		if (tabWidget10->tabText(i) == "&Jingle")
		{
			tabWidget10->removeTab(i);
			break;
		}
	}
#endif

#ifndef LIBJINGLE_SUPPORT
	//Remove Libjingle tab
	for ( int i=0; i<tabWidget10->count(); ++i )
	{
		if ( tabWidget10->tabText(i) == "&Libjingle" )
		{
			tabWidget10->removeTab(i);
			break;
		}
	}
#endif

	if (account())
	{
		// we are working with an existing account
		reopen ();
		registrationGroupBox->hide();
		btnRegister->setEnabled ( false );
		
		if ( account()->myself()->isOnline() )
			privacyListsButton->setEnabled (true);
		else
			privacyListsButton->setEnabled (false);
	}
	else
	{
		// this is a new account
		changePasswordGroupBox->hide();
		btnChangePassword->setEnabled ( false );
		connect (btnRegister, SIGNAL (clicked()), this, SLOT (registerClicked()));
		
		privacyListsButton->setEnabled (false);
	}
}

JabberEditAccountWidget::~JabberEditAccountWidget ()
{
}


#ifdef JINGLE_SUPPORT
void JabberEditAccountWidget::checkAudioDevices()
{
	kDebug() << "Start.";
	/*Solid::DeviceNotifier *notifier = Solid::DeviceNotifier::instance();
	foreach (const Solid::Device &device, Solid::Device::listFromType(Solid::DeviceInterface::AudioInterface, QString()))
	//foreach (const Solid::Device &device, Solid::Device::allDevices())
	{
		kDebug() << "Found :";
		kDebug() << device.udi().toLatin1().constData();
	}*/
	QList<Item> devices = getAlsaItems();
	for (int i = 0; i < devices.count(); i++)
	{
		if (devices.at(i).dir == Item::Input)
		{
			kDebug() << "Microphone :" << devices.at(i).name << "(" << devices.at(i).id << ")";
			audioInputsCombo->addItem(devices.at(i).name);
			inputDevices << devices.at(i);
		}
		else if (devices.at(i).dir == Item::Output)
		{
			kDebug() << "Audio output :" << devices.at(i).name << "(" << devices.at(i).id << ")";
			audioOutputsCombo->addItem(devices.at(i).name);
			outputDevices << devices.at(i);
		}

	}
	kDebug() << "End.";

}
#endif

JabberAccount *JabberEditAccountWidget::account ()
{

	return dynamic_cast<JabberAccount *>(KopeteEditAccountWidget::account () );

}

void JabberEditAccountWidget::reopen ()
{

	// FIXME: this is temporary until Kopete supports accound ID changes!
	mID->setReadOnly(true);

	mID->setText (account()->accountId ());
	mPass->load (&account()->password ());
	cbAutoConnect->setChecked (account()->excludeConnect());
	
	mResource->setText (account()->configGroup()->readEntry ("Resource", QString::fromLatin1("Kopete")));
	mPriority->setValue (account()->configGroup()->readEntry ("Priority", 5));
	mServer->setText (account()->configGroup()->readEntry ("Server", QString()));

	cbUseSSL->setChecked (account()->configGroup()->readEntry( "UseSSL", false));

	mPort->setValue (account()->configGroup()->readEntry("Port", 5222));

	QString auth = account()->configGroup()->readEntry("AuthType", QString());

	cbCustomServer->setChecked (account()->configGroup()->readEntry("CustomServer",false));

	if(cbCustomServer->isChecked ())
	{
		labelServer->setEnabled(true);
		mServer->setEnabled(true);
		labelPort->setEnabled(true);
		mPort->setEnabled(true);
	}
	else
	{
		mServer->setEnabled (false);
		mServer->setText(mID->text().section('@', 1));
	}

	if ( account()->configGroup()->hasKey("AwayPriority") )
	{
		cbAdjustPriority->setChecked(true);
		mAwayPriority->setValue( account()->configGroup()->readEntry("AwayPriority",0));
	}
	else
	{
		cbAdjustPriority->setChecked(false);
		mAwayPriority->setEnabled(false);
	}

	cbAllowPlainTextPassword->setChecked (account()->configGroup()->readEntry("AllowPlainTextPassword", true));

	KConfigGroup config = KGlobal::config()->group("Jabber");
	leLocalIP->setText (config.readEntry("LocalIP", QString()));
	sbLocalPort->setValue (config.readEntry("LocalPort", 8010));

	leProxyJID->setText (account()->configGroup()->readEntry("ProxyJID", QString()));

#ifdef JINGLE_SUPPORT
	//Jingle
	firstPortEdit->setValue(account()->configGroup()->readEntry("JingleFirstPort", QString("9000")).toInt());
	
	for (int i = 0; i < inputDevices.count(); i++)
	{
		if (inputDevices.at(i).id == account()->configGroup()->readEntry("JingleInputDevice", QString("default")))
		{
			audioInputsCombo->setCurrentIndex(i);
			break;
		}
	}
	
	for (int i = 0; i < outputDevices.count(); i++)
	{
		if (outputDevices.at(i).id == account()->configGroup()->readEntry("JingleOutputDevice", QString("default")))
		{
			audioOutputsCombo->setCurrentIndex(i);
			break;
		}
	}
	
	autoDetectIPBox->setChecked(account()->configGroup()->readEntry("JingleAutoDetectIP", false));
#endif

	// Privacy
	cbSendEvents->setChecked( account()->configGroup()->readEntry("SendEvents", true) );
	cbSendDeliveredEvent->setChecked( account()->configGroup()->readEntry("SendDeliveredEvent", true) );
	cbSendDisplayedEvent->setChecked( account()->configGroup()->readEntry("SendDisplayedEvent", true) );
	cbSendComposingEvent->setChecked( account()->configGroup()->readEntry("SendComposingEvent", true) );
	cbSendGoneEvent->setChecked( account()->configGroup()->readEntry("SendGoneEvent", true) );

	cbHideSystemInfo->setChecked( account()->configGroup()->readEntry("HideSystemInfo", false) );

	mergeMessages->setChecked(account()->mergeMessages());
	oldEncrypted->setChecked(account()->oldEncrypted());

#ifdef LIBJINGLE_SUPPORT
	Libjingle->setChecked(account()->enabledLibjingle());
#endif

}

Kopete::Account *JabberEditAccountWidget::apply ()
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "JabberEditAccount::apply()";

	if (!account())
	{
		setAccount(new JabberAccount (m_protocol, mID->text ()));
	}

	if(account()->isConnected())
	{
		KMessageBox::queuedMessageBox(this, KMessageBox::Information,
					i18n("The changes you just made will take effect next time you log in with Jabber."),
					i18n("Jabber Changes During Online Jabber Session"));
	}

	this->writeConfig ();

	static_cast<JabberAccount*>(account())->setS5BServerPort ( sbLocalPort->value () );

	return account();
}


void JabberEditAccountWidget::writeConfig ()
{
	account()->configGroup()->writeEntry("UseSSL", cbUseSSL->isChecked());

	mPass->save(&account()->password ());

	account()->configGroup()->writeEntry("CustomServer", cbCustomServer->isChecked());

	// FIXME: The call below represents a flaw in the current Kopete API.
	// Once the API is cleaned up, this will most likely require a change.
	//account()->setAccountId(mID->text());

	account()->configGroup()->writeEntry("AllowPlainTextPassword", cbAllowPlainTextPassword->isChecked());
	account()->configGroup()->writeEntry("Server", mServer->text().trimmed ());
	account()->configGroup()->writeEntry("Resource", mResource->text ());
	account()->configGroup()->writeEntry("Priority", QString::number (mPriority->value ()));

	if ( cbAdjustPriority->isChecked() )
	{
		account()->configGroup()->writeEntry("AwayPriority", QString::number( mAwayPriority->value ()));
	}
	else
	{
		account()->configGroup()->deleteEntry("AwayPriority");
	}

	account()->configGroup()->writeEntry("Port", QString::number (mPort->value ()));

#ifdef JINGLE_SUPPORT
	account()->configGroup()->writeEntry("JingleFirstPort", QString::number(firstPortEdit->value()));
	account()->configGroup()->writeEntry("JingleInputDevice", inputDevices.at(audioInputsCombo->currentIndex()).id);
	account()->configGroup()->writeEntry("JingleOutputDevice", outputDevices.at(audioOutputsCombo->currentIndex()).id);

	account()->configGroup()->writeEntry("JingleAutoDetectIP", autoDetectIPBox->isChecked());
#endif

	account()->setExcludeConnect(cbAutoConnect->isChecked());

	KConfigGroup config = KGlobal::config()->group("Jabber");
	
	config.writeEntry("LocalIP", leLocalIP->text());
	config.writeEntry("LocalPort", sbLocalPort->value());

	account()->configGroup()->writeEntry("ProxyJID", leProxyJID->text());

	// Privacy
	account()->configGroup()->writeEntry("SendEvents", cbSendEvents->isChecked());
	account()->configGroup()->writeEntry("SendDeliveredEvent", cbSendDeliveredEvent->isChecked());
	account()->configGroup()->writeEntry("SendDisplayedEvent", cbSendDisplayedEvent->isChecked());
	account()->configGroup()->writeEntry("SendComposingEvent", cbSendComposingEvent->isChecked());
	account()->configGroup()->writeEntry("SendGoneEvent", cbSendGoneEvent->isChecked());
	
	account()->configGroup()->writeEntry("HideSystemInfo", cbHideSystemInfo->isChecked());

	account()->setMergeMessages(mergeMessages->isChecked());
	account()->setOldEncrypted(oldEncrypted->isChecked());

#ifdef LIBJINGLE_SUPPORT
	account()->enableLibjingle(Libjingle->isChecked());
#endif

}

bool JabberEditAccountWidget::validateData ()
{

	if(!mID->text().contains('@'))
	{
		KMessageBox::sorry(this, i18n("The Jabber ID you have chosen is invalid. "
							"Please make sure it is in the form user@server.com, like an email address."),
							i18n("Invalid Jabber ID"));

		return false;
	}

	return true;
}

void JabberEditAccountWidget::updateServerField ()
{

	if(!cbCustomServer->isChecked())
	{
		QString newServer = mID->text().section('@', 1);
		mPort->setValue(5222);
		// check if ssl is enabled and set the port correctly
		sslToggled(cbUseSSL->isChecked());
		mServer->setText(newServer);
		labelServer->setEnabled(false);
		mServer->setEnabled(false);
		labelPort->setEnabled(false);
		mPort->setEnabled(false);
	}
	else
	{
		labelServer->setEnabled(true);
		mServer->setEnabled(true);
		labelPort->setEnabled(true);
		mPort->setEnabled(true);
	}

}

void JabberEditAccountWidget::awayPriorityToggled(bool enabled)
{
	mAwayPriority->setEnabled(enabled);
}

void JabberEditAccountWidget::deleteClicked ()
{

	// delete account here

}

void JabberEditAccountWidget::registerClicked ()
{

	JabberRegisterAccount *registerDlg = new JabberRegisterAccount ( this );

	registerDlg->show ();

}

void JabberEditAccountWidget::slotChangePasswordClicked ()
{

	DlgJabberChangePassword *passwordDlg = new DlgJabberChangePassword ( account (), this );

	connect ( passwordDlg, SIGNAL (destroyed()), this, SLOT (slotChangePasswordFinished()) );

	passwordDlg->show ();

}

void JabberEditAccountWidget::slotChangePasswordFinished ()
{

	// in case the password has been changed, we need to update it in the UI
	reopen ();

}

void JabberEditAccountWidget::sslToggled (bool value)
{
	if (value && (mPort->value() == 5222))
		mPort->stepUp ();
	else
		if(!value && (mPort->value() == 5223))
			mPort->stepDown ();
}

void JabberEditAccountWidget::slotPrivacyListsClicked()
{
	PrivacyDlg * dialog = new PrivacyDlg (account(), this);
	dialog->show();
}

#include "jabbereditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:
