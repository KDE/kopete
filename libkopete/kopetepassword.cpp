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

#include "kopeteuiglobal.h"
#include "kopetepassword.h"
#include "kopetepassworddialog.h"
#include "kopetewalletmanager.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qguardedptr.h>

#include <kactivelabel.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kwallet.h>
#include <kiconloader.h>
#include <kpassdlg.h>

/**
 * Function for symmetrically (en/de)crypting strings for config file,
 * taken from KMail.
 *
 * @author Stefan Taferner <taferner@alpin.or.at>
 */
static QString cryptStr( const QString &aStr )
{
	//Once Kopete depends on 3.2 just remove this function and use KStringHandler::obscure
	QString result;
	for ( uint i = 0; i < aStr.length(); i++ )
		result += ( aStr[ i ].unicode() < 0x20) ? aStr[ i ] : QChar( 0x1001F - aStr[ i ].unicode() );

	return result;
}

class Kopete::Password::Private
{
public:
	Private( const QString &group, uint maxLen )
	 : configGroup( group ), remembered( false ), maximumLength( maxLen ), isWrong( false )
	{
	}
	/** Group to use for KConfig and KWallet */
	QString configGroup;
	/** Is the password being remembered? */
	bool remembered;
	/** The current password in the KConfig file, or QString::null if no password there */
	QString passwordFromKConfig;
	/** The maximum length allowed for this password, or -1 if there is no limit */
	uint maximumLength;
	/** Is the current password known to be wrong? */
	bool isWrong;
	/** The cached password */
	QString cachedValue;
};

/**
 * Implementation detail of Kopete::Password: manages a single password request
 * @internal
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class KopetePasswordRequest : public KopetePasswordRequestBase
{
public:
	KopetePasswordRequest( Kopete::Password &pass )
	 : QObject( &pass ), mPassword( pass ), mWallet( 0 )
	{
	}

	/**
	 * Start the request - ask for the wallet
	 */
	void begin()
	{
		kdDebug( 14010 ) << k_funcinfo << endl;
		Kopete::WalletManager::self()->openWallet( this, SLOT( walletReceived( KWallet::Wallet* ) ) );
	}

	void walletReceived( KWallet::Wallet *wallet )
	{
		kdDebug( 14010 ) << k_funcinfo << endl;
		mWallet = wallet;
		processRequest();
	}

	/**
	 * Got wallet; now carry out whatever action this request represents
	 */
	virtual void processRequest() = 0;

	void slotOkPressed() {}
	void slotCancelPressed() {}

protected:
	Kopete::Password &mPassword;
	KWallet::Wallet *mWallet;
};

/**
 * Implementation detail of Kopete::Password: manages a single password retrieval request
 * @internal
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class KopetePasswordGetRequest : public KopetePasswordRequest
{
public:
	KopetePasswordGetRequest( Kopete::Password &pass )
	 : KopetePasswordRequest( pass )
	{
	}

	QString grabPassword()
	{
		// Before trying to read from the wallet, check if the config file holds a password.
		// If so, remove it from the config and set it through KWallet instead.
		QString pwd;
		if ( mPassword.d->remembered && !mPassword.d->passwordFromKConfig.isNull() )
		{
			pwd = mPassword.d->passwordFromKConfig;
			mPassword.set( pwd );
			return pwd;
		}

		if ( mWallet && mWallet->readPassword( mPassword.d->configGroup, pwd ) == 0 && !pwd.isNull() )
			return pwd;

		if ( mPassword.d->remembered && !mPassword.d->passwordFromKConfig.isNull() )
			return mPassword.d->passwordFromKConfig;

		return QString::null;
	}

	void finished( const QString &result )
	{
		mPassword.d->cachedValue = result;
		emit requestFinished( result );
		delete this;
	}
};

class KopetePasswordGetRequestPrompt : public KopetePasswordGetRequest
{
public:
	KopetePasswordGetRequestPrompt( Kopete::Password &pass,  const QPixmap &image, const QString &prompt, Kopete::Password::PasswordSource source )
	 : KopetePasswordGetRequest( pass ), mImage( image ), mPrompt( prompt ), mSource( source ), mView( 0 )
	{
	}

	void processRequest()
	{
		QString result = grabPassword();
		if ( mSource == Kopete::Password::FromUser || result.isNull() )
			doPasswordDialog( result );
		else
			finished( result );
	}

	void doPasswordDialog( const QString &password )
	{
		kdDebug( 14010 ) << k_funcinfo << endl;

		KDialogBase *passwdDialog = new KDialogBase( Kopete::UI::Global::mainWidget(), "passwdDialog", true, i18n( "Password Required" ),
			KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );

		mView = new KopetePasswordDialog( passwdDialog );
		passwdDialog->setMainWidget( mView );

		mView->m_text->setText( mPrompt );
		mView->m_image->setPixmap( mImage );
		mView->m_password->insert( password );
		int maxLength = mPassword.maximumLength();
		if ( maxLength != 0 )
			mView->m_password->setMaxLength( maxLength );

		// FIXME: either document what these are for or remove them - lilac
		mView->adjustSize();
		passwdDialog->adjustSize();

		connect( passwdDialog, SIGNAL( okClicked() ), SLOT( slotOkPressed() ) );
		connect( passwdDialog, SIGNAL( cancelClicked() ), SLOT( slotCancelPressed() ) );
		connect( this, SIGNAL( destroyed() ), passwdDialog, SLOT( deleteLater() ) );
		passwdDialog->show();
	}

	void slotOkPressed()
	{
		QString result = QString::fromLocal8Bit( mView->m_password->password() );
		if ( mView->m_save_passwd->isChecked() )
			mPassword.set( result );

		finished( result );
	}

	void slotCancelPressed()
	{
		finished( QString::null );
	}

private:
	QPixmap mImage;
	QString mPrompt;
	Kopete::Password::PasswordSource mSource;
	unsigned int mMaxLength;
	KopetePasswordDialog *mView;
};

class KopetePasswordGetRequestNoPrompt : public KopetePasswordGetRequest
{
public:
	KopetePasswordGetRequestNoPrompt( Kopete::Password &pass )
	 : KopetePasswordGetRequest( pass )
	{
	}

	void processRequest()
	{
		finished( grabPassword() );
	}
};

/**
 * Implementation detail of Kopete::Password: manages a single password change request
 * @internal
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class KopetePasswordSetRequest : public KopetePasswordRequest
{
public:
	KopetePasswordSetRequest( Kopete::Password &pass, const QString &newPass )
	 : KopetePasswordRequest( pass ), mNewPass( newPass )
	{
		if ( KApplication *app = KApplication::kApplication() )
			app->ref();
	}
	~KopetePasswordSetRequest()
	{
		if ( KApplication *app = KApplication::kApplication() )
			app->deref();
		kdDebug( 14010 ) << k_funcinfo << "job complete" << endl;
	}
	void processRequest()
	{
		if ( setPassword() )
		{
			mPassword.setWrong( false );
			mPassword.d->cachedValue = mNewPass;
		}
		delete this;
	}
	bool setPassword()
	{
		//TODO: refactor this function to remove duplication
		// and possibly to make it not need to be a friend of Kopete::Password

		if ( mNewPass.isNull() )
		{
			kdDebug( 14010 ) << k_funcinfo << " clearing password" << endl;

			mPassword.d->remembered = false;
			mPassword.d->passwordFromKConfig = QString::null;
			mPassword.writeConfig();
			if ( mWallet )
				mWallet->removeEntry( mPassword.d->configGroup );
			return true;
		}

		kdDebug( 14010 ) << k_funcinfo << " setting password for " << mPassword.d->configGroup << endl;

		if ( mWallet && mWallet->writePassword( mPassword.d->configGroup, mNewPass ) == 0 )
		{
			mPassword.d->remembered = true;
			mPassword.d->passwordFromKConfig = QString::null;
			mPassword.writeConfig();
			return true;
		}

		if ( KWallet::Wallet::isEnabled() )
		{
			// If we end up here, the wallet is enabled, but failed somehow.
			// Ask the user what to do now.

			// if the Kopete::Password object is destroyed during this message box's
			// nested event loop, and the user clicks 'Store Unsafe', we will crash!

			// solution: reparent to none, and track the parent. if it's deleted, don't use it.
			mPassword.removeChild( this );
			QGuardedPtr<Kopete::Password> watchParent = &mPassword;

			if ( KMessageBox::warningContinueCancel( Kopete::UI::Global::mainWidget(),
			        i18n( "<qt>Kopete is unable to save your password securely in your wallet!<br>"
			              "Do you want to save the password in the <b>unsafe</b> configuration file instead?</qt>" ),
			        i18n( "Unable to Store Secure Password" ),
			        KGuiItem( i18n( "Store &Unsafe" ), QString::fromLatin1( "unlock" ) ),
			        QString::fromLatin1( "KWalletFallbackToKConfig" ) ) != KMessageBox::Continue )
			{
				return false;
			}

			// if our parent was deleted, we can't save the password.
			// this is a corner case, so we don't worry about handling it properly. just make
			// sure we don't crash; tell the user and abort.
			//TODO: either handle this properly (ie make sure we actually save the password) or
			//      don't stop the app from closing when we're waiting to save it.
			// solution: reference count the password object. (TODO)
			if ( watchParent.isNull() )
			{
				KMessageBox::error( Kopete::UI::Global::mainWidget(), i18n( "Sorry, your password could not be saved at this time." ),
				                    i18n( "Unable to Store Password" ) );
				return false;
			}
		}

		mPassword.d->remembered = true;
		mPassword.d->passwordFromKConfig = mNewPass;
		mPassword.writeConfig();
		return true;
	}

private:
	QString mNewPass;
};

Kopete::Password::Password( const QString &configGroup, uint maximumLength, const char *name )
 : QObject( 0, name ), d( new Private( configGroup, maximumLength ) )
{
	readConfig();
}

Kopete::Password::~Password()
{
	delete d;
}

void Kopete::Password::readConfig()
{
	KConfig *config = KGlobal::config();
	config->setGroup( d->configGroup );

	QString passwordCrypted = config->readEntry( "Password" );
	if ( passwordCrypted.isNull() )
		d->passwordFromKConfig = QString::null;
	else
		d->passwordFromKConfig = cryptStr( passwordCrypted );

	d->remembered = config->readBoolEntry( "RememberPassword", false );
	d->isWrong = config->readBoolEntry( "PasswordIsWrong", false );
}

void Kopete::Password::writeConfig()
{
	KConfig *config = KGlobal::config();
	config->setGroup( d->configGroup );

	if ( d->remembered && !d->passwordFromKConfig.isNull() )
		config->writeEntry( "Password", cryptStr( d->passwordFromKConfig ) );
	else
		config->deleteEntry( "Password" );

	config->writeEntry( "RememberPassword", d->remembered );
	config->writeEntry( "PasswordIsWrong", d->isWrong );
}

int Kopete::Password::preferredImageSize()
{
	return IconSize(KIcon::Toolbar);
}

uint Kopete::Password::maximumLength()
{
	return d->maximumLength;
}

void Kopete::Password::setMaximumLength( uint max )
{
	d->maximumLength = max;
}

bool Kopete::Password::isWrong()
{
	return d->isWrong;
}

void Kopete::Password::setWrong( bool bWrong )
{
	d->isWrong = bWrong;
	writeConfig();

	if ( bWrong ) d->cachedValue = QString::null;
}

void Kopete::Password::requestWithoutPrompt( QObject *returnObj, const char *slot )
{
	KopetePasswordRequest *request = new KopetePasswordGetRequestNoPrompt( *this );
	// call connect on returnObj so we can still connect if 'slot' is protected/private
	returnObj->connect( request, SIGNAL( requestFinished( const QString & ) ), slot );
	request->begin();
}

void Kopete::Password::request( QObject *returnObj, const char *slot, const QPixmap &image, const QString &prompt, Kopete::Password::PasswordSource source )
{
	KopetePasswordRequest *request = new KopetePasswordGetRequestPrompt( *this, image, prompt, source );
	returnObj->connect( request, SIGNAL( requestFinished( const QString & ) ), slot );
	request->begin();
}

QString Kopete::Password::retrieve( const QPixmap &image, const QString &prompt, Kopete::Password::PasswordSource source )
{
	uint maxLength = maximumLength();
	if ( source == Kopete::Password::FromConfigOrUser )
	{
		if( KWallet::Wallet *wallet = Kopete::WalletManager::self()->wallet() )
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

		if ( d->remembered && !d->passwordFromKConfig.isNull() )
			return d->passwordFromKConfig;
	}

	KDialogBase *passwdDialog = new KDialogBase( Kopete::UI::Global::mainWidget(), "passwdDialog", true, i18n( "Password Required" ),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );

	KopetePasswordDialog *view = new KopetePasswordDialog( passwdDialog );
	passwdDialog->setMainWidget( view );

	view->m_text->setText( prompt );
	view->m_image->setPixmap( image );
	if ( maxLength != 0 )
		view->m_password->setMaxLength( maxLength );

	// FIXME: either document what these are for or remove them - lilac
	view->adjustSize();
	passwdDialog->adjustSize();

	QString pass;
	if ( passwdDialog->exec() == QDialog::Accepted )
	{
		d->remembered = view->m_save_passwd->isChecked();
		pass = QString::fromLocal8Bit( view->m_password->password() );
		if ( d->remembered )
			set( pass );
	}

	passwdDialog->deleteLater();
	return pass;
}

QString Kopete::Password::cachedValue()
{
	return d->cachedValue;
}

void Kopete::Password::set( const QString &pass )
{
	// if we're being told to forget the password, and we aren't remembering one,
	// don't try to open the wallet. fixes bug #71804.
	if ( pass.isNull() && !remembered() )
		return;

	KopetePasswordRequest *request = new KopetePasswordSetRequest( *this, pass );
	request->begin();
}

bool Kopete::Password::remembered()
{
	return d->remembered;
}

#include "kopetepassword.moc"

// vim: set noet ts=4 sts=4 sw=4:
