
/***************************************************************************
                   jabberaccountwidget.cpp  -  Account widget for Jabber
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           Based on code by Olivier Goffart <ogoffart@tiscalinet.be>
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

#include <kmessagebox.h>
#include <klocale.h>
#include <kpassdlg.h>

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

	if (account())
	{
		this->reopen ();
		btnRegister->setEnabled ( false );
	}
	else
	{
		connect (btnRegister, SIGNAL (clicked ()), this, SLOT (registerClicked ()));
	}
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
	mResource->setText (account()->pluginData (m_protocol, "Resource"));
	mPriority->setValue (account()->pluginData (m_protocol, "Priority").toInt ());
	mServer->setText (account()->pluginData (m_protocol, "Server"));

	cbUseSSL->setChecked (account()->pluginData (m_protocol, "UseSSL") == QString::fromLatin1("true"));

	mPort->setValue (account()->pluginData (m_protocol, "Port").toInt ());

	QString auth = account()->pluginData (m_protocol, "AuthType");

	cbCustomServer->setChecked (account()->pluginData(m_protocol, "CustomServer") == QString::fromLatin1 ("true"));

	if(cbCustomServer->isChecked ())
	{
		mServer->setEnabled(true);
	}
	else
	{
		mServer->setEnabled (false);
		mServer->setText(mID->text().section("@", 1));
	}

	cbAllowPlainTextPassword->setChecked (account()->pluginData (m_protocol, "AllowPlainTextPassword") == QString::fromLatin1 ("true"));

	KGlobal::config()->setGroup("Jabber");
	leLocalIP->setText (KGlobal::config()->readEntry("LocalIP", ""));
	sbLocalPort->setValue (KGlobal::config()->readNumEntry("LocalPort", 8010));

	leProxyJID->setText (account()->pluginData (m_protocol, "ProxyJID"));

}

KopeteAccount *JabberEditAccountWidget::apply ()
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

	if (cbUseSSL->isChecked ())
		account()->setPluginData (m_protocol, "UseSSL", "true");
	else
		account()->setPluginData (m_protocol, "UseSSL", "false");

	mPass->save(&account()->password ());

	if (cbCustomServer->isChecked ())
	{
		account()->setPluginData (m_protocol, "CustomServer", "true");
	}
	else
	{
		account()->setPluginData (m_protocol, "CustomServer", "false");
	}

	// FIXME: The call below represents a flaw in the current Kopete API.
	// Once the API is cleaned up, this will most likely require a change.
	//account()->setAccountId(mID->text());

	if (cbAllowPlainTextPassword->isChecked ())
		account()->setPluginData (m_protocol, "AllowPlainTextPassword", "true");
	else
		account()->setPluginData (m_protocol, "AllowPlainTextPassword", "false");

	account()->setPluginData (m_protocol, "Server", mServer->text ());
	account()->setPluginData (m_protocol, "Resource", mResource->text ());
	account()->setPluginData (m_protocol, "Priority", QString::number (mPriority->value ()));
	account()->setPluginData (m_protocol, "Port", QString::number (mPort->value ()));

	account()->setAutoLogin(cbAutoConnect->isChecked());

	KGlobal::config()->setGroup("Jabber");
	KGlobal::config()->writeEntry("LocalIP", leLocalIP->text());
	KGlobal::config()->writeEntry("LocalPort", sbLocalPort->value());

	account()->setPluginData (m_protocol, "ProxyJID", leProxyJID->text());

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

		mServer->setText(newServer);
		mServer->setEnabled(false);
	}
	else
	{
		mServer->setEnabled(true);
	}

}

void JabberEditAccountWidget::deleteClicked ()
{

	// delete account here

}

void JabberEditAccountWidget::registerClicked ()
{

	JabberRegisterAccount *registerDlg = new JabberRegisterAccount ( this );
	connect ( registerDlg, SIGNAL ( okClicked () ), this, SLOT ( slotRegisterOkClicked () ) );
	connect ( registerDlg, SIGNAL ( cancelClicked () ), this, SLOT ( slotRegisterCancelClicked () ) );

	registerDlg->show ();

}

void JabberEditAccountWidget::slotRegisterOkClicked ()
{

	//JabberRegisterAccount *registerDlg = static_cast<JabberRegisterAccount*>( sender () );

}

void JabberEditAccountWidget::slotRegisterCancelClicked ()
{

	//JabberRegisterAccount *registerDlg = static_cast<JabberRegisterAccount*>( sender () );

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
