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

#include "gaduregisteraccountui.h"
#include "gaduregisteraccount.h"
#include "gaducommands.h"

GaduRegisterAccount::GaduRegisterAccount( QWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n( "Register New Account" ), KDialogBase::Apply | KDialogBase::Cancel, KDialogBase::Apply, true )
{
	ui = new GaduRegisterAccountUI( this );
	setMainWidget( ui );

	ui->valueToken->setDisabled( true );
	setButtonText( Apply, i18n( "&Register" ) );
	setButtonText( Cancel, i18n( "&Close" ) );
	enableButton( Apply, false );
	updateStatus( "" );

	cRegister = new RegisterCommand( this );

	//emailRegexp = new QRegExp( "[\\w\\d\\.\\-\\+\\_]{1,}\\@[\\w\\d\\.\\-]{1,}" );
	emailRegexp = new QRegExp(  "[\\w\\d.+_-]{1,}@[\\w\\d.-]{1,}" );

	connect( this, SIGNAL( applyClicked() ), SLOT( doRegister() ) );
	connect( this, SIGNAL( cancelClicked() ), SLOT( slotCancel() ) );

	connect( ui->valueEmailAddress, SIGNAL( textChanged( const QString &) ), SLOT( emailChanged( const QString & ) ) );
	connect( ui->valuePassword, SIGNAL( textChanged( const QString & ) ), SLOT( passwordsChanged( const QString & ) ) );
	connect( ui->valuePasswordVerify, SIGNAL( textChanged( const QString & ) ), SLOT( passwordsChanged( const QString & ) ) );
	connect( ui->valueToken, SIGNAL( textChanged( const QString & ) ), SLOT( tokenChanged( const QString & ) ) );

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
	cRegister->setUserinfo( ui->valueEmailAddress->text(), ui->valuePassword->text(), ui->valueToken->text() );
	cRegister->execute();
	enableButton( Apply, false );
}

void
GaduRegisterAccount::validateInput()
{
	if ( emailRegexp->exactMatch( ui->valueEmailAddress->text() ) &&
		ui->valuePassword->text() == ui->valuePasswordVerify->text()  && !ui->valuePassword->text().isEmpty() && !ui->valuePasswordVerify->text().isEmpty() &&
		!ui->valueToken->text().isEmpty() )
	{
		enableButton( Apply, true );
	}
	else{
		enableButton( Apply, false );
	}
}

void
GaduRegisterAccount::tokenChanged( const QString & )
{
	validateInput();
}

void
GaduRegisterAccount::emailChanged( const QString & )
{
	// validate
	if ( emailRegexp->exactMatch( ui->valueEmailAddress->text() ) == FALSE && !ui->valueEmailAddress->text().isEmpty() ) {
		ui->valueEmailAddress->setPaletteBackgroundColor( QColor( 0, 150 , 227 ) );
	}
	else {
		ui->valueEmailAddress->setPaletteBackgroundColor( QColor( 255, 255 , 255 ) );
	}
	validateInput();
}

void
GaduRegisterAccount::passwordsChanged( const QString & )
{
	if ( ui->valuePassword->text() != ui->valuePasswordVerify->text()  && !ui->valuePassword->text().isEmpty() && !ui->valuePasswordVerify->text().isEmpty() ) {
		ui->valuePassword->setPaletteBackgroundColor( QColor( 164, 0 , 0 ) );
		ui->valuePasswordVerify->setPaletteBackgroundColor( QColor( 164, 0 , 0 ) );
	}
	else {
		ui->valuePassword->setPaletteBackgroundColor( QColor( 255, 255 , 255 ) );
		ui->valuePasswordVerify->setPaletteBackgroundColor( QColor( 255, 255 , 255 ) );
	}
	validateInput();
}

void
GaduRegisterAccount::registrationDone(  const QString& /*title*/,  const QString& /*what */ )
{
	ui->valueToken->setDisabled( true );
	ui->valuePassword->setDisabled( true );
	ui->valuePasswordVerify->setDisabled( true );
	ui->valueEmailAddress->setDisabled( true );
	enableButton( Apply, false );
	emit registeredNumber( cRegister->newUin(), ui->valuePassword->text() );
	updateStatus( i18n( "Your UIN is %1" ).arg(QString::number( cRegister->newUin() )  ) );
	enableButton( Apply, true );
}

void
GaduRegisterAccount::registrationError(  const QString& title,  const QString& what )
{
	updateStatus( i18n( "Registration failed: %1" ).arg( what ) );
	KMessageBox::sorry( this, what, title );

	disconnect( this, SLOT( displayToken( QPixmap, QString ) ) );
	disconnect( this, SLOT( registrationDone(  const QString&,  const QString& ) ) );
	disconnect( this, SLOT( registrationError(  const QString&,  const QString& ) ) );
	disconnect( this, SLOT( updateStatus( const QString ) ) );

// it is set to deleteLater, in case of error
	cRegister = NULL;
	ui->valueToken->setDisabled( true );
	ui->valueToken->setText( "" );
	enableButton( Apply, false );
	updateStatus( "" );

	cRegister = new RegisterCommand( this );

	connect( cRegister, SIGNAL( tokenRecieved( QPixmap, QString ) ), SLOT( displayToken( QPixmap, QString ) ) );
	connect( cRegister, SIGNAL( done(  const QString&,  const QString& ) ), SLOT( registrationDone(  const QString&,  const QString& ) ) );
	connect( cRegister, SIGNAL( error(  const QString&,  const QString& ) ), SLOT( registrationError(  const QString&,  const QString& ) ) );
	connect( cRegister, SIGNAL( operationStatus( const QString ) ), SLOT( updateStatus( const QString ) ) );

	updateStatus( i18n( "Retrieving token" ) );
	cRegister->requestToken();
}

void
GaduRegisterAccount::displayToken( QPixmap image, QString /*tokenId */ )
{
	ui->valueToken->setDisabled( false );
	ui->pixmapToken->setPixmap( image );
	updateStatus( i18n( "" ) );
}

void
GaduRegisterAccount::updateStatus( const QString status )
{
	ui->labelStatusMessage->setAlignment( AlignCenter );
	ui->labelStatusMessage->setText( status );
}

void
GaduRegisterAccount::slotCancel()
{
	deleteLater();
}

GaduRegisterAccount::~GaduRegisterAccount( )
{
	kdDebug( 14100 ) << " register Cancel " << endl;
	if ( cRegister ) {
		cRegister->cancel();
	}
}

#include "gaduregisteraccount.moc"
