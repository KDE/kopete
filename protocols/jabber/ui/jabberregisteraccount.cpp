
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

#include <kdebug.h>
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
#include "jabberclient.h"
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
	jabberClient = new JabberClient ();

	connect ( jabberClient, SIGNAL ( csError ( int ) ), this, SLOT ( slotCSError ( int ) ) );
	connect ( jabberClient, SIGNAL ( tlsWarning ( int ) ), this, SLOT ( slotHandleTLSWarning ( int ) ) );
	connect ( jabberClient, SIGNAL ( connected () ), this, SLOT ( slotConnected () ) );
	
	jidRegExp.setPattern ( "[\\w\\d.+_-]{1,}@[\\w\\d.-]{1,}" );
	hintPixmap = KGlobal::iconLoader()->loadIcon ( "jabber_online", KIcon::Small );

	mSuccess = false;

	// get all settings from the main dialog
	mMainWidget->leServer->setText ( parent->mServer->text () );
	mMainWidget->leJID->setText ( parent->mID->text () );
	mMainWidget->lePassword->setText ( parent->mPass->password () );
	//	mMainWidget->lePasswordVerify->setText ( parent->mPass->password () ); //BUG 114631
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
	delete jabberClient;
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

	// cancel any previous attempt
	jabberClient->disconnect ();

	// FIXME: we need to use the old protocol for now
	jabberClient->setUseXMPP09 ( true );

	jabberClient->setUseSSL ( mMainWidget->cbUseSSL->isChecked () );

	// FIXME: check this when using the new protocol
	jabberClient->setOverrideHost ( true, mMainWidget->leServer->text (), mMainWidget->sbPort->value () );

	// start connection, no authentication
	switch ( jabberClient->connect ( XMPP::Jid ( mMainWidget->leJID->text () ), QString::null, false ) )
	{
		case JabberClient::NoTLS:
			// no SSL support, at the connecting stage this means the problem is client-side
			KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,
								i18n ("SSL support could not be initialized for account %1. This is most likely because the QCA TLS plugin is not installed on your system.").
								arg ( mMainWidget->leJID->text () ),
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
		enableButtonOK ( true );

}

void JabberRegisterAccount::slotHandleTLSWarning ( int validityResult )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Handling TLS warning..." << endl;

	if ( JabberAccount::handleTLSWarning ( jabberClient, validityResult ) )
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
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Error in stream signalled, disconnecting." << endl;

	Kopete::Account::DisconnectReason errorClass;

	mMainWidget->lblStatusMessage->setText ( i18n ( "Protocol error." ) );

	// display message to user
	JabberAccount::handleStreamError (error, jabberClient->clientStream()->errorCondition (), jabberClient->clientConnector()->errorCode (), mMainWidget->leServer->text (), errorClass);

	disconnect ();

}

void JabberRegisterAccount::slotConnected ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Launching registration task..." << endl;

	mMainWidget->lblStatusMessage->setText ( i18n ( "Connected successfully, registering new account..." ) );

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
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Information,
								  i18n ("Unable to create account on the server. The Jabber ID is probably already in use."),
								  i18n ("Jabber Account Registration"));

	}

	// FIXME: this is required because Iris crashes if we try
	//        to disconnect here. Hopefully Justin can fix this.
	QTimer::singleShot(0, this, SLOT(disconnect ()));

}

#include "jabberregisteraccount.moc"
