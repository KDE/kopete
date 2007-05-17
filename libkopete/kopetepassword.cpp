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

#include <kwallet.h>

#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kactivelabel.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpassdlg.h>
#include <kstringhandler.h>

class Kopete::Password::Private
{
public:
	Private( const QString &group, uint maxLen, bool blanksAllowed )
	 : refCount( 1 ), configGroup( group ), remembered( false ), maximumLength( maxLen ),
	 isWrong( false ), allowBlankPassword( blanksAllowed )
	{
	}
	Private *incRef()
	{
		++refCount;
		return this;
	}
	void decRef()
	{
		if( --refCount == 0 )
			delete this;
	}
	/** Reference count */
	int refCount;
	/** Group to use for KConfig and KWallet */
	const QString configGroup;
	/** Is the password being remembered? */
	bool remembered;
	/** The current password in the KConfig file, or QString::null if no password there */
	QString passwordFromKConfig;
	/** The maximum length allowed for this password, or -1 if there is no limit */
	uint maximumLength;
	/** Is the current password known to be wrong? */
	bool isWrong;
	/** Are we allowed to have blank passwords? */
	bool allowBlankPassword;
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
	KopetePasswordRequest( QObject *owner, Kopete::Password &pass )
	 : QObject( owner ), mPassword( pass ), mWallet( 0 )
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
	Kopete::Password mPassword;
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
	KopetePasswordGetRequest( QObject *owner, Kopete::Password &pass )
	 : KopetePasswordRequest( owner, pass )
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
	KopetePasswordGetRequestPrompt( QObject *owner, Kopete::Password &pass,  const QPixmap &image, const QString &prompt, Kopete::Password::PasswordSource source )
	 : KopetePasswordGetRequest( owner, pass ), mImage( image ), mPrompt( prompt ), mSource( source ), mView( 0 )
	{
	}

	void processRequest()
	{
		QString result = grabPassword();
		if ( mSource == Kopete::Password::FromUser || result.isNull() )
			doPasswordDialog();
		else
			finished( result );
	}

