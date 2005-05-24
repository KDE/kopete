
/***************************************************************************
                   jabberaccountwidget.cpp  -  Account widget for Jabber
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           Based on code by Olivier Goffart <ogoffart @ kde.org>
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

#include <kdebug.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlabel.h>

#include <kconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kpassdlg.h>
#include <kopetepassword.h>
#include <kopetepasswordedaccount.h>

#include "kopeteuiglobal.h"
#include "kopetepasswordwidget.h"

#include "jabbereditaccountwidget.h"
#include "jabberregisteraccount.h"

JabberEditAccountWidget::JabberEditAccountWidget (JabberProtocol * proto, JabberAccount * ident, QWidget * parent, const char *name)
						: DlgJabberEditAccountWidget (parent, name), KopeteEditAccountWidget (ident)
{

	m_protocol = proto;

	connect (mID, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPass, SIGNAL (changed ()), this, SLOT (configChanged ()));
	connect (mResource, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPriority, SIGNAL (valueChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mServer, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPort, SIGNAL (valueChanged (int)), this, SLOT (configChanged ()));

	connect (cbAutoConnect, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbUseSSL, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbCustomServer, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbAllowPlainTextPassword, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));

	connect (mID, SIGNAL (textChanged (const QString &)), this, SLOT (updateServerField ()));
	connect (cbCustomServer, SIGNAL (toggled (bool)), this, SLOT (updateServerField ()));

	connect (cbUseSSL, SIGNAL (toggled (bool)), this, SLOT (sslToggled (bool)));

	connect (leLocalIP, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (sbLocalPort, SIGNAL (valueChanged (int)), this, SLOT (configChanged ()));
	connect (leProxyJID, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	
	connect (btnChangePw, SIGNAL ( clicked() ), this, SLOT ( slotChangePwClicked () ));

	tlChangePwSuccess->hide();
	tlChangePassError->hide();
	if (account())
	{
		this->reopen ();
		btnRegister->setEnabled ( false );
	}
	else
	{
		btnChangePw->setEnabled( false );
		pePassword1->setEnabled( false );
		pePassword2->setEnabled( false );
		connect (btnRegister, SIGNAL (clicked ()), this, SLOT (registerClicked ()));
	}
	QWidget::setTabOrder( mID, mPass->mRemembered );
	QWidget::setTabOrder( mPass->mRemembered, mPass->mPassword );
	QWidget::setTabOrder( mPass->mPassword, cbAutoConnect );
}

JabberEditAccountWidget::~JabberEditAccountWidget ()
{
}

JabberAccount *JabberEditAccountWidget::account ()
{

	return dynamic_cast<JabberAccount *>(KopeteEditAccountWidget::account () );

}

void JabberEditAccountWidget::reopen ()
{

	// FIXME: this is temporary until Kopete supports accound ID changes!
	mID->setDisabled(true);

	mID->setText (account()->accountId ());
	mPass->load (&account()->password ());
	
	mResource->setText (account()->configGroup()->readEntry ("Resource", QString::fromLatin1("Kopete")));
	mPriority->setValue (account()->configGroup()->readNumEntry ("Priority", 5));
	mServer->setText (account()->configGroup()->readEntry ("Server", QString::null));

	cbUseSSL->setChecked (account()->configGroup()->readBoolEntry( "UseSSL", false));

	mPort->setValue (account()->configGroup()->readNumEntry("Port", 5222));

	QString auth = account()->configGroup()->readEntry("AuthType", QString::null);

	cbCustomServer->setChecked (account()->configGroup()->readBoolEntry("CustomServer",false));

	if(cbCustomServer->isChecked ())
	{
		mServer->setEnabled(true);
		mPort->setEnabled(true);
	}
	else
	{
		mServer->setEnabled (false);
		mServer->setText(mID->text().section("@", 1));
	}

	cbAllowPlainTextPassword->setChecked (account()->configGroup()->readBoolEntry("AllowPlainTextPassword", true));

	KGlobal::config()->setGroup("Jabber");
	leLocalIP->setText (KGlobal::config()->readEntry("LocalIP", ""));
	sbLocalPort->setValue (KGlobal::config()->readNumEntry("LocalPort", 8010));

	leProxyJID->setText (account()->configGroup()->readEntry("ProxyJID", QString::null));

	// Privacy
	cbSendEvents->setChecked( account()->configGroup()->readBoolEntry("SendEvents", true) );
	cbSendDeliveredEvent->setChecked( account()->configGroup()->readBoolEntry("SendDeliveredEvent", true) );
	cbSendDisplayedEvent->setChecked( account()->configGroup()->readBoolEntry("SendDisplayedEvent", true) );
	cbSendComposingEvent->setChecked( account()->configGroup()->readBoolEntry("SendComposingEvent", true) );

	cbHideSystemInfo->setChecked( account()->configGroup()->readBoolEntry("HideSystemInfo", false) );
}

Kopete::Account *JabberEditAccountWidget::apply ()
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << "JabberEditAccount::apply()" << endl;

	if (!account())
	{
		setAccount(new JabberAccount (m_protocol, mID->text ()));
	}

	if(account()->isConnected())
	{
		KMessageBox::information(this,
					i18n("The changes you just made will take effect next time you log in with Jabber."),
					i18n("Jabber Changes During Online Jabber Session"));
	}

	this->writeConfig ();

	static_cast<JabberAccount*>(account())->setS5bPort ( sbLocalPort->value () );

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
	account()->configGroup()->writeEntry("Server", mServer->text ());
	account()->configGroup()->writeEntry("Resource", mResource->text ());
	account()->configGroup()->writeEntry("Priority", QString::number (mPriority->value ()));
	account()->configGroup()->writeEntry("Port", QString::number (mPort->value ()));

	account()->setExcludeConnect(cbAutoConnect->isChecked());

	KGlobal::config()->setGroup("Jabber");
	KGlobal::config()->writeEntry("LocalIP", leLocalIP->text());
	KGlobal::config()->writeEntry("LocalPort", sbLocalPort->value());

	account()->configGroup()->writeEntry("ProxyJID", leProxyJID->text());

	// Privacy
	account()->configGroup()->writeEntry("SendEvents", cbSendEvents->isChecked());
	account()->configGroup()->writeEntry("SendDeliveredEvent", cbSendDeliveredEvent->isChecked());
	account()->configGroup()->writeEntry("SendDisplayedEvent", cbSendDisplayedEvent->isChecked());
	account()->configGroup()->writeEntry("SendComposingEvent", cbSendComposingEvent->isChecked());
	
	account()->configGroup()->writeEntry("HideSystemInfo", cbHideSystemInfo->isChecked());
	settings_changed = false;
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

void JabberEditAccountWidget::configChanged ()
{
	settings_changed = true;
}

void JabberEditAccountWidget::updateServerField ()
{

	if(!cbCustomServer->isChecked())
	{
		QString newServer = mID->text().section("@", 1);
		mPort->setValue(5222);
		// check if ssl is enabled and set the port correctly
		sslToggled(cbUseSSL->isChecked());
		mServer->setText(newServer);
		mServer->setEnabled(false);
		mPort->setEnabled(false);
	}
	else
	{
		mServer->setEnabled(true);
		mPort->setEnabled(true);
	}

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

void JabberEditAccountWidget::slotChangePwDone () 
{
	QObject::disconnect( account(), SIGNAL( passwordChangedSuccess() ), this, SLOT( slotChangePwDone() ));
	QObject::disconnect( account(), SIGNAL( passwordChangedError() ), this, SLOT( slotChangePwDone() ));

	pePassword1->erase();
	pePassword2->erase();
	pePassword1->setEnabled( true );
	pePassword2->setEnabled( true );
	btnChangePw->setEnabled( true );	
}

void JabberEditAccountWidget::slotChangePassword () 
{
	QObject::disconnect( account(),SIGNAL( isConnectedChanged () ),  this, SLOT ( slotChangePassword () ));
	QObject::connect( account(), SIGNAL( passwordChangedSuccess() ), this, SLOT( slotChangePwDone() ));
	QObject::connect( account(), SIGNAL( passwordChangedError() ), this, SLOT( slotChangePwDone() ));
	account()->changePassword( pePassword1->password() );
}

void JabberEditAccountWidget::slotChangePwClicked ()
{
	tlChangePassError->hide();
	if (QString::compare( pePassword1->password(), pePassword2->password() ) == 0) 
	{
		pePassword1->setEnabled( false );
		pePassword2->setEnabled( false );
		btnChangePw->setEnabled( false );
		// Not connected, we will try to connect
		if(!( account()->isConnected() )) 
		{
			if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget (),
							i18n("You need to be connected to change your password. Do you want to connect now?") ) == KMessageBox::Yes)
			{
				QObject::connect ( account(), SIGNAL ( isConnectedChanged () ), this, SLOT ( slotChangePassword () ) );
				account()->connect();
			}
			else
			{
				pePassword1->setEnabled( true );
				pePassword2->setEnabled( true );
				btnChangePw->setEnabled( true );
 				return;
			}
		}
		else
		{
			slotChangePassword();
		}
	} 
	else 
	{
		tlChangePassError->show();
		return;
	}
}

void JabberEditAccountWidget::sslToggled (bool value)
{
	if (value && (mPort->value() == 5222))
		mPort->stepUp ();
	else
		if(!value && (mPort->value() == 5223))
			mPort->stepDown ();
}

#include "jabbereditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:
