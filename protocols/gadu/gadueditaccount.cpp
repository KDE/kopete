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
#include "gadusession.h"

#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbutton.h>
#include <qregexp.h>
#include <qpushbutton.h>
#include <qgroupbox.h>

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
		account_ = NULL;
	}
	else {
		account_ = static_cast<GaduAccount*>(ident);

		registerNew->setDisabled( true );
		loginEdit_->setDisabled( true );
		loginEdit_->setText( account_->accountId() );

		passwordWidget_->load( &account_->password() );

		QString nick = account()->myself()->property(
				Kopete::Global::Properties::self()->nickName() ).value().toString();
		if ( nick.isEmpty() ) {
			nick = account_->myself()->contactId();
		}

		nickName->setText( nick );

		autoLoginCheck_->setChecked( account_->excludeConnect() );
		dccCheck_->setChecked( account_->dccEnabled() );
		useTls_->setCurrentItem( isSsl ? ( account_->useTls() ) : 2 );
		ignoreCheck_->setChecked( account_->ignoreAnons() );

		connect( account(), SIGNAL( pubDirSearchResult( const SearchResult&, unsigned int ) ),
					SLOT( slotSearchResult( const SearchResult&, unsigned int ) ) );
		connectLabel->setText( i18n( "personal information being fetched from server",
					"<p align=\"center\">Fetching from server</p>" ) );
		seqNr = account_->getPersonalInformation();
	}

	connect( registerNew, SIGNAL( clicked( ) ), SLOT( registerNewAccount( ) ) );

	QWidget::setTabOrder( loginEdit_, passwordWidget_->mRemembered );
	QWidget::setTabOrder( passwordWidget_->mRemembered, passwordWidget_->mPassword );
	QWidget::setTabOrder( passwordWidget_->mPassword, autoLoginCheck_ );
}

void
GaduEditAccount::publishUserInfo()
{
	ResLine sr;

	enableUserInfo( false );
	
	sr.firstname	= uiName->text();
	sr.surname	= uiSurname->text();
	sr.nickname	= nickName->text();
	sr.age		= uiYOB->text();
	sr.city		= uiCity->text();
	sr.meiden	= uiMeiden->text();
	sr.orgin	= uiOrgin->text();

	kdDebug(14100) << uiGender->currentItem() << " gender " << endl;
	if ( uiGender->currentItem() == 1 ) {
		kdDebug(14100) << "so you become female now" << endl;
		sr.gender = QString( GG_PUBDIR50_GENDER_SET_FEMALE );
	}
	if ( uiGender->currentItem() == 2 ) {
		kdDebug(14100) << "so you become male now" << endl;
		sr.gender = QString( GG_PUBDIR50_GENDER_SET_MALE );
	}

	if ( account_ ) {
		account_->publishPersonalInformation( sr );
	}
}

void
GaduEditAccount::slotSearchResult( const SearchResult& result, unsigned int seq )
{
	if ( !( seq != 0 && seqNr != 0 && seq == seqNr ) ) {
		return;
	}
        
	connectLabel->setText( " " );
		
	uiName->setText( result[0].firstname );
	uiSurname->setText( result[0].surname );
	nickName->setText( result[0].nickname );
	uiYOB->setText( result[0].age );
	uiCity->setText( result[0].city );

	kdDebug( 14100 ) << "gender found: " << result[0].gender << endl;
	if ( result[0].gender == QString( GG_PUBDIR50_GENDER_SET_FEMALE ) ) {
		uiGender->setCurrentItem( 1 );
		kdDebug(14100) << "looks like female" << endl;
	}
	else {
		if ( result[0].gender == QString( GG_PUBDIR50_GENDER_SET_MALE ) ) {
			uiGender->setCurrentItem( 2 );
			kdDebug( 14100 ) <<" looks like male" << endl;
		}
	}

	uiMeiden->setText( result[0].meiden );
	uiOrgin->setText( result[0].orgin );

	enableUserInfo( true );
	
	disconnect( SLOT( slotSearchResult( const SearchResult&, unsigned int ) ) );
}

void
GaduEditAccount::enableUserInfo( bool e )
{
	uiName->setEnabled( e );
	uiSurname->setEnabled( e );
	uiYOB->setEnabled( e );
	uiCity->setEnabled( e );
	uiGender->setEnabled( e );
	uiMeiden->setEnabled( e );
	uiOrgin->setEnabled( e );
	
//	connectLabel->setEnabled( !e );
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
	publishUserInfo();
	
	if ( account() == NULL ) {
		setAccount( new GaduAccount( protocol_, loginEdit_->text() ) );
		account_ = static_cast<GaduAccount*>( account() );
	}

	account_->setExcludeConnect( autoLoginCheck_->isChecked() );

	passwordWidget_->save( &account_->password() );

	account_->myself()->setProperty( Kopete::Global::Properties::self()->nickName(), nickName->text() );

	// this is changed only here, so i won't add any proper handling now
        account_->configGroup()->writeEntry( QString::fromAscii( "nickName" ), nickName->text() );

	account_->setExcludeConnect( autoLoginCheck_->isChecked() );
	account_->setUseTls( (GaduAccount::tlsConnection) useTls_->currentItem() );
	account_->setIgnoreAnons( ignoreCheck_->isChecked() );
	
	if ( account_->setDcc( dccCheck_->isChecked() ) == false ) {
		KMessageBox::sorry( this, i18n( "<b>Starting DCC listening socket failed; dcc is not working now.</b>" ), i18n( "Gadu-Gadu" ) );
	}

	return account();
}

#include "gadueditaccount.moc"

