
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
#include <qtimer.h>

#include "kopeteuiglobal.h"

#include "qca.h"
#include "xmpp.h"
#include "xmpp_tasks.h"

#include "jabbereditaccountwidget.h"

JabberEditAccountWidget::JabberEditAccountWidget (JabberProtocol * proto, JabberAccount * ident, QWidget * parent, const char *name)
						: DlgJabberEditAccountWidget (parent, name), KopeteEditAccountWidget (ident)
{

	jabberTLS = 0L;
	jabberTLSHandler = 0L;
	jabberClientConnector = 0L;
	jabberClientStream = 0L;
	jabberClient = 0L;

	m_protocol = proto;

	connect (mID, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPass, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mResource, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPriority, SIGNAL (valueChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mServer, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (mPort, SIGNAL (valueChanged (int)), this, SLOT (configChanged ()));

	connect (cbAutoConnect, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbUseSSL, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbCustomServer, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbAllowPlainTextPassword, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (cbRemPass, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));

	connect (cbProxyType, SIGNAL (activated (int)), this, SLOT (configChanged ()));
	connect (leProxyName, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (spbProxyPort, SIGNAL (valueChanged (int)), this, SLOT (configChanged ()));
	connect (leProxyUrl, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));

	connect (cbProxyAuth, SIGNAL (toggled (bool)), this, SLOT (configChanged ()));
	connect (leProxyUser, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (leProxyPass, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));

	connect (mID, SIGNAL (textChanged (const QString &)), this, SLOT (updateServerField ()));
	connect (cbCustomServer, SIGNAL (toggled (bool)), this, SLOT (updateServerField ()));

	connect (btnRegister, SIGNAL (clicked ()), this, SLOT (registerClicked ()));
	connect (cbUseSSL, SIGNAL (toggled (bool)), this, SLOT (sslToggled (bool)));

	connect (leLocalIP, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));
	connect (sbLocalPort, SIGNAL (valueChanged (int)), this, SLOT (configChanged ()));
	connect (leProxyJID, SIGNAL (textChanged (const QString &)), this, SLOT (configChanged ()));

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
	mPriority->setValue (account()->pluginData (m_protocol, "Priority").toInt ());
	mServer->setText (account()->pluginData (m_protocol, "Server"));

	cbUseSSL->setChecked (account()->pluginData (m_protocol, "UseSSL") == QString::fromLatin1("true"));

	mPort->setValue (account()->pluginData (m_protocol, "Port").toInt ());

	cbRemPass->setChecked (account()->pluginData (m_protocol, "RemPass") == QString::fromLatin1("true"));

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

	KGlobal::config()->setGroup("Jabber");
	leLocalIP->setText (KGlobal::config()->readEntry("LocalIP", ""));
	sbLocalPort->setValue (KGlobal::config()->readNumEntry("LocalPort", 8010));

	leProxyJID->setText (account()->pluginData (m_protocol, "ProxyJID"));

}

KopeteAccount *JabberEditAccountWidget::apply ()
{
	kdDebug (14180) << "JabberEditAccount::apply()" << endl;

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
							"Please make sure it is in the form user@jabber.org."),
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

		if(newServer.isEmpty ())
			newServer = QString::fromLatin1("jabber.org");

		mServer->setText(newServer);
		mServer->setEnabled(false);
	}
	else
	{
		mServer->setEnabled(true);
	}

}

void JabberEditAccountWidget::registerClicked ()
{

	if(!validateData())
		return;

	kdDebug(14131) << k_funcinfo << "Registering a new Jabber account." << endl;

	btnRegister->setEnabled(false);

	pbRegistration->setEnabled(true);

	/*
	 * Setup authentication layer
	 */
	bool trySSL = false;
	if (cbUseSSL->isChecked ())
	{
		bool sslPossible = QCA::isSupported(QCA::CAP_TLS);

		if (!sslPossible)
		{
			KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,
								i18n ("SSL is not supported. This is most likely because the QCA TLS plugin is not installed on your system."), i18n ("SSL Error"));
			return;
		}
		else
		{
			trySSL = true;

			jabberTLS = new QCA::TLS;
			jabberTLSHandler = new XMPP::QCATLSHandler(jabberTLS);

			{
				using namespace XMPP;
				QObject::connect(jabberTLSHandler, SIGNAL(tlsHandshaken()), this, SLOT(slotTLSHandshaken()));
			}
		}
	}

	/*
	 * Setup proxy layer
	 */
	int proxyType = XMPP::AdvancedConnector::Proxy::None;
	switch(cbProxyType->currentItem ())
	{
		case 1:
			proxyType = XMPP::AdvancedConnector::Proxy::HttpConnect;
			break;

		case 2:
			proxyType = XMPP::AdvancedConnector::Proxy::HttpPoll;
			break;

		case 3:
			proxyType = XMPP::AdvancedConnector::Proxy::Socks;
			break;
	}

	XMPP::AdvancedConnector::Proxy proxy;

	switch(proxyType)
	{
		case XMPP::AdvancedConnector::Proxy::None:
			// no proxy
			break;

		case XMPP::AdvancedConnector::Proxy::HttpConnect:
			// use HTTP
			proxy.setHttpConnect( leProxyName->text (), spbProxyPort->value () );
			break;

		case XMPP::AdvancedConnector::Proxy::HttpPoll:
			// use HTTP polling
			proxy.setHttpPoll ( leProxyName->text (), spbProxyPort->value (), leProxyUrl->text () );
			proxy.setPollInterval (2);
			break;

		case XMPP::AdvancedConnector::Proxy::Socks:
			// use socks
			proxy.setSocks( leProxyName->text (), spbProxyPort->value () );
			break;
	}

	if (cbProxyAuth->isChecked ())
		proxy.setUserPass (leProxyUser->text (), leProxyPass->text ());

	/*
	 * Instantiate connector, responsible for dealing with the socket.
	 * This class makes use of the proxy layer created above.
	 */
	jabberClientConnector = new XMPP::AdvancedConnector;
	jabberClientConnector->setOptHostPort (mServer->text (), mPort->value ());
	jabberClientConnector->setProxy(proxy);
	jabberClientConnector->setOptSSL(trySSL);

	/*
	 * Instantiate client stream which handles the network communication by referring
	 * to a connector (proxying etc.) and a TLS handler (security layer)
	 */
	jabberClientStream = new XMPP::ClientStream(jabberClientConnector, jabberTLSHandler);

	{
		using namespace XMPP;
		QObject::connect (jabberClientStream, SIGNAL (authenticated()),
				  this, SLOT (slotCSAuthenticated ()));
		QObject::connect (jabberClientStream, SIGNAL (warning (int)),
				  this, SLOT (slotCSWarning ()));
		QObject::connect (jabberClientStream, SIGNAL (error (int)),
				  this, SLOT (slotCSError (int)));
	}

	/*
	 * FIXME: This is required until we fully support XMPP 1.0
	 *        Upon switching to XMPP 1.0, add full TLS capabilities
	 *        with fallback (setOptProbe()) and remove the call below.
	 */
	jabberClientStream->setOldOnly(true);

	/*
	 * Initiate anti-idle timer (will be triggered every 55 seconds).
	 */
	jabberClientStream->setNoopTime(55000);

	jabberClient = new XMPP::Client (this);

	pbRegistration->setProgress(25);

	/*
	 * Start connection, no authentication
	 */
	jabberClient->connectToServer (jabberClientStream, XMPP::Jid(mID->text ()), false);


}

