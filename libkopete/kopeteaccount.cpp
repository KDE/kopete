/*
    kopeteaccount.cpp - Kopete Account

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qstylesheet.h>
#include <qdom.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <qtimer.h>

#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopetepassworddialog.h"
#include "kopeteprotocol.h"

/*
 * Function for (en/de)crypting strings for config file, taken from KMail
 * Author: Stefan Taferner <taferner@alpin.or.at>
 */
QString cryptStr(const QString &aStr) {
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
};

KopeteAccount::KopeteAccount(KopeteProtocol *parent, const QString& _accountId , const char *name):  KopetePluginDataObject (parent, name)
{
	d=new KopeteAccountPrivate;
	d->protocol=parent;
	d->id=_accountId;
	d->autologin=false;
	d->password=QString::null;
	
	//we have to delete the account before the custom protocol.
	//because contact are deleted when the account is deleted
	QObject::connect( parent , SIGNAL(unloading() ) , this , SLOT(deleteLater())); 
	KopeteAccountManager::manager()->registerAccount(this);
	
	//the prococol need to acess to myself, which is create later, in the customAccount constructor
	QTimer::singleShot( 0, parent, SLOT( slotAccountAdded() ) );
}

KopeteAccount::~KopeteAccount()
{
	emit accountDestroyed(this);
	delete d;
}

KopeteProtocol *KopeteAccount::protocol() const
{
	return d->protocol;
}

QString KopeteAccount::accountId()
{
	return d->id;
}

void KopeteAccount::setAccountId( const QString &accountId )
{
	d->id = accountId;
	emit ( accountIdChanged() );
}

QString KopeteAccount::toXML()
{
	QString xml = QString::fromLatin1( "  <account account-id=\"" ) + QStyleSheet::escape(d->id) + 
		QString::fromLatin1( "\" protocol-id=\"" )  + QStyleSheet::escape( d->protocol->pluginId() ) + QString::fromLatin1( "\">\n" );

	if( !d->password.isNull())
	{
		xml += QString::fromLatin1( "    <password>" ) + QStyleSheet::escape( cryptStr(d->password) ) + QString::fromLatin1( "</password>\n" );
	}
	
	if( d->autologin )
		xml += QString::fromLatin1("    <autologin/>\n");
	
	
	// Store other plugin data
	xml += KopetePluginDataObject::toXML();

	xml += QString::fromLatin1( "  </account>\n" );

	return xml;

}

bool KopeteAccount::fromXML(const QDomNode& cnode)
{
	QDomNode accountNode = cnode;
	while( !accountNode.isNull() )
	{
		QDomElement accountElement = accountNode.toElement();
		if( !accountElement.isNull() )
		{
			if( accountElement.tagName() == QString::fromLatin1( "password" ) )
			{
				d->password= cryptStr(accountElement.text());
			}
			else if( accountElement.tagName() == QString::fromLatin1( "autologin" ) )
			{
				d->autologin=true;
			}
			else if( accountElement.tagName() == QString::fromLatin1( "plugin-data" ) )
			{
				KopetePluginDataObject::fromXML(accountElement) ;
			}
			else
			{
				kdDebug(14010) << "KopeteAccount::fromXML: unknown tag " << accountElement.tagName() <<  endl;
			}
		}
		accountNode = accountNode.nextSibling();
	}
	loaded();
	return true;
}


void KopeteAccount::loaded()
{
	//do nothing in default implementation
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

	KDialogBase *passwdDialog= new KDialogBase( qApp->mainWidget() ,"passwdDialog", true, i18n( "Password needed" ),
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
bool KopeteAccount::autoLogin()
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
	protocol()->registerContact(c);
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
	
		KopeteContactList::contactList()->addMetaContact( parentContact );;
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
