// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003-2004 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
//
// gadueditaccount.cpp
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

#include "gadueditaccount.h"
#include "gaduaccount.h"
#include "gaduprotocol.h"

#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbutton.h>
#include <qregexp.h>
#include <qpushbutton.h>

#include <klineedit.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpassdlg.h>

#include "kopetepasswordwidget.h"

GaduEditAccount::GaduEditAccount( GaduProtocol* proto, Kopete::Account* ident, QWidget* parent, const char* name )
: GaduAccountEditUI( parent, name ), KopeteEditAccountWidget( ident ), protocol_( proto ), rcmd( 0 )
{

#ifdef __GG_LIBGADU_HAVE_OPENSSL
	isSsl = true;
#else
	isSsl = false;
#endif

	useTls_->setDisabled( !isSsl );

	if ( account() == NULL ) {
		useTls_->setCurrentItem( GaduAccount::TLS_no );
		registerNew->setEnabled( true );
	}
	else {
		registerNew->setDisabled( true );
		loginEdit_->setDisabled( true );
		loginEdit_->setText( account()->accountId() );

		passwordWidget_->load( &static_cast<GaduAccount*>(account())->password() );
		
		QString nick = account()->myself()->property( 
				Kopete::Global::Properties::self()->nickName() ).value().toString();	
		if ( nick.isEmpty() ) {
			nick = account()->myself()->contactId();
		}
		
		nickName->setText( nick );

		autoLoginCheck_->setChecked( account()->autoConnect() );
		dccCheck_->setChecked( static_cast<GaduAccount*>(account())->dccEnabled() );
		useTls_->setCurrentItem( isSsl ?  ( static_cast<GaduAccount*> (account()) ->useTls() ) : 2 );
	}

	QObject::connect( registerNew, SIGNAL( clicked( ) ), SLOT( registerNewAccount( ) ) );
}

void
GaduEditAccount::registerNewAccount()
{
	registerNew->setDisabled( true );
	regDialog = new GaduRegisterAccount( NULL , "Register account dialog" );
	connect( regDialog, SIGNAL( registeredNumber( unsigned int, QString  ) ), SLOT( newUin( unsigned int, QString  ) ) );
	if ( regDialog->exec() != QDialog::Accepted ) {
		loginEdit_->setText( "" );
		return;
	}
	registerNew->setDisabled( false );
}

void
GaduEditAccount::registrationFailed()
{
	KMessageBox::sorry( this, i18n( "<b>Registration FAILED.</b>" ), i18n( "Gadu-Gadu" ) );
}

void
GaduEditAccount::newUin( unsigned int uin, QString password )
{
	if ( uin ) {
		loginEdit_->setText( QString::number( uin ) );
		passwordWidget_->setPassword( password );
	}
	else {
		// registration failed, enable button again
		registerNew->setDisabled( false );
	}
}

bool
GaduEditAccount::validateData()
{

	if ( loginEdit_->text().isEmpty() ) {
		KMessageBox::sorry( this, i18n( "<b>Enter UIN please.</b>" ), i18n( "Gadu-Gadu" ) );
		return false;
	}

	if ( loginEdit_->text().toInt() < 0 || loginEdit_->text().toInt() == 0 ) {
		KMessageBox::sorry( this, i18n( "<b>UIN should be a positive number.</b>" ), i18n( "Gadu-Gadu" ) );
		return false;
	}

	if ( !passwordWidget_->validate() ) {
		KMessageBox::sorry( this, i18n( "<b>Enter password please.</b>" ), i18n( "Gadu-Gadu" ) );
		return false;
	}

	return true;
}

Kopete::Account*
GaduEditAccount::apply()
{
	if ( account() == NULL ) {
		setAccount( new GaduAccount( protocol_, loginEdit_->text() ) );
	}

	account()->setAutoConnect( autoLoginCheck_->isChecked() );

	passwordWidget_->save( &static_cast<GaduAccount*>(account())->password() );

	account()->myself()->setProperty( Kopete::Global::Properties::self()->nickName(), nickName->text() );
	
	// this is changed only here, so i won't add any proper handling now
        account()->configGroup()->writeEntry( QString::fromAscii( "nickName" ), nickName->text() );
		
	account()->setAutoConnect( autoLoginCheck_->isChecked() );
	( static_cast<GaduAccount*> (account()) )->setUseTls( (GaduAccount::tlsConnection) useTls_->currentItem() );

	if ( static_cast<GaduAccount*>(account())->setDcc( dccCheck_->isChecked() ) == false ) {
		KMessageBox::sorry( this, i18n( "<b>Starting DCC listening socket failed; dcc is not working now.</b>" ), i18n( "Gadu-Gadu" ) );
	}

	return account();
}

#include "gadueditaccount.moc"