void JabberEditAccountWidget::cleanup ()
{

	delete jabberClient;
	delete jabberClientStream;
	delete jabberClientConnector;
	delete jabberTLSHandler;
	delete jabberTLS;

	jabberTLS = 0L;
	jabberTLSHandler = 0L;
	jabberClientConnector = 0L;
	jabberClientStream = 0L;
	jabberClient = 0L;

}

void JabberEditAccountWidget::disconnect ()
{

	if(jabberClient)
		jabberClient->close(true);

	cleanup ();

}

void JabberEditAccountWidget::slotTLSHandshaken ()
{
	kdDebug(14131) << k_funcinfo << "TLS handshake done, testing certificate validity..." << endl;

	pbRegistration->setProgress(50);

	int validityResult = jabberTLS->certificateValidityResult ();

	if(validityResult == QCA::TLS::Valid)
	{
		kdDebug(14131) << k_funcinfo << "Certificate is valid, continuing." << endl;

		// valid certificate, continue
		jabberTLSHandler->continueAfterHandshake ();
	}
	else
	{
		kdDebug(14131) << k_funcinfo << "Certificate is not valid, asking user what to do next." << endl;

		// certificate is not valid, query the user
		if(JabberAccount::handleTLSWarning (validityResult, mServer->text ()) == KMessageBox::Continue)
		{
			jabberTLSHandler->continueAfterHandshake ();
		}
		else
		{
			disconnect ();
		}
	}

}

void JabberEditAccountWidget::slotCSWarning ()
{

	// FIXME: with the next synch point of Iris, this should
	//        have become a slot, so simply connect it through.
	jabberClientStream->continueAfterWarning ();

}

void JabberEditAccountWidget::slotCSError (int error)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Error in stream signalled, disconnecting." << endl;

	// display message to user
	JabberAccount::handleStreamError (error, jabberClientStream->errorCondition (), jabberClientConnector->errorCode (), mServer->text ());

	disconnect ();

}

void JabberEditAccountWidget::slotCSAuthenticated ()
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Launching registration task..." << endl;

	pbRegistration->setProgress(75);

	/* slow down the polling interval for HTTP Poll proxies */
	jabberClientConnector->changePollInterval (10);

	/* start the client operation */
	XMPP::Jid jid(mID->text ());
	jabberClient->start ( jid.domain (), jid.node (), "", "" );

	XMPP::JT_Register * task = new XMPP::JT_Register (jabberClient->rootTask ());
	QObject::connect (task, SIGNAL (finished ()), this, SLOT (slotRegisterUserDone ()));
	task->reg (mID->text().section("@", 0, 0), mPass->text ());
	task->go (true);

}

void JabberEditAccountWidget::slotRegisterUserDone ()
{
	XMPP::JT_Register * task = (XMPP::JT_Register *) sender ();

	pbRegistration->setProgress(100);

	if (task->success ())
		KMessageBox::information (Kopete::UI::Global::mainWidget (), i18n ("Account successfully registered."), i18n ("Account Registration"));
	else
	{
		KMessageBox::information (Kopete::UI::Global::mainWidget (), i18n ("Unable to create account on the server."), i18n ("Account Registration"));

	}

	// FIXME: this is required because Iris crashes if we try
	//        to disconnect here. Hopefully Justin can fix this.
	QTimer::singleShot(0, this, SLOT(disconnect ()));

	pbRegistration->setEnabled(false);
	pbRegistration->setProgress(0);

	btnRegister->setEnabled(true);

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
