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

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qapplication.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <qtimer.h>

#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetemetacontact.h"
#include "kopetepassworddialog.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"

/*
 * Function for (en/de)crypting strings for config file, taken from KMail
 * Author: Stefan Taferner <taferner@alpin.or.at>
 */
QString cryptStr(const QString &aStr)
{
	QString result;
	for (unsigned int i = 0; i < aStr.length(); i++)
		result += (aStr[i].unicode() < 0x20) ? aStr[i] :
			QChar(0x1001F - aStr[i].unicode());
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
};

KopeteAccount::KopeteAccount(KopeteProtocol *parent, const QString& _accountId , const char *name):  KopetePluginDataObject (parent, name)
{
	d=new KopeteAccountPrivate;
	d->protocol=parent;
	d->id=_accountId;
	d->autologin=false;
	d->password=QString::null;
	d->color = QColor();

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

	delete d;
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
	if(d->id!=accountId)
	{
		d->id = accountId;
		emit ( accountIdChanged() );
	}
}

void KopeteAccount::writeConfig( const QString &configGroup )
{
	KConfig *config = KGlobal::config();
	config->setGroup( configGroup );

	config->writeEntry( "Protocol", d->protocol->pluginId() );
	config->writeEntry( "AccountId", d->id );

	if( !d->password.isNull() )
		config->writeEntry( "Password", cryptStr( d->password ) );

	config->writeEntry( "AutoConnect", d->autologin );

	if( d->color.isValid() )
		config->writeEntry( "Color", d->color );

	// Store other plugin data
	KopetePluginDataObject::writeConfig( configGroup );
}

void KopeteAccount::readConfig( const QString &configGroup )
{
	KConfig *config = KGlobal::config();
	config->setGroup( configGroup );

	d->password  = cryptStr( config->readEntry( "Password" ) );
	d->autologin = config->readBoolEntry( "AutoConnect", false );
	d->color     = config->readColorEntry( "Color", &d->color );


	// Handle the plugin data, if any
	QMap<QString, QString> entries = config->entryMap( configGroup );
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
		setPluginData( LibraryLoader::pluginLoader()->searchByID( pluginDataIt.key() ), pluginDataIt.data() );

	loaded();
}

void KopeteAccount::loaded()
{
	/* do nothing in default implementation */
}

QString KopeteAccount::getPassword( bool error, bool *ok )
{
	if(ok) *ok=true;
	if(!d->password.isNull())
	{
		if(!error)
		{
			return d->password;
		}
		else
		{	//if the cached password was wrong, we remove it
			d->password=QString::null;
		}
	}

	KDialogBase *passwdDialog= new KDialogBase( qApp->mainWidget() ,"passwdDialog", true, i18n( "Password Needed" ),
				KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );

	KopetePasswordDialog *view = new KopetePasswordDialog(passwdDialog);
	passwdDialog->setMainWidget(view);

	if(error)
		view->m_text->setText(i18n("<b>The password was wrong! Please re-enter your password for %1</b>").arg(protocol()->displayName()));
	else
		view->m_text->setText(i18n("Please enter password for %1").arg(protocol()->displayName()));

	view->m_login->setText(d->id);
	view->m_autologin->setChecked( d->autologin );

	QString pass=QString::null;

	if(passwdDialog->exec() == QDialog::Accepted )
	{
		pass=view->m_password->text();
		if(view->m_save_passwd->isChecked())
			setPassword(pass);
		d->autologin=view->m_autologin->isChecked();
	}
	else
	{
		if(ok) *ok=false;
	}
	passwdDialog->deleteLater();
	return pass;
}

void KopeteAccount::setPassword(const QString& pass)
{
	d->password=pass;
	emit( passwordChanged() );
}

void KopeteAccount::setAutoLogin(bool b)
{
	d->autologin=b;
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
//	kdDebug(14010) << "KopeteProtocol::slotKopeteContactDestroyed: " << c->contactId() << endl;
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
		if( ( *it )->metaContact() == mc )
			result.insert( ( *it )->contactId(), *it );
	}
	return result;
}*/


bool KopeteAccount::addContact( const QString &contactId, const QString &displayName,
	KopeteMetaContact *parentContact, const QString &groupName, bool isTemporary )
{
	if(contactId==accountId())
	{
		kdDebug(14010) << "KopeteAccount::addContact: WARNING: the user try to add myself to his contactlist - abort" << endl;
		return false;
	}
	KopeteContact *c=d->contacts[contactId];
	if(c && c->metaContact())
	{
		if(c->metaContact()->isTemporary() && !isTemporary)
		{
			kdDebug(14010) << "KopeteAccount::addContact: You are triying to add an existing temporary contact. Just add it on the list" << endl;
			parentContact->addToGroup( KopeteContactList::contactList()->getGroup( groupName ) );
		}
		else // should we here add the contact to the parentContact if any?
			kdDebug(14010) << "KopeteAccount::addContact: Contact already exist" << endl;
		return false;
	}

	KopeteGroup *parentGroup=0L;
	//If this is a temporary contact, use the temporary group
	if(!groupName.isNull())
		parentGroup = isTemporary ? KopeteGroup::temporary : KopeteContactList::contactList()->getGroup( groupName );

	if( parentContact )
	{
		//If we are given a MetaContact to add to that is marked as temporary. but
		//this contact is not temporary, then change the metacontact to non-temporary
		if( parentContact->isTemporary() && !isTemporary )
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
		if( isTemporary )
			parentContact->setTemporary(true);
		else
			parentContact->addToGroup( parentGroup );

		KopeteContactList::contactList()->addMetaContact( parentContact );
	}

	if( c )
	{
		c->setMetaContact(parentContact);
		return true;
	}
	else
		return addContactToMetaContact( contactId, displayName, parentContact );
}

KActionMenu* KopeteAccount::actionMenu()
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

