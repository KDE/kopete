
/***************************************************************************
                   jabberaccountwidget.h  -  Account widget for Jabber
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
#include <kmessagebox.h>
#include <klocale.h>


#include "jabbereditaccountwidget.h"

JabberEditAccountWidget::JabberEditAccountWidget (JabberProtocol * proto, JabberAccount * ident, QWidget * parent, const char *name)
						: DlgJabberEditAccountWidget (parent, name), KopeteEditAccountWidget (ident)
{
	m_protocol = proto;

	connect (mID, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPass, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mResource, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mServer, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPort, SIGNAL (valueChanged (int)), this, SLOT (configChanged ()));

	connect (cbAutoConnect, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbUseSSL, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbAllowPlainTextPassword, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbRemPass, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));

	connect (cbProxyType, SIGNAL (activated (int)), this, SLOT (configChanged ()));
	connect (leProxyName, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (spbProxyPort, SIGNAL (valueChanged (int)), this, SLOT (configChanged ()));
	connect (leProxyUrl, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));

	connect (cbProxyAuth, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (leProxyUser, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (leProxyPass, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));

	connect (mID, SIGNAL (textChanged (const QString &)), this, SLOT (setJIDValidation ()));
	connect (mServer, SIGNAL (textChanged (const QString &)), this, SLOT (setJIDValidation ()));

	connect (btnRegister, SIGNAL (clicked ()), this, SLOT (registerClicked ()));
	connect (cbUseSSL, SIGNAL (toggled (bool)), this, SLOT (sslToggled (bool)));

	if (account())
	{
		this->reopen ();
	}
}

JabberEditAccountWidget::~JabberEditAccountWidget ()
{
}


void JabberEditAccountWidget::reopen ()
{

	// FIXME: this is temporary until Kopete supports accound ID changes!
	mID->setDisabled(true);

	mID->setText (account()->accountId ());
	mPass->setText (account()->password ());
	mResource->setText (account()->pluginData (m_protocol, "Resource"));
	mServer->setText (account()->pluginData (m_protocol, "Server"));

	cbUseSSL->setChecked (account()->pluginData (m_protocol, "UseSSL") == QString::fromLatin1("true"));

	mPort->setValue (account()->pluginData (m_protocol, "Port").toInt ());

	cbRemPass->setChecked (account()->pluginData (m_protocol, "RemPass") == QString::fromLatin1("true"));

	QString auth = account()->pluginData (m_protocol, "AuthType");

	cbAllowPlainTextPassword->setChecked (account()->pluginData (m_protocol, "AllowPlainTextPassword") == QString::fromLatin1 ("true"));

	QString proxyType = account()->pluginData (m_protocol, "ProxyType");

	cbProxyType->setCurrentItem (0);
	if (proxyType == QString ("HTTPS"))
		cbProxyType->setCurrentItem (1);
	else if (proxyType == QString ("HTTPPoll"))
		cbProxyType->setCurrentItem (2);
	else if (proxyType == QString ("SOCKS"))
		cbProxyType->setCurrentItem (3);

	leProxyName->setText (account()->pluginData (m_protocol, "ProxyName"));
	spbProxyPort->setValue (account()->pluginData (m_protocol, "ProxyPort").toInt ());
	leProxyUrl->setText (account()->pluginData (m_protocol, "ProxyUrl"));
	cbProxyAuth->setChecked (account()->pluginData (m_protocol, "ProxyAuth") == QString::fromLatin1 ("true"));
	leProxyUser->setText (account()->pluginData (m_protocol, "ProxyUser"));
	leProxyPass->setText (account()->pluginData (m_protocol, "ProxyPass"));
	cbAutoConnect->setChecked (account()->autoLogin());

	revalidateJID = false;

}

KopeteAccount *JabberEditAccountWidget::apply ()
{
	kdDebug (14180) << "JabberEditAccount::apply()" << endl;

	if(revalidateJID)
		validateJID();

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

	return account();
}


void JabberEditAccountWidget::writeConfig ()
{

	// FIXME: The call below represents a flaw in the current Kopete API.
	// Once the API is cleaned up, this will most likely require a change.
	//account()->setAccountId(mID->text());

	account()->setPluginData (m_protocol, "Server", mServer->text ());
	account()->setPluginData (m_protocol, "Resource", mResource->text ());
	account()->setPluginData (m_protocol, "Port", QString::number (mPort->value ()));

	if (cbUseSSL->isChecked ())
		account()->setPluginData (m_protocol, "UseSSL", "true");
	else
		account()->setPluginData (m_protocol, "UseSSL", "false");

	if (cbRemPass->isChecked ())
	{
		account()->setPluginData (m_protocol, "RemPass", "true");
		account()->setPassword (mPass->text ());
	}
	else
	{
		account()->setPluginData (m_protocol, "RemPass", "false");
		account()->setPassword (NULL);
	}

	if (cbAllowPlainTextPassword->isChecked ())
		account()->setPluginData (m_protocol, "AllowPlainTextPassword", "true");
	else
		account()->setPluginData (m_protocol, "AllowPlainTextPassword", "false");

	account()->setAutoLogin(cbAutoConnect->isChecked());

	switch (cbProxyType->currentItem ())
	{
	case 0:
		account()->setPluginData (m_protocol, "ProxyType", "None");
		break;
	case 1:
		account()->setPluginData (m_protocol, "ProxyType", "HTTPS");
		break;
	case 2:
		account()->setPluginData (m_protocol, "ProxyType", "HTTPPoll");
		break;
	case 3:
		account()->setPluginData (m_protocol, "ProxyType", "SOCKS");
		break;
	default:					// this case should never happen, just
		// implemented for safety
		account()->setPluginData (m_protocol, "ProxyType", "None");
		break;
	}

	account()->setPluginData (m_protocol, "ProxyName", leProxyName->text ());
	account()->setPluginData (m_protocol, "ProxyPort", QString::number (spbProxyPort->value ()));
	account()->setPluginData (m_protocol, "ProxyUrl", leProxyUrl->text ());

	if (cbProxyAuth->isChecked ())
		account()->setPluginData (m_protocol, "ProxyAuth", "true");
	else
		account()->setPluginData (m_protocol, "ProxyAuth", "false");

	account()->setPluginData (m_protocol, "ProxyUser", leProxyUser->text ());
	account()->setPluginData (m_protocol, "ProxyPass", leProxyPass->text ());
	//config->sync();
	settings_changed = false;
}

bool JabberEditAccountWidget::validateData ()
{

	if(!mID->text().contains('@'))
	{
		KMessageBox::sorry(this, i18n("The Jabber ID you have chosen is invalid. "
							"Please make sure it is in the form user@jabber.org."),
							i18n("Invalid Jabber ID"));

		return false;
	}

	return true;
}

void JabberEditAccountWidget::validateJID ()
{
	QString server = mID->text().section('@', 1);

	if(mServer->text().isEmpty())
	{
		// the user didn't specify a server, automatically choose one
		mServer->setText(server);
	}
	else
	{
		if(mServer->text() != server)
		{
			// the user has chosen a different server than his JID
			// suggests, display a warning
			int result = KMessageBox::warningYesNo (this, i18n("You have chosen a different Jabber server than your Jabber "
													"ID suggests. Do you want Kopete to change your server setting? Selecting \"Yes\" "
													"will change your Jabber server to \"%1\" as indicated by your Jabber ID. "
													"Selecting \"No\" will keep your current settings.").arg(server),
													i18n("Are you sure about your server name?"));

			if(result == KMessageBox::Yes)
				mServer->setText(server);
		}
	}

}

void JabberEditAccountWidget::configChanged ()
{
	settings_changed = true;
}

void JabberEditAccountWidget::setJIDValidation ()
{

	revalidateJID = true;

	if((account() != 0L) && (account ()->pluginData(m_protocol, "Server") == mServer->text ()))
		revalidateJID = false;

}

void JabberEditAccountWidget::registerClicked ()
{

	if(!validateData())
		return;

	if (!account())
	{
		setAccount(new JabberAccount (m_protocol, mID->text ()));
	}

	this->writeConfig ();

	static_cast < JabberAccount * >(account())->registerUser ();

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
