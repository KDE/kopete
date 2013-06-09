
/***************************************************************************
                   jabberregister.cpp  -  Register dialog for Jabber
                             -------------------
    begin                : Sun Jul 11 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

    Copyright 2006 by Tommi Rantala <tommi.rantala@cs.helsinki.fi>

		Kopete (C) 2001-2006 Kopete developers <kopete-devel@kde.org>
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

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qregexp.h>

#include <QtCrypto>
#include "xmpp.h"
#include "xmpp_tasks.h"

#include "kopeteuiglobal.h"
#include "kopetepasswordwidget.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberclient.h"
#include "jabbereditaccountwidget.h"
#include "jabberchooseserver.h"
#include "ui_dlgjabberregisteraccount.h"

JabberRegisterAccount::JabberRegisterAccount ( JabberEditAccountWidget *parent )
 : KDialog ( parent )
{

	setCaption( i18n("Register New Jabber Account") );
	setButtons( KDialog::Ok | KDialog::Cancel );
	mParentWidget = parent;

	// setup main dialog
	QWidget* w = new QWidget( this );
	mMainWidget = new Ui::DlgJabberRegisterAccount;
	mMainWidget->setupUi( w );
	setMainWidget ( w );

	// replace "Ok" button with a "Register" button
	KGuiItem registerButton = KStandardGuiItem::ok();
	registerButton.setText ( i18n ( "Register" ) );
	setButtonGuiItem( KDialog::Ok, KGuiItem( registerButton ) );

	showButtonSeparator ( true );

	// clear variables
	jabberClient = new JabberClient ();

	connect ( jabberClient, SIGNAL (csError(int)), this, SLOT (slotCSError(int)) );
	connect ( jabberClient, SIGNAL (tlsWarning(QCA::TLS::IdentityResult,QCA::Validity)), this, SLOT (slotHandleTLSWarning(QCA::TLS::IdentityResult,QCA::Validity)) );
	connect ( jabberClient, SIGNAL (connected()), this, SLOT (slotConnected()) );
	connect ( jabberClient, SIGNAL (debugMessage(QString)), this, SLOT (slotDebugMessage(QString)) );

	jidRegExp.setPattern ( "[\\w\\d.+_-]{1,}@[\\w\\d.-]{1,}" );
	hintPixmap = SmallIcon ( "jabber_online" );

	mSuccess = false;

	// get all settings from the main dialog
	mMainWidget->leJID->setText ( parent->mID->text () );
	mMainWidget->lePassword->setText ( parent->mPass->password () );
    mMainWidget->lePassword->setPasswordMode ( true );
	//	mMainWidget->lePasswordVerify->setText ( parent->mPass->password () ); //BUG 114631
    mMainWidget->lePasswordVerify->setPasswordMode ( true );
	mMainWidget->cbUseSSL->setChecked ( parent->cbUseSSL->isChecked () );
	mMainWidget->cbOverrideHost->setChecked ( parent->cbCustomServer->isChecked () );
	mMainWidget->leServer->setText ( parent->mServer->text () );
	mMainWidget->sbPort->setValue ( parent->mPort->value () );

	slotOverrideHostToggled();

	connect ( this, SIGNAL (okClicked()), this, SLOT(slotOk()) );
	connect ( this, SIGNAL (cancelClicked()), this, SLOT (slotDeleteDialog()) );
	connect ( mMainWidget->btnChooseServer, SIGNAL (clicked()), this, SLOT (slotChooseServer()) );
	connect ( mMainWidget->leServer, SIGNAL (textChanged(QString)), this, SLOT (slotJIDInformation()) );
	connect ( mMainWidget->leJID, SIGNAL (textChanged(QString)), this, SLOT (slotJIDInformation()) );
	connect ( mMainWidget->cbUseSSL, SIGNAL (toggled(bool)), this, SLOT (slotSSLToggled()) );
	connect ( mMainWidget->cbOverrideHost, SIGNAL (toggled(bool)), this, SLOT (slotOverrideHostToggled()) );

	connect ( mMainWidget->leServer, SIGNAL (textChanged(QString)), this, SLOT (validateData()) );
	connect ( mMainWidget->leJID, SIGNAL (textChanged(QString)), this, SLOT (validateData()) );
	connect ( mMainWidget->lePassword, SIGNAL (textChanged(QString)), this, SLOT (validateData()) );
	connect ( mMainWidget->lePasswordVerify, SIGNAL (textChanged(QString)), this, SLOT (validateData()) );

	// display JID info now
	slotJIDInformation ();

	// display validation info
	validateData ();
}


JabberRegisterAccount::~JabberRegisterAccount()
{
	delete mMainWidget;
	delete jabberClient;
}

void JabberRegisterAccount::accept ()
{

}

void JabberRegisterAccount::slotDeleteDialog ()
{

	deleteLater ();

}

void JabberRegisterAccount::validateData ()
{

	bool valid = true;
	bool passwordHighlight = false;

	if ( mMainWidget->cbOverrideHost->isChecked () && mMainWidget->leServer->text().isEmpty () )
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Please enter a server name." ) );
		valid = false;
	}

	if ( valid && !jidRegExp.exactMatch ( mMainWidget->leJID->text() ) )
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Please enter a valid Jabber ID or click Choose." ) );
		mMainWidget->pixJID->setPixmap ( hintPixmap );
		valid = false;
	}
	else
	{
		mMainWidget->pixJID->setText ( "" );
	}

	if ( valid &&
	   (  mMainWidget->lePassword->text().isEmpty () ||
	      mMainWidget->lePasswordVerify->text ().isEmpty () ) )
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Please enter the same password twice." ) );
		valid = false;
		passwordHighlight = true;
	}

	if ( valid &&  mMainWidget->lePassword->text () != mMainWidget->lePasswordVerify->text () )
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

	enableButtonOk ( valid );

}

void JabberRegisterAccount::slotJIDInformation ()
{

	if ( !mMainWidget->leServer->text().isEmpty () &&
		 ( !jidRegExp.exactMatch ( mMainWidget->leJID->text () ) ||
		 ( mMainWidget->cbOverrideHost->isChecked () && mMainWidget->leJID->text().section ( '@', 1 ) != mMainWidget->leServer->text () ) ) )
	{
		mMainWidget->lblJIDInformation->setText ( i18n ( "Unless you know what you are doing, your JID should be of the form "
														 "\"username@server.com\".  In your case for example \"username@%1\"." , 
													  mMainWidget->leServer->text () ) );
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

void JabberRegisterAccount::slotOverrideHostToggled()
{

	bool overrideHost = mMainWidget->cbOverrideHost->isChecked ();
	mMainWidget->leServer->setEnabled ( overrideHost );
	mMainWidget->sbPort->setEnabled ( overrideHost );
	mMainWidget->lblServer->setEnabled ( overrideHost );
	mMainWidget->lblPort->setEnabled ( overrideHost );
	slotJIDInformation();
	validateData();

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

	kDebug ( JABBER_DEBUG_GLOBAL ) << "Registering a new Jabber account.";

	enableButtonOk ( false );

	mMainWidget->lblStatusMessage->setText ( i18n ( "Connecting to server..." ) );

	// cancel any previous attempt
	jabberClient->disconnect ();

	jabberClient->setUseSSL ( mMainWidget->cbUseSSL->isChecked () );

	if ( mMainWidget->cbOverrideHost->isChecked () )
	{
		jabberClient->setUseXMPP09 ( true );
		jabberClient->setOverrideHost ( true, mMainWidget->leServer->text (), mMainWidget->sbPort->value () );
	}
	else
	{
		jabberClient->setUseXMPP09 ( false );
		jabberClient->setOverrideHost ( false );
	}

	// start connection, no authentication
	switch ( jabberClient->connect ( XMPP::Jid ( mMainWidget->leJID->text () ), QString(), false ) )
	{
		case JabberClient::NoTLS:
			// no SSL support, at the connecting stage this means the problem is client-side
			KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,
								i18n ("SSL support could not be initialized for account %1. This is most likely because the QCA TLS plugin is not installed on your system.", 
								  mMainWidget->leJID->text () ),
								i18n ("Jabber SSL Error"));
			break;
	
		case JabberClient::Ok:
		default:
			// everything alright!
			break;
	}

}

void JabberRegisterAccount::disconnect ()
{

	if(jabberClient)
		jabberClient->disconnect ();

	if ( !mSuccess )
		enableButtonOk ( true );

}

void JabberRegisterAccount::slotHandleTLSWarning (
		QCA::TLS::IdentityResult identityResult,
		QCA::Validity validityResult )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Handling TLS warning...";

	if ( JabberAccount::handleTLSWarning ( jabberClient, identityResult, validityResult ) )
	{
		// resume stream
		jabberClient->continueAfterTLSWarning ();
	}
	else
	{
		// disconnect stream
		disconnect ();
	}

}

void JabberRegisterAccount::slotCSError (int error)
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Error in stream signalled, disconnecting.";

	Kopete::Account::DisconnectReason errorClass;

	mMainWidget->lblStatusMessage->setText ( i18n ( "Protocol error." ) );

	// display message to user
	JabberAccount::handleStreamError (error, jabberClient->clientStream()->errorCondition (), jabberClient->clientConnector()->errorCode (), mMainWidget->leServer->text (), errorClass, jabberClient->clientStream()->errorText());

	disconnect ();

}

void JabberRegisterAccount::slotConnected ()
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Launching registration task...";

	mMainWidget->lblStatusMessage->setText ( i18n ( "Connected successfully, registering new account..." ) );

	XMPP::JT_Register * task = new XMPP::JT_Register (jabberClient->rootTask ());
	QObject::connect (task, SIGNAL (finished()), this, SLOT (slotRegisterUserDone()));
	task->reg (mMainWidget->leJID->text().section('@', 0, 0), mMainWidget->lePassword->text ());
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
		mParentWidget->mPass->setPassword ( mMainWidget->lePassword->text () );
		mParentWidget->mPort->setValue ( mMainWidget->sbPort->value () );
		mParentWidget->cbUseSSL->setChecked ( mMainWidget->cbUseSSL->isChecked () );
		mParentWidget->cbCustomServer->setChecked ( mMainWidget->cbOverrideHost->isChecked () );

		// disable input widgets
		mMainWidget->btnChooseServer->setEnabled ( false );
		mMainWidget->leServer->setEnabled ( false );
		mMainWidget->leJID->setEnabled ( false );
		mMainWidget->lePassword->setEnabled ( false );
		mMainWidget->lePasswordVerify->setEnabled ( false );
		mMainWidget->sbPort->setEnabled ( false );
		mMainWidget->cbUseSSL->setEnabled ( false );
		mMainWidget->cbOverrideHost->setEnabled ( false );

		// disable input widget labels
		mMainWidget->lblServer->setEnabled ( false );
		mMainWidget->lblJID->setEnabled ( false );
		mMainWidget->lblPassword->setEnabled ( false );
		mMainWidget->lblPasswordVerify->setEnabled ( false );
		mMainWidget->lblPort->setEnabled ( false );

		mSuccess = true;

		// rewire buttons
		enableButtonOk ( false );
		setButtonGuiItem( KDialog::Cancel, KStandardGuiItem::close () );
		connect ( this, SIGNAL (closeClicked()), this, SLOT (slotDeleteDialog()) );
	}
	else
	{
		mMainWidget->lblStatusMessage->setText ( i18n ( "Registration failed." ) );
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Information,
								  i18n ("Unable to create an account on the server. The Jabber ID is probably already in use."),
								  i18n ("Jabber Account Registration"));

	}

	// FIXME: this is required because Iris crashes if we try
	//        to disconnect here. Hopefully Justin can fix this.
	QTimer::singleShot(0, this, SLOT(disconnect()));

}

void JabberRegisterAccount::slotDebugMessage ( const QString &msg )
{

	kDebug (JABBER_DEBUG_PROTOCOL) << msg;

}

#include "jabberregisteraccount.moc"
