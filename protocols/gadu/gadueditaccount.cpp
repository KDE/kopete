// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 2003 Casey Allen Shobe 	<cshobe@somerandomdomain.com>
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

#include <klineedit.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>

GaduEditAccount::GaduEditAccount( GaduProtocol* proto, KopeteAccount* ident, QWidget* parent, const char* name )
: GaduAccountEditUI( parent, name ), EditAccountWidget( ident ), protocol_( proto ), rcmd( 0 )
{

#ifdef __GG_LIBGADU_HAVE_OPENSSL
	isSsl = true;
#else
	isSsl = false;
#endif

	connect( rememberCheck_, SIGNAL( toggled( bool ) ),
		this, SLOT( pswdChecked( bool ) ) );

	useTls_->setDisabled( !isSsl );

	if ( !m_account ) {
		useTls_->setCurrentItem( isSsl ? 0 : 2 );
	}
	else {
		loginEdit_->setDisabled( true );
		loginEdit_->setText( m_account->accountId() );

		if ( m_account->rememberPassword() ) {
			passwordEdit_->setText( m_account->password()  );
		}
		else {
			passwordEdit_->setText( "" );
		}

		nickName->setText( m_account->myself()->displayName() );

		rememberCheck_->setChecked( m_account->rememberPassword() );
		autoLoginCheck_->setChecked( m_account->autoLogin() );
		useTls_->setCurrentItem( isSsl ? ( static_cast<GaduAccount*> (m_account) )->isConnectionEncrypted() : 2 );
	}
}

void GaduEditAccount::pswdChecked( bool on )
{
	if ( on ) {
		passwordEdit_->setDisabled( false );
		passwordText->setDisabled( false );
	}
	else {
		passwordEdit_->setDisabled( true );
		passwordText->setDisabled( true );
	}
}

bool GaduEditAccount::validateData()
{

	if ( loginEdit_->text().toInt() < 0 || loginEdit_->text().toInt() == 0 ) {
		KMessageBox::sorry( this, i18n( "<b>UIN should be a positive number.</b>" ), i18n( "Gadu-Gadu" ) );
		return false;
	}

	if ( passwordEdit_->text().isEmpty() && rememberCheck_->isChecked() ) {
		KMessageBox::sorry( this, i18n( "<b>Enter password please.</b>" ), i18n( "Gadu-Gadu" ) );
		return false;
	}

	return true;
}

KopeteAccount* GaduEditAccount::apply()
{
	if ( m_account == NULL ) {
		m_account = new GaduAccount( protocol_, loginEdit_->text() );
		if ( !m_account ) {
			kdDebug(14100)<<"Couldn't create GaduAccount object, fatal!"<<endl;
			return NULL;
		}
		m_account->setAccountId( loginEdit_->text() );
		
	}

	m_account->setAutoLogin( autoLoginCheck_->isChecked() );

	if( rememberCheck_->isChecked() && passwordEdit_->text().length() ) {
		m_account->setPassword( passwordEdit_->text() );
	}
	else {
		m_account->setPassword();
	}

	m_account->myself()->rename( nickName->text() );

	// this is changed only here, so i won't add any proper handling now
	m_account->setPluginData( m_account->protocol(),  QString::fromAscii( "nickName" ), nickName->text() );

	m_account->setAutoLogin( autoLoginCheck_->isChecked() );
	( static_cast<GaduAccount*> (m_account) )->useTls( useTls_->currentItem() );

	return m_account;
}

#include "gadueditaccount.moc"

