// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
//
// gaduregisteraccount.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include <qstring.h>
#include <qregexp.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <klineedit.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>

#include "gaduregisteraccountui.h"
#include "gaduregisteraccount.h"
#include "gaducommands.h"

GaduRegisterAccount::GaduRegisterAccount( QWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n( "Register New Account" ), KDialogBase::User1 | KDialogBase::Ok, KDialogBase::User1, true )
{
	ui = new GaduRegisterAccountUI( this );
	setMainWidget( ui );

	ui->valueVerificationSequence->setDisabled( true );
	setButtonText( User1, i18n( "&Register" ) );
	setButtonText( Ok, i18n( "&Cancel" ) );
	enableButton( User1, false );

	cRegister = new RegisterCommand( this );

	emailRegexp = new QRegExp(  "[\\w\\d.+_-]{1,}@[\\w\\d.-]{1,}" );
	hintPixmap = KGlobal::iconLoader()->loadIcon ( "gadu_protocol", KIcon::Small );

	connect( this, SIGNAL( user1Clicked() ), SLOT( doRegister() ) );
	connect( this, SIGNAL( okClicked() ), SLOT( slotClose() ) );

	connect( ui->valueEmailAddress, SIGNAL( textChanged( const QString &) ), SLOT( inputChanged( const QString & ) ) );
	connect( ui->valuePassword, SIGNAL( textChanged( const QString & ) ), SLOT( inputChanged( const QString & ) ) );
	connect( ui->valuePasswordVerify, SIGNAL( textChanged( const QString & ) ), SLOT( inputChanged( const QString & ) ) );
	connect( ui->valueVerificationSequence, SIGNAL( textChanged( const QString & ) ), SLOT( inputChanged( const QString & ) ) );

	connect( cRegister, SIGNAL( tokenRecieved( QPixmap, QString ) ), SLOT( displayToken( QPixmap, QString ) ) );
	connect( cRegister, SIGNAL( done(  const QString&,  const QString& ) ), SLOT( registrationDone(  const QString&,  const QString& ) ) );
	connect( cRegister, SIGNAL( error(  const QString&,  const QString& ) ), SLOT( registrationError(  const QString&,  const QString& ) ) );
	connect( cRegister, SIGNAL( operationStatus( const QString ) ), SLOT( updateStatus( const QString ) ) );

	updateStatus( i18n( "Retrieving token" ) );
	cRegister->requestToken();

	show();
}

void
GaduRegisterAccount::doRegister( )
{
	cRegister->setUserinfo( ui->valueEmailAddress->text(), ui->valuePassword->text(), ui->valueVerificationSequence->text() );
	cRegister->execute();
	enableButton( User1, false );
}

void
GaduRegisterAccount::validateInput()
{
	int valid = true;
	int passwordHighlight = false;

	if ( !emailRegexp->exactMatch( ui->valueEmailAddress->text() ) )
	{
		updateStatus( i18n( "Please enter a valid E-Mail Address." ) );
		ui->pixmapEmailAddress->setPixmap ( hintPixmap );
		valid = false;
	}
	else {
		ui->pixmapEmailAddress->setText ( "" );
	}

	if ( valid && ( ( ui->valuePassword->text().isEmpty() ) || ( ui->valuePasswordVerify->text().isEmpty() ) ) )
	{
		updateStatus( i18n( "Please enter the same password twice." ) );
		valid = false;
		passwordHighlight = true;
	}

	if ( valid && ( ui->valuePassword->text() != ui->valuePasswordVerify->text() ) )
	{
		updateStatus( i18n( "Password entries do not match." ) );
		valid = false;
		passwordHighlight = true;
	}

	if ( valid && ( ui->valueVerificationSequence->text().isEmpty() ) )
	{
		updateStatus( i18n( "Please enter the verification sequence." ) );
		ui->pixmapVerificationSequence->setPixmap ( hintPixmap );
		valid = false;
	}
	else {
		ui->pixmapVerificationSequence->setText ( "" );
	}

	if ( passwordHighlight == true )
	{
		ui->pixmapPassword->setPixmap ( hintPixmap );
		ui->pixmapPasswordVerify->setPixmap ( hintPixmap );
	}
	else {
		ui->pixmapPassword->setText ( "" );
		ui->pixmapPasswordVerify->setText ( "" );
	}

	if ( valid )
	{
		// clear status message if we have valid data
		updateStatus( i18n( "" ) );
	}

	enableButton( User1, valid );
}

void
GaduRegisterAccount::inputChanged( const QString & )
{
	validateInput();
}

void
GaduRegisterAccount::registrationDone(  const QString& /*title*/,  const QString& /*what */ )
{
	ui->valueEmailAddress->setDisabled( true );
	ui->valuePassword->setDisabled( true );
	ui->valuePasswordVerify->setDisabled( true );
	ui->valueVerificationSequence->setDisabled( true );
	ui->labelEmailAddress->setDisabled( true );
	ui->labelPassword->setDisabled( true );
	ui->labelPasswordVerify->setDisabled( true );
	ui->labelVerificationSequence->setDisabled( true );
	ui->labelInstructions->setDisabled( true );
	emit registeredNumber( cRegister->newUin(), ui->valuePassword->text() );
	updateStatus( i18n( "Account created; your new UIN is %1." ).arg(QString::number( cRegister->newUin() )  ) );
	enableButton( User1, false );
	setButtonText( Ok, i18n( "&Close" ) );
}

void
GaduRegisterAccount::registrationError(  const QString& title,  const QString& what )
{
	updateStatus( i18n( "Registration failed: %1" ).arg( what ) );
	KMessageBox::sorry( this, "Registration was unsucessful, please try again.", title );

	disconnect( this, SLOT( displayToken( QPixmap, QString ) ) );
	disconnect( this, SLOT( registrationDone(  const QString&,  const QString& ) ) );
	disconnect( this, SLOT( registrationError(  const QString&,  const QString& ) ) );
	disconnect( this, SLOT( updateStatus( const QString ) ) );

	ui->valueVerificationSequence->setDisabled( true );
	ui->valueVerificationSequence->setText( "" );
	enableButton( User1, false );
	updateStatus( "" );

	// emit UIN 0, to enable 'register new account' button again in dialog below
	emit registeredNumber( 0, QString( "" ) );

	deleteLater();
}

void
GaduRegisterAccount::displayToken( QPixmap image, QString /*tokenId */ )
{
	ui->valueVerificationSequence->setDisabled( false );
	ui->pixmapToken->setPixmap( image );
	validateInput();
}

void
GaduRegisterAccount::updateStatus( const QString status )
{
	ui->labelStatusMessage->setAlignment( AlignCenter );
	ui->labelStatusMessage->setText( status );
}

void
GaduRegisterAccount::slotClose()
{
	deleteLater();
}

GaduRegisterAccount::~GaduRegisterAccount( )
{
	kdDebug( 14100 ) << " register Cancel " << endl;
}

#include "gaduregisteraccount.moc"
