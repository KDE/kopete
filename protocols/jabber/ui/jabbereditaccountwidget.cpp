
/*
    jabbereditaccountwidget.cpp - Jabber Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart@tiscalinet.be>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>


#include "jabbereditaccountwidget.h"

JabberEditAccountWidget::JabberEditAccountWidget (JabberProtocol * proto, JabberAccount * ident, QWidget * parent, const char *name)
						: DlgJabberEditAccountWidget (parent, name), EditAccountWidget (ident)
{
	m_protocol = proto;

	connect (mID, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPass, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mResource, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mServer, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPort, SIGNAL (valueChanged (int)), this, SLOT (configChanged ()));

	connect (mAutoConnect, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (chkUseSSL, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (chkRemPass, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));

	// Chat TAB
	connect (mLogAll, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));

	connect (cmbAuth, SIGNAL (activated (int)), this, SLOT (configChanged ()));

	connect (cbProxyType, SIGNAL (activated (int)), this, SLOT (configChanged ()));
	connect (leProxyName, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (spbProxyPort, SIGNAL (valueChanged (int)), this, SLOT (configChanged ()));

	connect (cbProxyAuth, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (leProxyUser, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (leProxyPass, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));

	connect (btnRegister, SIGNAL (clicked ()), this, SLOT (registerClicked ()));
	connect (chkUseSSL, SIGNAL (toggled (bool)), this, SLOT (sslToggled (bool)));

	if (m_account)
	{
		this->reopen ();
	}
}

JabberEditAccountWidget::~JabberEditAccountWidget ()
{
}


void JabberEditAccountWidget::reopen ()
{

	mID->setText (m_account->accountId ());
	mPass->setText (m_account->getPassword ());
	mResource->setText (m_account->pluginData (m_protocol, "Resource"));
	mServer->setText (m_account->pluginData (m_protocol, "Server"));

	mID->setReadOnly(true); //the accountId is constant

	// START Order Important =====
	// !!! First set the checkbox, THEN the port
	// if done the other way round: checking the checkbox triggers +/- 1 for the port-value
	// ( see dlgJabberPrefs::sslToggled() )
	if (m_account->pluginData (m_protocol, "UseSSL") == "true")
		chkUseSSL->setChecked (true);

	if (m_account->pluginData (m_protocol, "RemPass") == "true")
		chkRemPass->setChecked (true);

	mPort->setValue (m_account->pluginData (m_protocol, "Port").toInt ());
	// END Order Important =====

	QString auth = m_account->pluginData (m_protocol, "AuthType");

	cmbAuth->setCurrentItem (0);

	if (auth == QString ("plain"))
		cmbAuth->setCurrentItem (1);

	QString proxyType = m_account->pluginData (m_protocol, "ProxyType");

	cbProxyType->setCurrentItem (0);
	if (proxyType == QString ("HTTPS"))
		cbProxyType->setCurrentItem (1);
	else if (proxyType == QString ("SOCKS4"))
		cbProxyType->setCurrentItem (2);
	else if (proxyType == QString ("SOCKS5"))
		cbProxyType->setCurrentItem (3);

	leProxyName->setText (m_account->pluginData (m_protocol, "ProxyName"));
	spbProxyPort->setValue (m_account->pluginData (m_protocol, "ProxyPort").toInt ());
	cbProxyAuth->setChecked (m_account->pluginData (m_protocol, "ProxyAuth") == QString::fromLatin1 ("true"));
	leProxyUser->setText (m_account->pluginData (m_protocol, "ProxyUser"));
	leProxyPass->setText (m_account->pluginData (m_protocol, "ProxyPass"));
	mAutoConnect->setChecked (m_account->autoLogin());

	if (m_account->pluginData (m_protocol, "LogAll") == "true")
		mLogAll->setChecked (true);
}

KopeteAccount *JabberEditAccountWidget::apply ()
{
	kdDebug (14180) << "JabberEditAccount::apply()" << endl;
	if (!m_account)
	{
		m_account = new JabberAccount (m_protocol, mID->text ());
	}
	/*else
	{
		m_account->setAccountId (mID->text ());
	}*/

	this->writeConfig ();
	return m_account;
}


void JabberEditAccountWidget::writeConfig ()
{
	m_account->setPluginData (m_protocol, "Server", mServer->text ());
	m_account->setPluginData (m_protocol, "Resource", mResource->text ());
	m_account->setPluginData (m_protocol, "Port", QString::number (mPort->value ()));

	if (chkUseSSL->isChecked ())
		m_account->setPluginData (m_protocol, "UseSSL", "true");
	else
		m_account->setPluginData (m_protocol, "UseSSL", "false");

	if (chkRemPass->isChecked ())
	{
		m_account->setPluginData (m_protocol, "RemPass", "true");
		m_account->setPassword (mPass->text ());
	}
	else
	{
		m_account->setPluginData (m_protocol, "RemPass", "false");
		m_account->setPassword (NULL);
	}

	m_account->setAutoLogin(mAutoConnect->isChecked());

	if (mLogAll->isChecked ())
		m_account->setPluginData (m_protocol, "LogAll", "true");
	else
		m_account->setPluginData (m_protocol, "LogAll", "false");

	switch (cmbAuth->currentItem ())
	{
	case 0:
		m_account->setPluginData (m_protocol, "AuthType", "digest");
		break;
	case 1:
		m_account->setPluginData (m_protocol, "AuthType", "plain");
		break;
	default:					// this case should never happen, just
		// implemented for safety
		m_account->setPluginData (m_protocol, "AuthType", "digest");
		break;
	}

	switch (cbProxyType->currentItem ())
	{
	case 0:
		m_account->setPluginData (m_protocol, "ProxyType", "None");
		break;
	case 1:
		m_account->setPluginData (m_protocol, "ProxyType", "HTTPS");
		break;
	case 2:
		m_account->setPluginData (m_protocol, "ProxyType", "SOCKS4");
		break;
	case 3:
		m_account->setPluginData (m_protocol, "ProxyType", "SOCKS5");
		break;
	default:					// this case should never happen, just
		// implemented for safety
		m_account->setPluginData (m_protocol, "ProxyType", "None");
		break;
	}

	m_account->setPluginData (m_protocol, "ProxyName", leProxyName->text ());
	m_account->setPluginData (m_protocol, "ProxyPort", QString::number (spbProxyPort->value ()));

	if (cbProxyAuth->isChecked ())
		m_account->setPluginData (m_protocol, "ProxyAuth", "true");
	else
		m_account->setPluginData (m_protocol, "ProxyAuth", "false");

	m_account->setPluginData (m_protocol, "ProxyUser", leProxyUser->text ());
	m_account->setPluginData (m_protocol, "ProxyPass", leProxyPass->text ());
	//config->sync();
	settings_changed = false;
}

bool JabberEditAccountWidget::validateData ()
{
	return true;
}

void JabberEditAccountWidget::configChanged ()
{
	settings_changed = true;
}

void JabberEditAccountWidget::registerClicked ()
{
	if (!m_account)
	{
		m_account = new JabberAccount (m_protocol, mID->text ());
	}
/*	else
	{
		m_account->setAccountId (mID->text ());
	}*/

	this->writeConfig ();
	static_cast < JabberAccount * >(m_account)->registerUser ();
}

void JabberEditAccountWidget::sslToggled (bool value)
{
	if (value)
		mPort->stepUp ();
	else
		mPort->stepDown ();
}


#include "jabbereditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:
