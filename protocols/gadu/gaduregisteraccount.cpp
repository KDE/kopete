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

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <klineedit.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpixmap.h>

#include <kmessagebox.h>
#include <kdebug.h>

#include "gaduregisteraccount.h"

GaduRegisterAccount::GaduRegisterAccount( QWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n( "Register New Account" ), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true )
{
	ui = new GaduRegisterAccountUI( this );
	setMainWidget( ui );

	ui->textToken->setDisabled( true );
	enableButton( Ok, false );
	updateStatus( "" );

	cRegister = new RegisterCommand( this );

	emailRegexp = new QRegExp( "[\\w\\d\\.\\-\\+\\_]{1,}\\@[\\w\\d\\.\\-]{1,}" );

	connect( this, SIGNAL( applyClicked() ), SLOT( slotApply() ) );
	connect( this, SIGNAL( cancelClicked() ), SLOT( slotCancel() ) );

	connect( ui->submitData, SIGNAL( clicked() ), SLOT( doRegister() ) );
	connect( ui->emailArea, SIGNAL( textChanged( const QString &) ), SLOT( emailChanged( const QString & ) ) );
	connect( ui->password1, SIGNAL( textChanged( const QString & ) ), SLOT( passwordsChanged( const QString & ) ) );
	connect( ui->password2, SIGNAL( textChanged( const QString & ) ), SLOT( passwordsChanged( const QString & ) ) );
	connect( ui->textToken, SIGNAL( textChanged( const QString & ) ), SLOT( tokenChanged( const QString & ) ) );

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
	cRegister->setUserinfo( ui->emailArea->text(), ui->password1->text(), ui->textToken->text() );
	cRegister->execute();
	ui->submitData->setDisabled( true );
	enableButton( Ok, false );
}

void 
GaduRegisterAccount::validateInput()
{
	if ( emailRegexp->exactMatch( ui->emailArea->text() ) &&
		ui->password1->text() == ui->password2->text()  && !ui->password1->text().isEmpty() && !ui->password2->text().isEmpty() &&
		!ui->textToken->text().isEmpty() ) 
	{
		ui->submitData->setDisabled( false );
	}
	else{
		ui->submitData->setDisabled( true );
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
	if ( emailRegexp->exactMatch( ui->emailArea->text() ) == FALSE && !ui->emailArea->text().isEmpty() ) {
		ui->emailArea->setPaletteBackgroundColor( QColor( 0, 150 , 227 ) );
	}
	else {
		ui->emailArea->setPaletteBackgroundColor( QColor( 255, 255 , 255 ) );
	}
	validateInput();
}

void 
GaduRegisterAccount::passwordsChanged( const QString & )
{
	if ( ui->password1->text() != ui->password2->text()  && !ui->password1->text().isEmpty() && !ui->password2->text().isEmpty() ) {
		ui->password1->setPaletteBackgroundColor( QColor( 164, 0 , 0 ) );
		ui->password2->setPaletteBackgroundColor( QColor( 164, 0 , 0 ) );
	}
	else {
		ui->password1->setPaletteBackgroundColor( QColor( 255, 255 , 255 ) );
		ui->password2->setPaletteBackgroundColor( QColor( 255, 255 , 255 ) );
	}
	validateInput();
}

void 
GaduRegisterAccount::registrationDone(  const QString& /*title*/,  const QString& /*what */ )
{
	ui->textToken->setDisabled( true );
	ui->password1->setDisabled( true );
	ui->password2->setDisabled( true );
	ui->emailArea->setDisabled( true );
	ui->submitData->setDisabled( true );
	emit registeredNumber( cRegister->newUin(), ui->password1->text() );
	updateStatus( i18n( "Your UIN is %1" ).arg(QString::number( cRegister->newUin() )  ) );
	enableButton( Ok, true );
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
	ui->textToken->setDisabled( true );
	ui->textToken->setText( "" );
	enableButton( Ok, false );
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
	ui->textToken->setDisabled( false );
	ui->pixmapToken->setPixmap( image );
	updateStatus( i18n( "" ) );
}

void 
GaduRegisterAccount::updateStatus( const QString status )
{
	ui->statusLabel->setAlignment( AlignCenter );
	ui->statusLabel->setText( status );
}

void 
GaduRegisterAccount::slotApply()
{
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
