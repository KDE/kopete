/*
    kopeteaccount.cpp - Kopete Account

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qapplication.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qfile.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kdialogbase.h>
#include <klocale.h>

#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetemetacontact.h"
#include "kopetepassworddialog.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"

#if KDE_IS_VERSION( 3, 1, 90 )
// KMessageBox is only used in the KWallet code path
#include <kmessagebox.h>
#include <kwallet.h>
#endif

/*
 * Function for (en/de)crypting strings for config file, taken from KMail
 * Author: Stefan Taferner <taferner@alpin.or.at>
 */
QString cryptStr( const QString &aStr )
{
	QString result;
	for ( uint i = 0; i < aStr.length(); i++ )
		result += ( aStr[ i ].unicode() < 0x20) ? aStr[ i ] : QChar( 0x1001F - aStr[ i ].unicode() );

	return result;
}

struct KopeteAccountPrivate
{
	KopeteProtocol *protocol;
	QString id;
	QString password;
	bool autologin;
	QDict<KopeteContact> contacts;
	QColor color;
	QString stylesheet;
	QString styleContents;
#if KDE_IS_VERSION( 3, 1, 90 )
	KWallet::Wallet *wallet;
#endif
};

KopeteAccount::KopeteAccount( KopeteProtocol *parent, const QString &accountId, const char *name )
: KopetePluginDataObject( parent, name )
{
	d = new KopeteAccountPrivate;
	d->protocol = parent;
	d->id = accountId;
	d->autologin = false;

#if KDE_IS_VERSION( 3, 1, 90 )
	d->wallet = 0L;
#endif

	KopeteAccountManager::manager()->registerAccount( this );
	QTimer::singleShot( 0, this, SLOT( slotAccountReady() ) );
}

KopeteAccount::~KopeteAccount()
{
	// Delete all registered child contacts first
	while ( !d->contacts.isEmpty() )
		delete *QDictIterator<KopeteContact>( d->contacts );
	KopeteAccountManager::manager()->unregisterAccount( this );

	// Let the protocol know that one of its accounts
	// is no longer there
	d->protocol->refreshAccounts();

#if KDE_IS_VERSION( 3, 1, 90 )
	delete d->wallet;
#endif

	delete d;
}

void KopeteAccount::setStylesheet( const QString &stylesheet )
{
	d->stylesheet = stylesheet;
	if( !( stylesheet.isNull() || stylesheet.isEmpty() ) )
	{
		QFile file( stylesheet );
		if ( file.open( IO_ReadOnly ) )
		{
			QTextStream stream( &file );
			d->styleContents = stream.read();
			file.close();
		}
	}
	
	emit( accountStyleChanged() );
}

const QString KopeteAccount::styleSheet() const
{
	return d->stylesheet;
}

const QString KopeteAccount::styleContents() const
{
	return d->styleContents;
}

void KopeteAccount::slotAccountReady()
{
	KopeteAccountManager::manager()->notifyAccountReady( this );
}

KopeteProtocol *KopeteAccount::protocol() const
{
	return d->protocol;
}

QString KopeteAccount::accountId() const
{
	return d->id;
}

const QColor KopeteAccount::color() const
{
	return d->color;
}

void KopeteAccount::setColor( const QColor &color )
{
	d->color = color;
}

void KopeteAccount::setAccountId( const QString &accountId )
{
	if ( d->id != accountId )
	{
		d->id = accountId;
		emit ( accountIdChanged() );
	}
}

QString KopeteAccount::configGroup() const
{
#if QT_VERSION < 0x030200
	return QString::fromLatin1( "Account_%2_%1" ).arg( accountId() ).arg( protocol()->pluginId() );
#else
	return QString::fromLatin1( "Account_%2_%1" ).arg( accountId(), protocol()->pluginId() );
#endif
}

void KopeteAccount::writeConfig( const QString &configGroupName )
{
	KConfig *config = KGlobal::config();
	config->setGroup( configGroupName );

	config->writeEntry( "Protocol", d->protocol->pluginId() );
	config->writeEntry( "AccountId", d->id );

	if ( !d->password.isNull() )
		config->writeEntry( "Password", cryptStr( d->password ) );
	else
		config->deleteEntry( "Password" );

	config->writeEntry( "AutoConnect", d->autologin );

	if ( d->color.isValid() )
		config->writeEntry( "Color", d->color );
	else
		config->deleteEntry( "Color" );
		
	if ( d->stylesheet.isNull() )
		config->deleteEntry( "Stylesheet" );
	else
		config->writeEntry( "Stylesheet", d->stylesheet );

	// Store other plugin data
	KopetePluginDataObject::writeConfig( configGroupName );
}

