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
#include "kopeteuiglobal.h"
#include "kopetewalletmanager.h"

#include <kwallet.h>

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <k3activelabel.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kstringhandler.h>
#include <kpassworddialog.h>

class Kopete::Password::Private
{
public:
	Private( const QString &group, bool blanksAllowed )
	 : refCount( 1 ), configGroup( group ), remembered( false ),
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
	/** The current password in the KConfig file, or QString() if no password there */
	QString passwordFromKConfig;
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
	 : KopetePasswordRequestBase( owner ), mPassword( pass ), mWallet( 0 )
	{
	}

	/**
	 * Start the request - ask for the wallet
	 */
	void begin()
	{
		kDebug( 14010 );

		Kopete::WalletManager::self()->openWallet( this, SLOT(walletReceived(KWallet::Wallet*)) );
	}

	void walletReceived( KWallet::Wallet *wallet )
	{
		kDebug( 14010 ) ;
		mWallet = wallet;
		processRequest();
	}

	/**
	 * Got wallet; now carry out whatever action this request represents
	 */
	virtual void processRequest() = 0;

	void gotPassword(const QString&, bool) {}
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

		if ( mWallet && mWallet->readPassword( mPassword.d->configGroup, pwd ) == 0 && !pwd.isEmpty() )
			return pwd;

		if ( mPassword.d->remembered && !mPassword.d->passwordFromKConfig.isEmpty() )
			return mPassword.d->passwordFromKConfig;

		return QString();
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
		const QString result = grabPassword();
		if ( mSource == Kopete::Password::FromUser || result.isNull() )
			doPasswordDialog();
		else
			finished( result );
	}

	void doPasswordDialog()
	{
		kDebug( 14010 ) ;

		KPasswordDialog *passwdDialog = new KPasswordDialog( Kopete::UI::Global::mainWidget(), KPasswordDialog::ShowKeepPassword );
		passwdDialog->setWindowTitle( i18n( "Password Required" ) );
		passwdDialog->setPrompt( mPrompt );
		passwdDialog->setPixmap( mImage );

		connect( passwdDialog, SIGNAL(gotPassword(QString,bool)), SLOT(gotPassword(QString,bool))) ;
		connect( passwdDialog, SIGNAL(rejected()), SLOT(slotCancelPressed()) );
		connect( this, SIGNAL(destroyed()), passwdDialog, SLOT(deleteLater()) );
		passwdDialog->show();
	}

	void gotPassword(const QString& pass, bool keep)
	{
		if ( keep )
			mPassword.set( pass );

		finished( pass );
	}

	void slotCancelPressed()
	{
		finished( QString() );
	}

private:
	QPixmap mImage;
	QString mPrompt;
	Kopete::Password::PasswordSource mSource;
	QWidget *mView;
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
		KGlobal::ref();
	}
	~KopetePasswordSetRequest()
	{
		KGlobal::deref();
		kDebug( 14010 ) << "job complete";
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
		kDebug( 14010 ) << " setting password for " << mPassword.d->configGroup;

		if ( mWallet && mWallet->writePassword( mPassword.d->configGroup, mNewPass ) == 0 )
		{
			mPassword.d->remembered = true;
			mPassword.d->passwordFromKConfig.clear();
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
			        i18n( "<qt>Kopete is unable to save your password securely in your wallet;<br />"
			              "do you want to save the password in the <b>unsafe</b> configuration file instead?</qt>" ),
			        i18n( "Unable to Store Secure Password" ),
			        KGuiItem( i18n( "Store &Unsafe" ), QString::fromLatin1( "unlock" ) ), KStandardGuiItem::cancel(),
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
		KGlobal::ref();
	}
	~KopetePasswordClearRequest()
	{
		KGlobal::deref();
		kDebug( 14010 ) << "job complete";
	}
	void processRequest()
	{
		if ( clearPassword() )
		{
			mPassword.setWrong( true );
			mPassword.d->cachedValue.clear();
		}

		delete this;
	}
	bool clearPassword()
	{
		kDebug( 14010 ) << " clearing password";

		mPassword.d->remembered = false;
		mPassword.d->passwordFromKConfig.clear();
		mPassword.writeConfig();
		if ( mWallet )
			mWallet->removeEntry( mPassword.d->configGroup );
		return true;
	}
};


Kopete::Password::Password( const QString &configGroup, bool allowBlankPassword )
 : QObject( 0 ), d( new Private( configGroup, allowBlankPassword ) )
{
	readConfig();
}

Kopete::Password::Password( const Password &other )
 : QObject( 0 ), d( other.d->incRef() )
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
	KConfigGroup config(KGlobal::config(), d->configGroup );

	const QString passwordCrypted = config.readEntry( "Password", QString() );
	if ( passwordCrypted.isNull() )
		d->passwordFromKConfig.clear();
	else
		d->passwordFromKConfig = KStringHandler::obscure( passwordCrypted );

	d->remembered = config.readEntry( "RememberPassword", false );
	d->isWrong = config.readEntry( "PasswordIsWrong", false );
}

void Kopete::Password::writeConfig()
{
	KSharedConfig::Ptr config = KGlobal::config();
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

	KConfigGroup group = config->group( d->configGroup );

	if ( d->remembered && !d->passwordFromKConfig.isNull() )
		group.writeEntry( "Password", KStringHandler::obscure( d->passwordFromKConfig ) );
	else
		group.deleteEntry( "Password" );

	group.writeEntry( "RememberPassword", d->remembered );
	group.writeEntry( "PasswordIsWrong", d->isWrong );
}

int Kopete::Password::preferredImageSize()
{
	return IconSize(KIconLoader::Toolbar);
}

bool Kopete::Password::allowBlankPassword()
{
	return d->allowBlankPassword;
}

bool Kopete::Password::isWrong()
{
	return d->isWrong;
}

void Kopete::Password::setWrong( bool bWrong )
{
	d->isWrong = bWrong;
	writeConfig();

	if ( bWrong ) d->cachedValue.clear();
}

void Kopete::Password::requestWithoutPrompt( QObject *returnObj, const char *slot )
{
	KopetePasswordRequest *request = new KopetePasswordGetRequestNoPrompt( returnObj, *this );
	// call connect on returnObj so we can still connect if 'slot' is protected/private
	returnObj->connect( request, SIGNAL(requestFinished(QString)), slot );
	request->begin();
}

void Kopete::Password::request( QObject *returnObj, const char *slot, const QPixmap &image, const QString &prompt, Kopete::Password::PasswordSource source )
{
	KopetePasswordRequest *request = new KopetePasswordGetRequestPrompt( returnObj, *this, image, prompt, source );
	returnObj->connect( request, SIGNAL(requestFinished(QString)), slot );
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
