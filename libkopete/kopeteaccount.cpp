/*
    kopeteaccount.cpp - Kopete Identity

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

struct KopeteIdentityPrivate
{
	KopeteProtocol *protocol;
	QString id;
	QString password;
	bool autologin;
	QMap<QString, QMap<QString, QString> > pluginData;
	QDict<KopeteContact> contacts;
};

KopeteIdentity::KopeteIdentity(KopeteProtocol *parent, const QString& _identityId , const char *name):   QObject(parent, name)
{
	d=new KopeteIdentityPrivate;
	d->protocol=parent;
	d->id=_identityId;
	d->autologin=false;
	d->password=QString::null;
	
	//we have to delete the identity before the custom protocol.
	//because contact are deleted when the identity is deleted
	QObject::connect( parent , SIGNAL(unloading() ) , this , SLOT(deleteLater())); 
	KopeteIdentityManager::manager()->registerIdentity(this);
	
	//the prococol need to acess to myself, which is create later, in the customIdentity constructor
	QTimer::singleShot( 0, parent, SLOT( slotIdentityAdded() ) );
}

KopeteIdentity::~KopeteIdentity()
{
	emit identityDestroyed(this);
	delete d;
}

KopeteProtocol *KopeteIdentity::protocol() const
{
	return d->protocol;
}

QString KopeteIdentity::identityId()
{
	return d->id;
}

void KopeteIdentity::setIdentityId( const QString &identityId )
{
	d->id = identityId;
	emit ( identityIdChanged() );
}

QString KopeteIdentity::toXML()
{
	QString xml = QString::fromLatin1( "  <identity identity-id=\"" ) + QStyleSheet::escape(d->id) + 
			QString::fromLatin1( "\" protocol-id=\"" )  + QStyleSheet::escape(QString::fromLatin1(d->protocol->pluginId())) + QString::fromLatin1( "\">\n" );

	if( !d->password.isNull())
	{
		xml += QString::fromLatin1( "    <password>" ) + QStyleSheet::escape( cryptStr(d->password) ) + QString::fromLatin1( "</password>\n" );
	}
	
	if( d->autologin )
		xml += QString::fromLatin1("    <autologin/>\n");
	
	
	// Store other plugin data
	if( !d->pluginData.isEmpty() )
	{
		QMap<QString, QMap<QString, QString> >::ConstIterator pluginIt;
		for( pluginIt = d->pluginData.begin(); pluginIt != d->pluginData.end(); ++pluginIt )
		{
			xml += QString::fromLatin1( "    <plugin-data plugin-id=\"" ) + QStyleSheet::escape( pluginIt.key() ) + QString::fromLatin1( "\">\n" );

			QMap<QString, QString>::ConstIterator it;
			for( it = pluginIt.data().begin(); it != pluginIt.data().end(); ++it )
			{
				if(!it.key().isNull())
					xml += QString::fromLatin1( "      <plugin-data-field key=\"" ) + QStyleSheet::escape( it.key() ) + QString::fromLatin1( "\">" )
						+ QStyleSheet::escape( it.data() ) + QString::fromLatin1( "</plugin-data-field>\n" );
			}

			xml += QString::fromLatin1( "    </plugin-data>\n" );
		}
	}

	xml += QString::fromLatin1( "  </identity>\n" );

	return xml;

}

bool KopeteIdentity::fromXML(const QDomNode& cnode)
{
	QDomNode identityNode = cnode;
	while( !identityNode.isNull() )
	{
		QDomElement identityElement = identityNode.toElement();
		if( !identityElement.isNull() )
		{
			if( identityElement.tagName() == QString::fromLatin1( "password" ) )
			{
				d->password= cryptStr(identityElement.text());
			}
			else if( identityElement.tagName() == QString::fromLatin1( "autologin" ) )
			{
				d->autologin=true;
			}
			else if( identityElement.tagName() == QString::fromLatin1( "plugin-data" ) )
			{
				QMap<QString, QString> pluginData;
				QString pluginId = identityElement.attribute( QString::fromLatin1( "plugin-id" ), QString::null );

				QDomNode field = identityElement.firstChild();
				while( !field.isNull() )
				{
					QDomElement fieldElement = field.toElement();
					if( fieldElement.tagName() == QString::fromLatin1( "plugin-data-field" ) )
					{
						pluginData.insert( fieldElement.attribute( QString::fromLatin1( "key" ),
							QString::fromLatin1( "undefined-key" ) ), fieldElement.text() );
					}

					field = field.nextSibling();
				}

				d->pluginData.insert( pluginId, pluginData );
			}
			else
			{
				kdDebug(14010) << "KopeteIdentity::fromXML: unknown tag " << identityElement.tagName() <<  endl;
			}
		}
		identityNode = identityNode.nextSibling();
	}
	loaded();
	return true;
}


void KopeteIdentity::loaded()
{
	//do nothing in default implementation
}



QString KopeteIdentity::getPassword( bool error, bool *ok )
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

void KopeteIdentity::setPassword(const QString& pass)
{
	d->password=pass;
	emit( passwordChanged() );
}

void KopeteIdentity::setAutoLogin(bool b)
{
	d->autologin=b;
}
bool KopeteIdentity::autoLogin()
{
	return d->autologin;
}
bool KopeteIdentity::rememberPassword()
{
	return !d->password.isNull();
}


void KopeteIdentity::setPluginData( KopetePlugin *p, const QString &key, const QString &value )
{
	d->pluginData[ QString::fromLatin1( p->pluginId() ) ][ key ] = value;
}

QString KopeteIdentity::pluginData( KopetePlugin *p, const QString &key ) const
{
	if( !d->pluginData.contains( QString::fromLatin1( p->pluginId() ) ) || !d->pluginData[ QString::fromLatin1( p->pluginId() ) ].contains( key ) )
		return QString::null;

	return d->pluginData[ QString::fromLatin1( p->pluginId() ) ][ key ];
}

void KopeteIdentity::registerContact( KopeteContact *c )
{
	d->contacts.insert( c->contactId(), c );
	QObject::connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
		SLOT( slotKopeteContactDestroyed( KopeteContact * ) ) );
	protocol()->registerContact(c);
}

void KopeteIdentity::slotKopeteContactDestroyed( KopeteContact *c )
{
//	kdDebug(14010) << "KopeteProtocol::slotKopeteContactDestroyed: " << c->contactId() << endl;
	d->contacts.remove( c->contactId() );
}

const QDict<KopeteContact>& KopeteIdentity::contacts()
{
	return d->contacts;
}

/*QDict<KopeteContact> KopeteIdentity::contacts( KopeteMetaContact *mc )
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


bool KopeteIdentity::addContact( const QString &contactId, const QString &displayName,
	KopeteMetaContact *parentContact, const QString &groupName, bool isTemporary )
{
	KopeteContact *c=d->contacts[contactId];
	if(c && c->metaContact())
	{
		if(c->metaContact()->isTemporary() && !isTemporary)
		{
			kdDebug(14010) << "KopeteIdentity::addContact: You are triying to add an existing temporary contact. Just add it on the list" << endl;
			parentContact->addToGroup( KopeteContactList::contactList()->getGroup( groupName ) );
		}
		else // should we here add the contact to the parentContact if any?
			kdDebug(14010) << "KopeteIdentity::addContact: Contact already exist" << endl;
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

bool KopeteIdentity::addContactToMetaContact( const QString &, const QString &, KopeteMetaContact *)
{
	kdDebug(14010) << "KopeteIdentity::addContactToMetaContact() Not Implemented!!!" << endl;
	return false;
}

KActionMenu* KopeteIdentity::actionMenu()
{
	//default implementation
	return 0L;
}

bool KopeteIdentity::isConnected() const
{
    return myself()->onlineStatus().status() != KopeteOnlineStatus::Offline;
}

bool KopeteIdentity::isAway() const
{
    return myself()->onlineStatus().status() == KopeteOnlineStatus::Away;
}

#include "kopeteaccount.moc"