void KopeteAccount::readConfig( const QString &configGroupName )
{
	KConfig *config = KGlobal::config();
	config->setGroup( configGroupName );

	d->password  = cryptStr( config->readEntry( "Password" ) );
	d->autologin = config->readBoolEntry( "AutoConnect", false );
	d->color     = config->readColorEntry( "Color", &d->color );
	setStylesheet( config->readEntry( "Stylesheet", QString::null ) );

	// Handle the plugin data, if any
	QMap<QString, QString> entries = config->entryMap( configGroupName );
	QMap<QString, QString>::Iterator entryIt;
	QMap<QString, QMap<QString, QString> > pluginData;
	for ( entryIt = entries.begin(); entryIt != entries.end(); ++entryIt )
	{
		if ( entryIt.key().startsWith( QString::fromLatin1( "PluginData_" ) ) )
		{
			QStringList data = QStringList::split( '_', entryIt.key(), true );
			data.pop_front(); // Strip "PluginData_" first
			QString pluginName = data.first();
			data.pop_front();

			// Join remainder and store it
			pluginData[ pluginName ][ data.join( QString::fromLatin1( "_" ) ) ] = entryIt.data();
		}
	}

	// Lastly, pass on the plugin data to the account
	QMap<QString, QMap<QString, QString> >::Iterator pluginDataIt;
	for ( pluginDataIt = pluginData.begin(); pluginDataIt != pluginData.end(); ++pluginDataIt )
	{
		KopetePlugin *plugin = KopetePluginManager::self()->plugin( pluginDataIt.key() );
		if ( plugin )
			setPluginData( plugin, pluginDataIt.data() );
		else
			kdDebug( 14010 ) << k_funcinfo << "No plugin object found for id '" << pluginDataIt.key() << "'" << endl;
	}

	loaded();
}

void KopeteAccount::loaded()
{
	/* do nothing in default implementation */
}

QString KopeteAccount::password( bool error, bool *ok, unsigned int maxLength )
{
	if ( ok )
		*ok = true;

	if ( !error )
	{
#if KDE_IS_VERSION( 3, 1, 90 )
		if ( KWallet::Wallet::folderDoesNotExist( KWallet::Wallet::NetworkWallet(), QString::fromLatin1( "Kopete" ) ) )
		{
			// The folder might exist, try to open the wallet
			if ( !d->wallet )
				d->wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), KWallet::Wallet::Synchronous );
			QString pwd;

			// Before trying to read from the wallet, check if the config file holds a password.
			// If so, remove it from the config and set it through KWallet instead.
			if ( !d->password.isNull() )
			{
				pwd = d->password;
				setPassword( pwd );
				return pwd;
			}

			if ( d->wallet && d->wallet->setFolder( QString::fromLatin1( "Kopete" ) ) &&
				d->wallet->readPassword( configGroup(), pwd ) == 0 && !pwd.isNull() )
			{
				return pwd;
			}
		}
#endif

		if ( !d->password.isNull() )
			return d->password;
	}

	KDialogBase *passwdDialog = new KDialogBase( qApp->mainWidget(), "passwdDialog", true, i18n( "Password Needed" ),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );

	KopetePasswordDialog *view = new KopetePasswordDialog( passwdDialog );

	if ( error )
	{
		view->m_text->setText( i18n( "<b>The password was wrong! Please re-enter your password for %1</b>" ).
			arg( protocol()->displayName() ) );

		// Invalidate any stored pass
		setPassword( QString::null );
	}
	else
	{
		view->m_text->setText( i18n( "Please enter password for %1" ).arg( protocol()->displayName() ) );
	}

	passwdDialog->setMainWidget( view );

	view->m_login->setText( d->id );
	view->m_autologin->setChecked( d->autologin );
	if ( maxLength != 0 )
		view->m_password->setMaxLength( maxLength );

	view->adjustSize();
	passwdDialog->adjustSize();

	QString pass;
	if ( passwdDialog->exec() == QDialog::Accepted )
	{
		pass = view->m_password->text();
		if ( view->m_save_passwd->isChecked() )
			setPassword( pass );
		d->autologin = view->m_autologin->isChecked();
	}
	else
	{
		if ( ok )
            *ok = false;
	}

	passwdDialog->deleteLater();
	return pass;
}

