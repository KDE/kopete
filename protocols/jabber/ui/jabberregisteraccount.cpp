
/***************************************************************************
                   jabberregister.cpp  -  Register dialog for Jabber
                             -------------------
    begin                : Sun Jul 11 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

		Kopete (C) 2001-2004 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabberregisteraccount.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <kpassdlg.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qregexp.h>

#include "qca.h"
#include "xmpp.h"
#include "xmpp_tasks.h"

#include "kopeteuiglobal.h"
#include "kopetepasswordwidget.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberconnector.h"
#include "jabbereditaccountwidget.h"
#include "jabberchooseserver.h"
#include "dlgjabberregisteraccount.h"

JabberRegisterAccount::JabberRegisterAccount ( JabberEditAccountWidget *parent, const char *name )
 : KDialogBase ( parent, name, true, i18n("Register New Jabber Account"),
 				 KDialogBase::Ok | KDialogBase::Cancel )
{

	mParentWidget = parent;

	// setup main dialog
	mMainWidget = new DlgJabberRegisterAccount ( this );
	setMainWidget ( mMainWidget );

	// replace "Ok" button with a "Register" button
	KGuiItem registerButton = KStdGuiItem::ok();
	registerButton.setText ( i18n ( "Register" ) );
	setButtonOK ( registerButton );

	enableButtonSeparator ( true );

	// clear variables
	jabberTLS = 0L;
	jabberTLSHandler = 0L;
	jabberClientConnector = 0L;
	jabberClientStream = 0L;
	jabberClient = 0L;

	jidRegExp.setPattern ( "[\\w\\d.+_-]{1,}@[\\w\\d.-]{1,}" );
	hintPixmap = KGlobal::iconLoader()->loadIcon ( "jabber_online", KIcon::Small );

	mSuccess = false;

	// get all settings from the main dialog
	mMainWidget->leServer->setText ( parent->mServer->text () );
	mMainWidget->leJID->setText ( parent->mID->text () );
	mMainWidget->lePassword->setText ( parent->mPass->password () );
	mMainWidget->lePasswordVerify->setText ( parent->mPass->password () );
	mMainWidget->sbPort->setValue ( parent->mPort->value () );
	mMainWidget->cbUseSSL->setChecked ( parent->cbUseSSL->isChecked () );

	// connect buttons to slots, ok is already connected by default
	connect ( this, SIGNAL ( cancelClicked () ), this, SLOT ( slotDeleteDialog () ) );
	connect ( mMainWidget->btnChooseServer, SIGNAL ( clicked () ), this, SLOT ( slotChooseServer () ) );
	connect ( mMainWidget->leServer, SIGNAL ( textChanged ( const QString & ) ), this, SLOT ( slotJIDInformation () ) );
	connect ( mMainWidget->leJID, SIGNAL ( textChanged ( const QString & ) ), this, SLOT ( slotJIDInformation () ) );
	connect ( mMainWidget->cbUseSSL, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotSSLToggled () ) );

	connect ( mMainWidget->leServer, SIGNAL ( textChanged ( const QString & ) ), this, SLOT ( validateData () ) );
	connect ( mMainWidget->leJID, SIGNAL ( textChanged ( const QString & ) ), this, SLOT ( validateData () ) );
	connect ( mMainWidget->lePassword, SIGNAL ( textChanged ( const QString & ) ), this, SLOT ( validateData () ) );
	connect ( mMainWidget->lePasswordVerify, SIGNAL ( textChanged ( const QString & ) ), this, SLOT ( validateData () ) );

	// display JID info now
	slotJIDInformation ();

	// display validation info
	validateData ();
}


JabberRegisterAccount::~JabberRegisterAccount()
{
}

void JabberRegisterAccount::slotDeleteDialog ()
{

	deleteLater ();

}

void JabberRegisterAccount::validateData ()
{

	int valid = true;
	int passwordHighlight = false;

	if ( mMainWidget->leServer->text().isEmpty () )
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Please enter a server name, or click Choose." ) );
		mMainWidget->pixServer->setPixmap ( hintPixmap );
		valid = false;
	}
	else
	{
		mMainWidget->pixServer->setText ( "" );
	}

	if ( valid && !jidRegExp.exactMatch ( mMainWidget->leJID->text() ) )
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Please enter a valid Jabber ID." ) );
		mMainWidget->pixJID->setPixmap ( hintPixmap );
		valid = false;
	}
	else
	{
		mMainWidget->pixJID->setText ( "" );
	}

	if ( valid &&
	   ( QString::fromLatin1 ( mMainWidget->lePassword->password () ).isEmpty () ||
	     QString::fromLatin1 ( mMainWidget->lePasswordVerify->password () ).isEmpty () ) )
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Please enter the same password twice." ) );
		valid = false;
		passwordHighlight = true;
	}

	if ( valid &&
	   ( QString::fromLatin1 ( mMainWidget->lePassword->password () ) !=
	     QString::fromLatin1 ( mMainWidget->lePasswordVerify->password () ) ) )
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Password entries do not match." ) );
		valid = false;
		passwordHighlight = true;
	}

	if ( passwordHighlight == true )
	{
		mMainWidget->pixPassword->setPixmap ( hintPixmap );
		mMainWidget->pixPasswordVerify->setPixmap ( hintPixmap );
	}
	else {
		mMainWidget->pixPassword->setText ( "" );
		mMainWidget->pixPasswordVerify->setText ( "" );
	}

	if ( valid )
	{
		// clear status message if we have valid data
		mMainWidget->lblStatusMessage->setText ( "" );
	}

	enableButtonOK ( valid );

}

void JabberRegisterAccount::slotJIDInformation ()
{

	if ( !mMainWidget->leServer->text().isEmpty () &&
		 ( !jidRegExp.exactMatch ( mMainWidget->leJID->text () ) ||
		 ( mMainWidget->leJID->text().section ( "@", 1 ) != mMainWidget->leServer->text () ) ) )
	{
		mMainWidget->lblJIDInformation->setText ( i18n ( "Unless you know what you are doing, your JID should be of the form "
														 "\"username@server.com\".  In your case for example \"username@%1\"." ).
													arg ( mMainWidget->leServer->text () ) );
	}
	else
	{
		mMainWidget->lblJIDInformation->setText ( "" );
	}

}

void JabberRegisterAccount::slotSSLToggled ()
{

	if ( mMainWidget->cbUseSSL->isChecked () )
	{
		if ( mMainWidget->sbPort->value () == 5222 )
		{
			mMainWidget->sbPort->setValue ( 5223 );
		}
	}
	else
	{
		if ( mMainWidget->sbPort->value () == 5223 )
		{
			mMainWidget->sbPort->setValue ( 5222 );
		}
	}

}

void JabberRegisterAccount::slotChooseServer ()
{

	(new JabberChooseServer ( this ))->show ();

}

void JabberRegisterAccount::setServer ( const QString &server )
{

	mMainWidget->leServer->setText ( server );
	mMainWidget->leJID->setText ( QString ( "@%1" ).arg ( server ) );

}

void JabberRegisterAccount::slotOk ()
{

	mMainWidget->lblStatusMessage->setText ( "" );

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Registering a new Jabber account." << endl;

	enableButtonOK ( false );

	mMainWidget->lblStatusMessage->setText ( i18n ( "Connecting to server..." ) );

	/*
	 * Check for SSL availability first
	 */
	bool trySSL = false;
	if ( mMainWidget->cbUseSSL->isChecked () )
	{
		bool sslPossible = QCA::isSupported(QCA::CAP_TLS);

		if (!sslPossible)
		{
			KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,
								i18n ("SSL support could not be initialized for account %1. This is most likely because the QCA TLS plugin is not installed on your system.").
								arg(mMainWidget->leJID->text()),
								i18n ("Jabber SSL Error"));
			return;
		}
		else
		{
			trySSL = true;
		}
	}

	/*
	 * Instantiate connector, responsible for dealing with the socket.
	 * This class uses KDE's socket code, which in turn makes use of
	 * the global proxy settings.
	 */
	jabberClientConnector = new JabberConnector;
	jabberClientConnector->setOptHostPort ( mMainWidget->leServer->text (), mMainWidget->sbPort->value () );
	jabberClientConnector->setOptSSL(trySSL);

	/*
	 * Setup authentication layer
	 */
	if ( trySSL )
	{
		jabberTLS = new QCA::TLS;
		jabberTLSHandler = new XMPP::QCATLSHandler(jabberTLS);

		{
			using namespace XMPP;
			QObject::connect(jabberTLSHandler, SIGNAL(tlsHandshaken()), this, SLOT(slotTLSHandshaken()));
		}
	}

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

	/*
	 * Start connection, no authentication
	 */
	jabberClient->connectToServer (jabberClientStream, XMPP::Jid(mMainWidget->leJID->text ()), false);


}

void JabberRegisterAccount::cleanup ()
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

void JabberRegisterAccount::disconnect ()
{

	if(jabberClient)
		jabberClient->close(true);

	cleanup ();

	if ( !mSuccess )
		enableButtonOK ( true );

}

void JabberRegisterAccount::slotTLSHandshaken ()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "TLS handshake done, testing certificate validity..." << endl;

	mMainWidget->lblStatusMessage->setText ( i18n ( "Security handshake..." ) );

	int validityResult = jabberTLS->certificateValidityResult ();

	if(validityResult == QCA::TLS::Valid)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Certificate is valid, continuing." << endl;

		// valid certificate, continue
		jabberTLSHandler->continueAfterHandshake ();
	}
	else
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Certificate is not valid, asking user what to do next." << endl;

		// certificate is not valid, query the user
		if(JabberAccount::handleTLSWarning (validityResult, mMainWidget->leServer->text (), mMainWidget->leJID->text ()) == KMessageBox::Continue)
		{
			jabberTLSHandler->continueAfterHandshake ();
		}
		else
		{
			mMainWidget->lblStatusMessage->setText ( i18n ( "Security handshake failed." ) );
			disconnect ();
		}
	}

}

void JabberRegisterAccount::slotCSWarning ()
{

	// FIXME: with the next synch point of Iris, this should
	//        have become a slot, so simply connect it through.
	jabberClientStream->continueAfterWarning ();

}

void JabberRegisterAccount::slotCSError (int error)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Error in stream signalled, disconnecting." << endl;

	KopeteAccount::DisconnectReason errorClass;

	mMainWidget->lblStatusMessage->setText ( i18n ( "Protocol error." ) );

	// display message to user
	JabberAccount::handleStreamError (error, jabberClientStream->errorCondition (), jabberClientConnector->errorCode (), mMainWidget->leServer->text (), errorClass);

	disconnect ();

}

void JabberRegisterAccount::slotCSAuthenticated ()
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Launching registration task..." << endl;

	mMainWidget->lblStatusMessage->setText ( i18n ( "Authentication successful, registering new account..." ) );

	/* start the client operation */
	XMPP::Jid jid(mMainWidget->leJID->text ());
	jabberClient->start ( jid.domain (), jid.node (), "", "" );

	XMPP::JT_Register * task = new XMPP::JT_Register (jabberClient->rootTask ());
	QObject::connect (task, SIGNAL (finished ()), this, SLOT (slotRegisterUserDone ()));
	task->reg (mMainWidget->leJID->text().section("@", 0, 0), mMainWidget->lePassword->password ());
	task->go (true);

}

void JabberRegisterAccount::slotRegisterUserDone ()
{
	XMPP::JT_Register * task = (XMPP::JT_Register *) sender ();

	if (task->success ())
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Registration successful." ) );

		// save settings to parent
		mParentWidget->mServer->setText ( mMainWidget->leServer->text () );
		mParentWidget->mID->setText ( mMainWidget->leJID->text () );
		mParentWidget->mPass->setPassword ( mMainWidget->lePassword->password () );
		mParentWidget->mPort->setValue ( mMainWidget->sbPort->value () );
		mParentWidget->cbUseSSL->setChecked ( mMainWidget->cbUseSSL->isChecked () );

		// disable input widgets
		mMainWidget->btnChooseServer->setEnabled ( false );
		mMainWidget->leServer->setEnabled ( false );
		mMainWidget->leJID->setEnabled ( false );
		mMainWidget->lePassword->setEnabled ( false );
		mMainWidget->lePasswordVerify->setEnabled ( false );
		mMainWidget->sbPort->setEnabled ( false );
		mMainWidget->cbUseSSL->setEnabled ( false );

		// disable input widget labels
		mMainWidget->lblServer->setEnabled ( false );
		mMainWidget->lblJID->setEnabled ( false );
		mMainWidget->lblPassword->setEnabled ( false );
		mMainWidget->lblPasswordVerify->setEnabled ( false );
		mMainWidget->lblPort->setEnabled ( false );

		mSuccess = true;

		// rewire buttons
		enableButtonOK ( false );
		setButtonCancel ( KStdGuiItem::close () );
		connect ( this, SIGNAL ( closeClicked () ), this, SLOT ( slotDeleteDialog () ) );
	}
	else
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Registration failed." ) );
		KMessageBox::information (Kopete::UI::Global::mainWidget (),
								  i18n ("Unable to create account on the server. The Jabber ID is probably already in use."),
								  i18n ("Jabber Account Registration"));

	}

	// FIXME: this is required because Iris crashes if we try
	//        to disconnect here. Hopefully Justin can fix this.
	QTimer::singleShot(0, this, SLOT(disconnect ()));

}

#include "jabberregisteraccount.moc"