	void doPasswordDialog()
	{
		kdDebug( 14010 ) << k_funcinfo << endl;

		KDialogBase *passwdDialog = new KDialogBase( Kopete::UI::Global::mainWidget(), "passwdDialog", true, i18n( "Password Required" ),
			KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );

		mView = new KopetePasswordDialog( passwdDialog );
		passwdDialog->setMainWidget( mView );

		mView->m_text->setText( mPrompt );
		mView->m_image->setPixmap( mImage );
		/* Do not put the default password, or it will confuse those which doesn't echo anything for the password
		mView->m_password->insert( password );
		*/
		int maxLength = mPassword.maximumLength();
		if ( maxLength != 0 )
			mView->m_password->setMaxLength( maxLength );
		mView->m_password->setFocus();

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
	KopetePasswordGetRequestNoPrompt( QObject *owner, Kopete::Password &pass )
	 : KopetePasswordGetRequest( owner, pass )
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
	 : KopetePasswordRequest( 0, pass ), mNewPass( newPass )
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

			//NOTE: This will start a nested event loop. However, this is fine; the only code we
			// call after this point is in Kopete::Password, so as long as we've not been deleted
			// everything should work out OK. We have no parent QObject, so we should survive.
			if ( KMessageBox::warningContinueCancel( Kopete::UI::Global::mainWidget(),
			        i18n( "<qt>Kopete is unable to save your password securely in your wallet;<br>"
			              "do you want to save the password in the <b>unsafe</b> configuration file instead?</qt>" ),
			        i18n( "Unable to Store Secure Password" ),
			        KGuiItem( i18n( "Store &Unsafe" ), QString::fromLatin1( "unlock" ) ),
			        QString::fromLatin1( "KWalletFallbackToKConfig" ) ) != KMessageBox::Continue )
			{
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

class KopetePasswordClearRequest : public KopetePasswordRequest
{
public:
	KopetePasswordClearRequest( Kopete::Password &pass )
	 : KopetePasswordRequest( 0, pass )
	{
		if ( KApplication *app = KApplication::kApplication() )
			app->ref();
	}
	~KopetePasswordClearRequest()
	{
		if ( KApplication *app = KApplication::kApplication() )
			app->deref();
		kdDebug( 14010 ) << k_funcinfo << "job complete" << endl;
	}
	void processRequest()
	{
		if ( clearPassword() )
		{
			mPassword.setWrong( true );
			mPassword.d->cachedValue = QString::null;
		}

		delete this;
	}
	bool clearPassword()
	{
		kdDebug( 14010 ) << k_funcinfo << " clearing password" << endl;

		mPassword.d->remembered = false;
		mPassword.d->passwordFromKConfig = QString::null;
		mPassword.writeConfig();
		if ( mWallet )
			mWallet->removeEntry( mPassword.d->configGroup );
		return true;
	}
};

Kopete::Password::Password( const QString &configGroup, uint maximumLength, const char *name )
 : QObject( 0, name ), d( new Private( configGroup, maximumLength, false ) )
{
	readConfig();
}

Kopete::Password::Password( const QString &configGroup, uint maximumLength,
	bool allowBlankPassword, const char *name )
 : QObject( 0, name ), d( new Private( configGroup, maximumLength, allowBlankPassword ) )
{
	readConfig();
}

Kopete::Password::Password( Password &other, const char *name )
 : QObject( 0, name ), d( other.d->incRef() )
{
}

Kopete::Password::~Password()
{
	d->decRef();
}

Kopete::Password &Kopete::Password::operator=( Password &other )
{
	if ( d == other.d ) return *this;
	d->decRef();
	d = other.d->incRef();
	return *this;
}

void Kopete::Password::readConfig()
{
	KConfig *config = KGlobal::config();
	config->setGroup( d->configGroup );

	QString passwordCrypted = config->readEntry( "Password" );
	if ( passwordCrypted.isNull() )
		d->passwordFromKConfig = QString::null;
	else
		d->passwordFromKConfig = KStringHandler::obscure( passwordCrypted );

	d->remembered = config->readBoolEntry( "RememberPassword", false );
	d->isWrong = config->readBoolEntry( "PasswordIsWrong", false );
}

void Kopete::Password::writeConfig()
{
	KConfig *config = KGlobal::config();
	if(!config->hasGroup(d->configGroup))
	{
		//### (KOPETE)
		// if the kopete account has been removed, we have no way to know it.
		//  but we don't want in any case to recreate the group.
		//  see Bug 106460
		// (the problem is that when we remove the account, we remove the password
		//  also, which cause a call to this function )
		return;
	}
		  
	config->setGroup( d->configGroup );

	if ( d->remembered && !d->passwordFromKConfig.isNull() )
		config->writeEntry( "Password", KStringHandler::obscure( d->passwordFromKConfig ) );
	else
		config->deleteEntry( "Password" );

	config->writeEntry( "RememberPassword", d->remembered );
	config->writeEntry( "PasswordIsWrong", d->isWrong );
}

int Kopete::Password::preferredImageSize()
{
	return IconSize(KIcon::Toolbar);
}

bool Kopete::Password::allowBlankPassword()
{
	return d->allowBlankPassword;
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
	KopetePasswordRequest *request = new KopetePasswordGetRequestNoPrompt( returnObj, *this );
	// call connect on returnObj so we can still connect if 'slot' is protected/private
	returnObj->connect( request, SIGNAL( requestFinished( const QString & ) ), slot );
	request->begin();
}

void Kopete::Password::request( QObject *returnObj, const char *slot, const QPixmap &image, const QString &prompt, Kopete::Password::PasswordSource source )
{
	KopetePasswordRequest *request = new KopetePasswordGetRequestPrompt( returnObj, *this, image, prompt, source );
	returnObj->connect( request, SIGNAL( requestFinished( const QString & ) ), slot );
	request->begin();
}

QString Kopete::Password::cachedValue()
{
	return d->cachedValue;
}

void Kopete::Password::set( const QString &pass )
{
	// if we're being told to forget the password, and we aren't remembering one,
	// don't try to open the wallet. fixes bug #71804.
	if( pass.isNull() && !d->allowBlankPassword )
	{
		if( remembered() )
			clear();
		return;
	}

	KopetePasswordRequest *request = new KopetePasswordSetRequest( *this, pass );
	request->begin();
}

void Kopete::Password::clear()
{
	KopetePasswordClearRequest *request = new KopetePasswordClearRequest( *this );
	request->begin();
}

bool Kopete::Password::remembered()
{
	return d->remembered;
}

#include "kopetepassword.moc"

// vim: set noet ts=4 sts=4 sw=4:
