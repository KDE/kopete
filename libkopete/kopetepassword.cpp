/*
    kopetepassword.cpp - Kopete Password

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetepassword.h"
#include "kopetepassworddialog.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kdialogbase.h>
#include <klocale.h>

#if KDE_IS_VERSION( 3, 1, 90 )
#include "kopetewalletmanager.h"
// KMessageBox is only used in the KWallet code path
#include <kmessagebox.h>
#include <kwallet.h>
#endif

namespace
{

/**
 * Function for symmetrically (en/de)crypting strings for config file,
 * taken from KMail.
 *
 * @author Stefan Taferner <taferner@alpin.or.at>
 */
QString cryptStr( const QString &aStr )
{
	QString result;
	for ( uint i = 0; i < aStr.length(); i++ )
		result += ( aStr[ i ].unicode() < 0x20) ? aStr[ i ] : QChar( 0x1001F - aStr[ i ].unicode() );

	return result;
}

}

struct KopetePassword::KopetePasswordPrivate
{
	KopetePasswordPrivate( const QString &group, const QString &id, const QString &displayName )
	 : id( id ), configGroup( group ), displayName( displayName ), remembered( false )
	{
	}
	/** ID for password in KConfig and KWallet */
	QString id;
	/** Group to use for KConfig and KWallet */
	QString configGroup;
	/** Display name for this password when asking the user for it */
	QString displayName;
	/** Is the password being remembered? */
	bool remembered;
	/** The current password in the KConfig file, or QString::null if no password there */
	QString passwordFromKConfig;
};

KopetePassword::KopetePassword( const QString &configGroup, const QString &passwordId, const QString &displayName, const char *name )
 : QObject( 0, name ), d( new KopetePasswordPrivate( configGroup, passwordId, displayName ) ) 
{
	readConfig();
}

KopetePassword::~KopetePassword()
{
	delete d;
}

void KopetePassword::setDisplayName( const QString &name )
{
	d->displayName = name;
}

void KopetePassword::readConfig()
{
	KConfig *config = KGlobal::config();
	config->setGroup( d->configGroup );

	QString passwordCrypted = config->readEntry( "Password" );
	if ( passwordCrypted.isNull() )
		d->passwordFromKConfig = QString::null;
	else
		d->passwordFromKConfig = cryptStr( passwordCrypted );

	d->remembered = config->readBoolEntry( "RememberPassword", false );
}

void KopetePassword::writeConfig()
{
	KConfig *config = KGlobal::config();
	config->setGroup( d->configGroup );

	if ( d->remembered && !d->passwordFromKConfig.isNull() )
		config->writeEntry( "Password", cryptStr( d->passwordFromKConfig ) );
	else
		config->deleteEntry( "Password" );

	config->writeEntry( "RememberPassword", d->remembered );
}

QString KopetePassword::retrieve( bool error, bool *ok, unsigned int maxLength )
{
	if ( ok )
		*ok = true;

	if ( !error )
	{
#if KDE_IS_VERSION( 3, 1, 90 )
		if( KWallet::Wallet *wallet = KopeteWalletManager::self()->wallet( KopeteWalletManager::DoNotCreateFolder ) )
		{
			// Before trying to read from the wallet, check if the config file holds a password.
			// If so, remove it from the config and set it through KWallet instead.
			QString pwd;
			if ( d->remembered && !d->passwordFromKConfig.isNull() )
			{
				pwd = d->passwordFromKConfig;
				set( pwd );
				return pwd;
			}

			if ( wallet->readPassword( d->configGroup, pwd ) == 0 && !pwd.isNull() )
				return pwd;
		}
#endif

		if ( d->remembered && !d->passwordFromKConfig.isNull() )
			return d->passwordFromKConfig;
	}

	KDialogBase *passwdDialog = new KDialogBase( qApp->mainWidget(), "passwdDialog", true, i18n( "Password Required" ),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );

	KopetePasswordDialog *view = new KopetePasswordDialog( passwdDialog );

	if ( error )
	{
		view->m_text->setText( i18n( "<b>The password was wrong! Please re-enter your password for %1</b>" ).arg( d->displayName ) );

		// Invalidate any stored pass
		set();
	}
	else
	{
		view->m_text->setText( i18n( "Please enter your password for %1" ).arg( d->displayName ) );
	}

	passwdDialog->setMainWidget( view );

	view->m_login->setText( d->id );
// TODO	view->m_autologin->setChecked( d->autologin );
	if ( maxLength != 0 )
		view->m_password->setMaxLength( maxLength );

	view->adjustSize();
	passwdDialog->adjustSize();

	QString pass;
	if ( passwdDialog->exec() == QDialog::Accepted )
	{
		d->remembered = view->m_save_passwd->isChecked();
// TODO		d->autologin = view->m_autologin->isChecked();
		pass = view->m_password->text();
		if ( d->remembered )
			set( pass );
	}
	else
	{
		if ( ok )
			*ok = false;
	}

	passwdDialog->deleteLater();
	return pass;
}

void KopetePassword::set( const QString &pass )
{
	if ( pass.isNull() )
	{
		// FIXME: This is a quick workaround for the problem that after Jason
		//        added the rememberPassword flag he didn't accordingly update
		//        all plugins to setRememberPassword( false ), so they now
		//        try to set a null pass when the pass is not to be remembered.
		//
		//        After KDE 3.2 this should be fixed by disallowing null
		//        passwords here and adding said property setter method - Martijn
		d->passwordFromKConfig = QString::null;
		d->remembered = false;
		writeConfig();
		return;
	}

#if KDE_IS_VERSION( 3, 1, 90 )
	kdDebug( 14010 ) << k_funcinfo << endl;

	if ( KWallet::Wallet *wallet = KopeteWalletManager::self()->wallet() )
	{
		if ( wallet->writePassword( d->configGroup, pass ) == 0 )
		{
			// Remove any pass from KConfig if it is still there
			if ( !d->passwordFromKConfig.isNull() )
			{
				d->passwordFromKConfig = QString::null;
				writeConfig();
			}
			return;
		}
	}

	if ( KWallet::Wallet::isEnabled() )
	{
		// If we end up here, the wallet is enabled, but failed somehow.
		// Ask the user what to do now.
		if ( KMessageBox::warningContinueCancel( qApp->mainWidget(),
			i18n( "<qt>Kopete is unable to save your password securely in your wallet!<br>"
			"Do you want to save the password in the <b>unsafe</b> configuration file instead?</qt>" ),
			i18n( "Unable to Store Secure Password" ),
			KGuiItem( i18n( "Store &Unsafe" ), QString::fromLatin1( "unlock" ) ),
			QString::fromLatin1( "KWalletFallbackToKConfig" ) ) != KMessageBox::Continue )
		{
			return;
		}
	}
#endif

	d->passwordFromKConfig = pass;
	writeConfig();
}

bool KopetePassword::remembered()
{
	return d->remembered;
}

#include "kopetepassword.moc"

// vim: set noet ts=4 sts=4 sw=4:

