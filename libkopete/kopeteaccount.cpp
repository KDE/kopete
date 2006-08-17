/*
    kopeteaccount.cpp - Kopete Account

    Copyright (c) 2003-2005 by Olivier Goffart       <ogoffart@ kde.org>
    Copyright (c) 2003-2004 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

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
#include <qtimer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <knotifyclient.h>

#include "kabcpersistence.h"
#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopetegroup.h"
#include "kopeteprefs.h"
#include "kopeteutils.h"
#include "kopeteuiglobal.h"
#include "kopeteblacklister.h"
#include "kopeteonlinestatusmanager.h"
#include "editaccountwidget.h"

namespace Kopete
{


class Account::Private
{
public:
	Private( Protocol *protocol, const QString &accountId )
	 : protocol( protocol ), id( accountId )
	 , excludeconnect( true ), priority( 0 ), myself( 0 )
	 , suppressStatusTimer( 0 ), suppressStatusNotification( false )
	 , blackList( new Kopete::BlackLister( protocol->pluginId(), accountId ) )
	 , connectionTry(0)
	{ }


	~Private() { delete blackList; }

	Protocol *protocol;
	QString id;
	QString accountLabel;
	bool excludeconnect;
	uint priority;
	QDict<Contact> contacts;
	QColor color;
	Contact *myself;
	QTimer suppressStatusTimer;
	bool suppressStatusNotification;
	Kopete::BlackLister *blackList;
	KConfigGroup *configGroup;
	uint connectionTry;
	QString customIcon;
	Kopete::OnlineStatus restoreStatus;
	QString restoreMessage;
};

Account::Account( Protocol *parent, const QString &accountId, const char *name )
 : QObject( parent, name ), d( new Private( parent, accountId ) )
{
	d->configGroup=new KConfigGroup(KGlobal::config(), QString::fromLatin1( "Account_%1_%2" ).arg( d->protocol->pluginId(), d->id ));

	d->excludeconnect = d->configGroup->readBoolEntry( "ExcludeConnect", false );
	d->color = d->configGroup->readColorEntry( "Color", &d->color );
	d->customIcon = d->configGroup->readEntry( "Icon", QString() );
	d->priority = d->configGroup->readNumEntry( "Priority", 0 );

	d->restoreStatus = Kopete::OnlineStatus::Online;
	d->restoreMessage = "";

	QObject::connect( &d->suppressStatusTimer, SIGNAL( timeout() ),
		this, SLOT( slotStopSuppression() ) );
}

Account::~Account()
{
	d->contacts.remove( d->myself->contactId() );
	
	// Delete all registered child contacts first
	while ( !d->contacts.isEmpty() )
		delete *QDictIterator<Contact>( d->contacts );

	kdDebug( 14010 ) << k_funcinfo << " account '" << d->id << "' about to emit accountDestroyed " << endl;
	emit accountDestroyed(this);

	delete d->myself;
	delete d->configGroup;
	delete d;
}

void Account::reconnect()
{
	kdDebug( 14010 ) << k_funcinfo << "account " << d->id << " restoreStatus " << d->restoreStatus.status() << " restoreMessage " << d->restoreMessage << endl;
	setOnlineStatus( d->restoreStatus, d->restoreMessage );
}

void Account::disconnected( DisconnectReason reason )
{
	kdDebug( 14010 ) << k_funcinfo << reason << endl;
	//reconnect if needed
	if(reason == BadPassword )
	{
		QTimer::singleShot(0, this, SLOT(reconnect()));
	}
	else if ( KopetePrefs::prefs()->reconnectOnDisconnect() == true && reason > Manual )
	{
		d->connectionTry++;
		//use a timer to allow the plugins to clean up after return
		if(d->connectionTry < 3)
			QTimer::singleShot(10000, this, SLOT(reconnect())); // wait 10 seconds before reconnect
	}
	if(reason== OtherClient)
	{
		Kopete::Utils::notifyConnectionLost(this, i18n("You have been disconnected"), i18n( "You have connected from another client or computer to the account '%1'" ).arg(d->id), i18n("Most proprietary Instant Messaging services do not allow you to connect from more than one location. Check that nobody is using your account without your permission. If you need a service that supports connection from various locations at the same time, use the Jabber protocol."));
	}
}

Protocol *Account::protocol() const
{
	return d->protocol;
}

QString Account::accountId() const
{
	return d->id;
}

const QColor Account::color() const
{
	return d->color;
}

void Account::setColor( const QColor &color )
{
	d->color = color;
	if ( d->color.isValid() )
		d->configGroup->writeEntry( "Color", d->color );
	else
		d->configGroup->deleteEntry( "Color" );
	emit colorChanged( color );
}

void Account::setPriority( uint priority )
{
 	d->priority = priority;
	d->configGroup->writeEntry( "Priority", d->priority );
}

uint Account::priority() const
{
	return d->priority;
}


QPixmap Account::accountIcon(const int size) const
{
	QString icon= d->customIcon.isEmpty() ? d->protocol->pluginIcon() : d->customIcon;
	
	// FIXME: this code is duplicated with OnlineStatus, can we merge it somehow?
	QPixmap base = KGlobal::instance()->iconLoader()->loadIcon(
		icon, KIcon::Small, size );

	if ( d->color.isValid() )
	{
		KIconEffect effect;
		base = effect.apply( base, KIconEffect::Colorize, 1, d->color, 0);
	}

	if ( size > 0 && base.width() != size )
	{
		base = QPixmap( base.convertToImage().smoothScale( size, size ) );
	}

	return base;
}

KConfigGroup* Kopete::Account::configGroup() const
{
	return d->configGroup;
}

void Account::setAccountLabel( const QString &label )
{
	d->accountLabel = label;
}

QString Account::accountLabel() const
{
	if( d->accountLabel.isNull() )
		return d->id;
	return d->accountLabel;
}

void Account::setExcludeConnect( bool b )
{
	d->excludeconnect = b;
	d->configGroup->writeEntry( "ExcludeConnect", d->excludeconnect );
}

bool Account::excludeConnect() const
{
	return d->excludeconnect;
}

void Account::registerContact( Contact *c )
{
	d->contacts.insert( c->contactId(), c );
	QObject::connect( c, SIGNAL( contactDestroyed( Kopete::Contact * ) ),
		SLOT( contactDestroyed( Kopete::Contact * ) ) );
}

void Account::contactDestroyed( Contact *c )
{
	d->contacts.remove( c->contactId() );
}


const QDict<Contact>& Account::contacts()
{
	return d->contacts;
}


Kopete::MetaContact* Account::addContact( const QString &contactId, const QString &displayName , Group *group, AddMode mode  )
{

	if ( contactId == d->myself->contactId() )
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error,
			i18n("You are not allowed to add yourself to the contact list. The addition of \"%1\" to account \"%2\" will not take place.").arg(contactId,accountId()), i18n("Error Creating Contact")
		);
		return false;
	}
 
	bool isTemporary = mode == Temporary;

	Contact *c = d->contacts[ contactId ];

	if(!group)
		group=Group::topLevel();

	if ( c && c->metaContact() )
	{
		if ( c->metaContact()->isTemporary() && !isTemporary )
		{
			kdDebug( 14010 ) << k_funcinfo <<  " You are trying to add an existing temporary contact. Just add it on the list" << endl;

			c->metaContact()->setTemporary(false, group );
			ContactList::self()->addMetaContact(c->metaContact());
		}
		else
		{
			// should we here add the contact to the parentContact if any?
			kdDebug( 14010 ) << k_funcinfo << "Contact already exists" << endl;
		}
		return c->metaContact();
	}

	MetaContact *parentContact = new MetaContact();
	if(!displayName.isEmpty())
		parentContact->setDisplayName( displayName );

	//Set it as a temporary contact if requested
	if ( isTemporary )
		parentContact->setTemporary( true );
	else
		parentContact->addToGroup( group );

	if ( c )
	{
		c->setMetaContact( parentContact );
		if ( mode == ChangeKABC )
		{
			kdDebug( 14010 ) << k_funcinfo << " changing KABC" << endl;
			KABCPersistence::self()->write( parentContact );
		}
	}
	else
	{
		if ( !createContact( contactId, parentContact ) )
		{
			delete parentContact;
			return 0L;
		}
	}

	ContactList::self()->addMetaContact( parentContact );
	return parentContact;
}

bool Account::addContact(const QString &contactId , MetaContact *parent, AddMode mode )
{
	if ( contactId == myself()->contactId() )
	{
	    	KMessageBox::error( Kopete::UI::Global::mainWidget(),
			i18n("You are not allowed to add yourself to the contact list. The addition of \"%1\" to account \"%2\" will not take place.").arg(contactId,accountId()), i18n("Error Creating Contact")
		);
		return 0L;
	}

	bool isTemporary= parent->isTemporary();
	Contact *c = d->contacts[ contactId ];
	if ( c && c->metaContact() )
	{
		if ( c->metaContact()->isTemporary() && !isTemporary )
		{
			kdDebug( 14010 ) <<
				"Account::addContact: You are trying to add an existing temporary contact. Just add it on the list" << endl;

				//setMetaContact ill take care about the deletion of the old contact
			c->setMetaContact(parent);
			return true;
		}
		else
		{
			// should we here add the contact to the parentContact if any?
			kdDebug( 14010 ) << "Account::addContact: Contact already exists" << endl;
		}
		return false; //(the contact is not in the correct metacontact, so false)
	}

	bool success = createContact(contactId, parent);

	if ( success && mode == ChangeKABC )
	{
		kdDebug( 14010 ) << k_funcinfo << " changing KABC" << endl;
		KABCPersistence::self()->write( parent );
	}

	return success;
}

KActionMenu * Account::actionMenu()
{
	//default implementation
	KActionMenu *menu = new KActionMenu( accountId(), myself()->onlineStatus().iconFor( this ),  this );
	QString nick = myself()->property( Kopete::Global::Properties::self()->nickName()).value().toString();

	menu->popupMenu()->insertTitle( myself()->onlineStatus().iconFor( myself() ),
		nick.isNull() ? accountLabel() : i18n( "%2 <%1>" ).arg( accountLabel(), nick )
	);

	OnlineStatusManager::self()->createAccountStatusActions(this, menu);
	menu->popupMenu()->insertSeparator();
	menu->insert( new KAction ( i18n( "Properties" ),  0, this, SLOT( editAccount() ), menu, "actionAccountProperties" ) );

	return menu;
}


bool Account::isConnected() const
{
	return myself() && myself()->isOnline();
}

bool Account::isAway() const
{
	return d->myself && ( d->myself->onlineStatus().status() == Kopete::OnlineStatus::Away );
}

Contact * Account::myself() const
{
	return d->myself;
}

void Account::setMyself( Contact *myself )
{
	bool wasConnected = isConnected();

	if ( d->myself )
	{
		QObject::disconnect( d->myself, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ),
			this, SLOT( slotOnlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ) );
		QObject::disconnect( d->myself, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
			this, SLOT( slotContactPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) );
	}

	d->myself = myself;

//	d->contacts.remove( myself->contactId() );
	
	QObject::connect( d->myself, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ),
		this, SLOT( slotOnlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ) );
	QObject::connect( d->myself, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
		this, SLOT( slotContactPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) );

	if ( isConnected() != wasConnected )
		emit isConnectedChanged();
}

void Account::slotOnlineStatusChanged( Contact * /* contact */,
	const OnlineStatus &newStatus, const OnlineStatus &oldStatus )
{
	bool wasOffline = !oldStatus.isDefinitelyOnline();
	bool isOffline  = !newStatus.isDefinitelyOnline();

	if ( wasOffline || newStatus.status() == OnlineStatus::Offline )
	{
		// Wait for five seconds until we treat status notifications for contacts
		// as unrelated to our own status change.
		// Five seconds may seem like a long time, but just after your own
		// connection it's basically neglectible, and depending on your own
		// contact list's size, the protocol you are using, your internet
		// connection's speed and your computer's speed you *will* need it.
		d->suppressStatusNotification = true;
		d->suppressStatusTimer.start( 5000, true );
		//the timer is also used to reset the d->connectionTry
	}

	if ( !isOffline )
	{
		d->restoreStatus = newStatus;
		d->restoreMessage = myself()->property( Kopete::Global::Properties::self()->awayMessage() ).value().toString();
//		kdDebug( 14010 ) << k_funcinfo << "account " << d->id << " restoreStatus " << d->restoreStatus.status() << " restoreMessage " << d->restoreMessage << endl;
	}

/*	kdDebug(14010) << k_funcinfo << "account " << d->id << " changed status. was "
	               << Kopete::OnlineStatus::statusTypeToString(oldStatus.status()) << ", is "
	               << Kopete::OnlineStatus::statusTypeToString(newStatus.status()) << endl;*/
	if ( wasOffline != isOffline )
		emit isConnectedChanged();
}

void Account::setAllContactsStatus( const Kopete::OnlineStatus &status )
{
	d->suppressStatusNotification = true;
	d->suppressStatusTimer.start( 5000, true );

	for ( QDictIterator<Contact> it( d->contacts ); it.current(); ++it )
		if ( it.current() != d->myself )
			it.current()->setOnlineStatus( status );
}

void Account::slotContactPropertyChanged( Contact * /* contact */,
	const QString &key, const QVariant &old, const QVariant &newVal )
{
	if ( key == QString::fromLatin1("awayMessage") && old != newVal && isConnected() )
	{
		d->restoreMessage = newVal.toString();
//		kdDebug( 14010 ) << k_funcinfo << "account " << d->id << " restoreMessage " << d->restoreMessage << endl;
	}
}

void Account::slotStopSuppression()
{
	d->suppressStatusNotification = false;
	if(isConnected())
		d->connectionTry=0;
}

bool Account::suppressStatusNotification() const
{
	return d->suppressStatusNotification;
}

bool Account::removeAccount()
{
	//default implementation
	return true;
}


BlackLister* Account::blackLister()
{
	return d->blackList;
}

void Account::block( const QString &contactId )
{
	d->blackList->addContact( contactId );
}

void Account::unblock( const QString &contactId )
{
	d->blackList->removeContact( contactId );
}

bool Account::isBlocked( const QString &contactId )
{
	return d->blackList->isBlocked( contactId );
}

void Account::editAccount(QWidget *parent)
{
	KDialogBase *editDialog = new KDialogBase( parent, "KopeteAccountConfig::editDialog", true,
						   i18n( "Edit Account" ), KDialogBase::Ok | KDialogBase::Cancel,
						   KDialogBase::Ok, true );

	KopeteEditAccountWidget *m_accountWidget = protocol()->createEditAccountWidget( this, editDialog );
	if ( !m_accountWidget )
		return;

	// FIXME: Why the #### is EditAccountWidget not a QWidget?!? This sideways casting
	//        is braindead and error-prone. Looking at MSN the only reason I can see is
	//        because it allows direct subclassing of designer widgets. But what is
	//        wrong with embedding the designer widget in an empty QWidget instead?
	//        Also, if this REALLY has to be a pure class and not a widget, then the
	//        class should at least be renamed to EditAccountIface instead - Martijn
	QWidget *w = dynamic_cast<QWidget *>( m_accountWidget );
	if ( !w )
		return;

	editDialog->setMainWidget( w );
	if ( editDialog->exec() == QDialog::Accepted )
	{
		if( m_accountWidget->validateData() )
			m_accountWidget->apply();
	}

	editDialog->deleteLater();
}

void Account::setPluginData( Plugin* /*plugin*/, const QString &key, const QString &value )
{
	configGroup()->writeEntry(key,value);
}

QString Account::pluginData( Plugin* /*plugin*/, const QString &key ) const
{
	return configGroup()->readEntry(key);
}

void Account::setAway(bool away, const QString& reason)
{
	setOnlineStatus( OnlineStatusManager::self()->onlineStatus(protocol() , away ? OnlineStatusManager::Away : OnlineStatusManager::Online)  , reason );
}

void Account::setCustomIcon( const QString & i)
{
	d->customIcon = i;
	if(!i.isEmpty())
		d->configGroup->writeEntry( "Icon", i );
	else
		d->configGroup->deleteEntry( "Icon" );
	emit colorChanged( color() );
}

QString Account::customIcon()  const
{
	return d->customIcon;
}

void Account::virtual_hook( uint /*id*/, void* /*data*/)
{
}



}

 //END namespace Kopete

#include "kopeteaccount.moc"