void KopeteAccount::setPassword( const QString &pass )
{
#if KDE_IS_VERSION( 3, 1, 90 )
	kdDebug( 14010 ) << k_funcinfo << endl;

	if ( KWallet::Wallet::isEnabled() )
	{
		if ( !d->wallet )
			d->wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), KWallet::Wallet::Synchronous );

		if ( d->wallet )
		{
			if ( !d->wallet->hasFolder( QString::fromLatin1( "Kopete" ) ) )
				d->wallet->createFolder( QString::fromLatin1( "Kopete" ) );

			if ( d->wallet->setFolder( QString::fromLatin1( "Kopete" ) ) &&
				d->wallet->writePassword( configGroup(), pass ) == 0 )
			{
				// Remove any pass from KConfig if it is still there
				if ( !d->password.isNull() )
				{
					KConfig *config = KGlobal::config();
					config->setGroup( configGroup() );

					config->deleteEntry( "Password" );
					config->sync();

					d->password = QString::null;
				}
				return;
			}
		}

		// If we end up here, the wallet is enabled, but failed somehow.
		// Ask the user what to do now.
		if ( KMessageBox::warningContinueCancel( qApp->mainWidget(),
			i18n( "<qt>Kopete is unable to save your password securely in your wallet!<br>"
			"Do you want to want to save the password in the <b>unsafe</b> configuration file instead?</qt>" ),
			i18n( "Unable to store secure password - Kopete" ),
			KGuiItem( i18n( "Store &Unsafe" ), QString::fromLatin1( "unlock" ) ),
			QString::fromLatin1( "KWalletFallbackToKConfig" ) ) != KMessageBox::Continue )
		{
			return;
		}
	}
#endif

	d->password = pass;
	writeConfig( configGroup() );
}

void KopeteAccount::setAutoLogin( bool b )
{
	d->autologin = b;
}

bool KopeteAccount::autoLogin() const
{
	return d->autologin;
}

bool KopeteAccount::rememberPassword()
{
	return !d->password.isNull();
}

void KopeteAccount::registerContact( KopeteContact *c )
{
	d->contacts.insert( c->contactId(), c );
	QObject::connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
		SLOT( slotKopeteContactDestroyed( KopeteContact * ) ) );
}

void KopeteAccount::slotKopeteContactDestroyed( KopeteContact *c )
{
	//kdDebug( 14010 ) << "KopeteProtocol::slotKopeteContactDestroyed: " << c->contactId() << endl;
	d->contacts.remove( c->contactId() );
}

const QDict<KopeteContact>& KopeteAccount::contacts()
{
	return d->contacts;
}

/*QDict<KopeteContact> KopeteAccount::contacts( KopeteMetaContact *mc )
{
	QDict<KopeteContact> result;

	QDictIterator<KopeteContact> it( d->contacts );
	for ( ; it.current() ; ++it )
	{
		if ( ( *it )->metaContact() == mc )
			result.insert( ( *it )->contactId(), *it );
	}
	return result;
}*/


bool KopeteAccount::addContact( const QString &contactId, const QString &displayName,
	KopeteMetaContact *parentContact, const QString &groupName, bool isTemporary )
{
	if ( contactId == accountId() )
	{
		kdDebug( 14010 ) << "KopeteAccount::addContact: WARNING: the user try to add myself to his contactlist - abort" << endl;
		return false;
	}

	KopeteContact *c = d->contacts[ contactId ];
	if ( c && c->metaContact() )
	{
		if ( c->metaContact()->isTemporary() && !isTemporary )
		{
			kdDebug( 14010 ) << "KopeteAccount::addContact: You are triying to add an existing temporary contact. Just add it on the list" << endl;
			parentContact->addToGroup( KopeteContactList::contactList()->getGroup( groupName ) );
		}
		else
		{
			// should we here add the contact to the parentContact if any?
			kdDebug( 14010 ) << "KopeteAccount::addContact: Contact already exist" << endl;
		}

		return false;
	}

	KopeteGroup *parentGroup = 0L;
	//If this is a temporary contact, use the temporary group
	if ( !groupName.isNull() )
		parentGroup = isTemporary ? KopeteGroup::temporary : KopeteContactList::contactList()->getGroup( groupName );

	if ( parentContact )
	{
		//If we are given a MetaContact to add to that is marked as temporary. but
		//this contact is not temporary, then change the metacontact to non-temporary
		if ( parentContact->isTemporary() && !isTemporary )
			parentContact->setTemporary( false, parentGroup );
		else
			parentContact->addToGroup( parentGroup );
	}
	else
	{
		//Create a new MetaContact
		parentContact = new KopeteMetaContact();
		parentContact->setDisplayName( displayName );

		//Set it as a temporary contact if requested
		if ( isTemporary )
			parentContact->setTemporary( true );
		else
			parentContact->addToGroup( parentGroup );

		KopeteContactList::contactList()->addMetaContact( parentContact );
	}

	if ( c )
	{
		c->setMetaContact( parentContact );
		return true;
	}
	else
	{
		return addContactToMetaContact( contactId, displayName, parentContact );
	}
}

KActionMenu * KopeteAccount::actionMenu()
{
	//default implementation
	return 0L;
}

bool KopeteAccount::isConnected() const
{
	return myself()->onlineStatus().status() != KopeteOnlineStatus::Offline;
}

bool KopeteAccount::isAway() const
{
	return myself()->onlineStatus().status() == KopeteOnlineStatus::Away;
}

#include "kopeteaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

