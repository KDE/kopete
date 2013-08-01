/*
    kopeteaccount.cpp - Kopete Account

    Copyright (c) 2003-2005 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003-2004 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Copyright (c) 2007         Will Stephenson       <wstephenson@kde.org>

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

#include "kopeteaccount.h"

#include <qapplication.h>
#include <QTimer>
#include <QPixmap>
#include <QIcon>
#include <QPointer>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kiconeffect.h>
#include <kicon.h>
#include <kaction.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kactionmenu.h>
#include <kconfiggroup.h>

#include "kopeteidentity.h"
#include "kopeteidentitymanager.h"
#include "kabcpersistence.h"
#include "kopetecontactlist.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopetegroup.h"
#include "kopetebehaviorsettings.h"
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
	 , excludeconnect( true ), priority( 0 )
	 , connectionTry(0), identity( 0 ), myself( 0 )
	 , suppressStatusTimer( 0 ), suppressStatusNotification( false )
	 , blackList( new Kopete::BlackLister( protocol->pluginId(), accountId ) )
	{ }


	~Private() { delete blackList; }

	QPointer <Protocol> protocol;
	QString id;
	QString accountLabel;
	bool excludeconnect;
	uint priority;
	QHash<QString, Contact*> contacts;
	QColor color;
	uint connectionTry;
	QPointer <Identity> identity;
	QPointer <Contact> myself;
	QTimer suppressStatusTimer;
	QTimer reconnectTimer;
	bool reconnectOnNetworkIsOnline;
	bool suppressStatusNotification;
	Kopete::BlackLister *blackList;
	KConfigGroup *configGroup;
	QString customIcon;
	Kopete::OnlineStatus restoreStatus;
	Kopete::StatusMessage restoreMessage;
	QDateTime lastLoginTime;
	bool suspended;
	Kopete::OnlineStatus suspendStatus;
};

Account::Account( Protocol *parent, const QString &accountId )
 : QObject( parent ), d( new Private( parent, accountId ) )
{
	d->configGroup=new KConfigGroup(KGlobal::config(), QString::fromLatin1( "Account_%1_%2" ).arg( d->protocol->pluginId(), d->id ));

	d->excludeconnect = d->configGroup->readEntry( "ExcludeConnect", false );
	d->color = d->configGroup->readEntry( "Color" , QColor() );
	d->customIcon = d->configGroup->readEntry( "Icon", QString() );
	d->priority = d->configGroup->readEntry( "Priority", 0 );

	d->restoreStatus = Kopete::OnlineStatus::Online;
	d->restoreMessage = Kopete::StatusMessage();
	d->reconnectOnNetworkIsOnline = false;

	d->reconnectTimer.setSingleShot( true );
	QObject::connect( &d->reconnectTimer, SIGNAL(timeout()),
	                  this, SLOT(reconnect()) );

	QObject::connect(Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)), this, SLOT(networkingStatusChanged(Solid::Networking::Status)));

	QObject::connect( &d->suppressStatusTimer, SIGNAL(timeout()),
		this, SLOT(slotStopSuppression()) );

	d->suspended = false;
	d->suspendStatus = Kopete::OnlineStatus::Offline;
}

Account::~Account()
{
	// Delete all registered child contacts first
	foreach (Contact* c, d->contacts) QObject::disconnect(c, SIGNAL(contactDestroyed(Kopete::Contact*)), this, 0);
	qDeleteAll(d->contacts);
	d->contacts.clear();

	kDebug( 14010 ) << " account '" << d->id << "' about to emit accountDestroyed ";
	emit accountDestroyed(this);

	delete d->myself;
	delete d->configGroup;
	delete d;
}

void Account::reconnect()
{
	if ( isConnected() )
		return; // Already connected

	kDebug( 14010 ) << "account " << d->id << " restoreStatus " << d->restoreStatus.status()
	                << " restoreTitle " << d->restoreMessage.title()
	                << " restoreMessage " << d->restoreMessage.message();
	setOnlineStatus( d->restoreStatus, d->restoreMessage );
}

void Account::networkingStatusChanged( const Solid::Networking::Status status )
{
	switch(status) {
	case Solid::Networking::Connected:
		if (d->reconnectOnNetworkIsOnline && ! excludeConnect()) {
			reconnect();
		}
		break;

	case Solid::Networking::Unconnected:
	case Solid::Networking::Disconnecting:
		setOnlineStatus( OnlineStatus::Offline );
		if (Kopete::BehaviorSettings::self()->reconnectOnDisconnect()) {
			d->reconnectOnNetworkIsOnline = true;
		}
		break;

	case Solid::Networking::Unknown:
	case Solid::Networking::Connecting:
		break;
	}
}

void Account::disconnected( DisconnectReason reason )
{
	kDebug( 14010 ) << reason;
	//reconnect if needed
	if ( reason == BadPassword )
	{
		d->reconnectTimer.start( 0 );
	}
	else if ( Kopete::BehaviorSettings::self()->reconnectOnDisconnect() == true && reason > Manual )
	{
		if (reason == ConnectionReset) {
			d->reconnectOnNetworkIsOnline = true;
			d->reconnectTimer.stop();
		} else {
			if ( d->reconnectTimer.isActive() )
				return; // In case protocol calls disconnected more than one time on disconnect.

			d->connectionTry++;
			//use a timer to allow the plugins to clean up after return
			if ( d->connectionTry < 3 )
				d->reconnectTimer.start( 10000 ); // wait 10 seconds before reconnect
			else if ( d->connectionTry <= 10 )
				d->reconnectTimer.start( ((2 * (d->connectionTry - 2)) - 1) * 60000 ); // wait 1,3,5...15 minutes => stops after 64 min
		}
	}
	else
	{
		d->reconnectOnNetworkIsOnline = false;
		d->reconnectTimer.stop();
	}

	if ( reason == OtherClient )
	{
		Kopete::Utils::notifyConnectionLost(this, i18n("You have been disconnected"), i18n( "You have connected from another client or computer to the account '%1'" , d->id), i18n("Most proprietary Instant Messaging services do not allow you to connect from more than one location. Check that nobody is using your account without your permission. If you need a service that supports connection from various locations at the same time, use the Jabber protocol."));
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
	QPixmap base = KIconLoader::global()->loadIcon(
		icon, KIconLoader::Small, size );

	if ( d->color.isValid() )
	{
		KIconEffect effect;
		base = effect.apply( base, KIconEffect::Colorize, 1, d->color, 0);
	}

	if ( size > 0 && base.width() != size )
	{
		base.scaled( size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
	}

	return base;
}

QString Account::accountIconPath(const KIconLoader::Group size) const
{
	return KIconLoader::global()->iconPath(d->customIcon.isEmpty() ?
	                                       d->protocol->pluginIcon() :
					       d->customIcon, size);
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

bool Account::registerContact( Contact *c )
{
	Q_ASSERT ( c->metaContact() != Kopete::ContactList::self()->myself() );

	if ( d->contacts.value( c->contactId() ) )
	{
		kWarning(14010) << "Contact already exists!!! accountId: " << c->account() << " contactId: " << c->contactId();
		return false;
	}

	d->contacts.insert( c->contactId(), c );
	QObject::connect( c, SIGNAL(contactDestroyed(Kopete::Contact*)),
		SLOT(contactDestroyed(Kopete::Contact*)) );

	return true;
}

void Account::contactDestroyed( Contact *c )
{
	d->contacts.remove( c->contactId() );
}


const QHash<QString, Contact*>& Account::contacts()
{
	return d->contacts;
}


Kopete::MetaContact* Account::addContact( const QString &contactId, const QString &displayName , Group *group, AddMode mode  )
{

	if ( !protocol()->canAddMyself() && contactId == d->myself->contactId() )
	{
		if ( isConnected() && d->lastLoginTime.secsTo(QDateTime::currentDateTime()) > 30 )
		{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error,
			                               i18n("You are not allowed to add yourself to the contact list. The addition of \"%1\" to account \"%2\" will not take place.", contactId, accountId()), i18n("Error Creating Contact")
			                             );
		}
		else
		{
			kWarning(14010) << "You are not allowed to add yourself to the contact list. The addition of" << contactId
			                << "to account" << accountId() << "will not take place.";
		}
		return 0;
	}

	bool isTemporary = (mode == Temporary);

	Contact *c = d->contacts.value( contactId );

	if(!group)
		group=Group::topLevel();

	if ( c && c->metaContact() )
	{
		if ( c->metaContact()->isTemporary() && !isTemporary )
		{
			kDebug( 14010 ) <<  " You are trying to add an existing temporary contact. Just add it on the list";

			c->metaContact()->setTemporary(false, group );
			ContactList::self()->addMetaContact(c->metaContact());
		}
		else
		{
			// should we here add the contact to the parentContact if any?
			kDebug( 14010 ) << "Contact already exists";
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
			kDebug( 14010 ) << " changing KABC";
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
	if ( !protocol()->canAddMyself() && contactId == myself()->contactId() )
	{
		if ( isConnected() && d->lastLoginTime.secsTo(QDateTime::currentDateTime()) > 30 )
		{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error,
			                               i18n("You are not allowed to add yourself to the contact list. The addition of \"%1\" to account \"%2\" will not take place.", contactId, accountId()), i18n("Error Creating Contact")
			                             );
		}
		else
		{
			kWarning(14010) << "You are not allowed to add yourself to the contact list. The addition of" << contactId
			                << "to account" << accountId() << "will not take place.";
		}
		
		return 0L;
	}

	const bool isTemporary= parent->isTemporary();
	Contact *c = d->contacts.value( contactId );
	if ( c && c->metaContact() )
	{
		if ( c->metaContact()->isTemporary() && !isTemporary )
		{
			kDebug( 14010 ) <<
				"Account::addContact: You are trying to add an existing temporary contact. Just add it on the list" << endl;

				//setMetaContact ill take care about the deletion of the old contact
			c->setMetaContact(parent);
			return true;
		}
		else
		{
			// should we here add the contact to the parentContact if any?
			kDebug( 14010 ) << "Account::addContact: Contact already exists";
		}
		return false; //(the contact is not in the correct metacontact, so false)
	}

	const bool success = createContact(contactId, parent);

	if ( success && mode == ChangeKABC )
	{
		kDebug( 14010 ) << " changing KABC";
		KABCPersistence::self()->write( parent );
	}

	return success;
}

void Account::fillActionMenu( KActionMenu *actionMenu )
{
	//default implementation
// 	KActionMenu *menu = new KActionMenu( QIcon(myself()->onlineStatus().iconFor( this )), accountId(), 0, 0);
#ifdef __GNUC__
#warning No icon shown, we should go away from QPixmap genered icons with overlays.
#endif
	QString nick;
       	if (identity()->hasProperty( Kopete::Global::Properties::self()->nickName().key() ))
		nick = identity()->property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	else
		nick = myself()->displayName();

	// Always add title at the beginning of actionMenu
	QAction *before = actionMenu->menu()->actions().value( 0, 0 );
	actionMenu->menu()->addTitle( myself()->onlineStatus().iconFor( myself() ),
		nick.isNull() ? accountLabel() : i18n( "%2 <%1>", accountLabel(), nick ),
		before
	);

	actionMenu->menu()->addSeparator();

	KAction *propertiesAction = new KAction( i18n("Properties"), actionMenu );
	QObject::connect( propertiesAction, SIGNAL(triggered(bool)), this, SLOT(editAccount()) );
	actionMenu->addAction( propertiesAction );
}

bool Account::hasCustomStatusMenu() const
{
	return false;
}

bool Account::isConnected() const
{
	return myself() && myself()->isOnline();
}

bool Account::isAway() const
{
	//We might want to change the method name here.
	return d->myself && ( d->myself->onlineStatus().status() == Kopete::OnlineStatus::Away ||
					d->myself->onlineStatus().status() == Kopete::OnlineStatus::Busy );
}

bool Account::isBusy() const
{
	return d->myself && ( d->myself->onlineStatus().status() == Kopete::OnlineStatus::Busy );
}

Identity * Account::identity() const
{
	return d->identity;
}

bool Account::setIdentity( Identity *ident )
{
	if ( d->identity == ident )
	{
		return false;
	}

	if (d->identity)
	{
		d->identity->removeAccount( this );
	}

	ident->addAccount( this );
	d->identity = ident;
	d->configGroup->writeEntry("Identity", ident->id());
	return true;
}

Contact * Account::myself() const
{
	return d->myself;
}

void Account::setMyself( Contact *myself )
{
	Q_ASSERT( !d->myself );

	d->myself = myself;

	QObject::connect( d->myself, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
		this, SLOT(slotOnlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );
	QObject::connect( d->myself, SIGNAL(propertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)),
		this, SLOT(slotContactPropertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)) );

	if ( isConnected() )
		emit isConnectedChanged();
}

void Account::slotOnlineStatusChanged( Contact * /* contact */,
	const OnlineStatus &newStatus, const OnlineStatus &oldStatus )
{
	const bool wasOffline = !oldStatus.isDefinitelyOnline();
	const bool isOffline  = !newStatus.isDefinitelyOnline();
	d->suspended = false;

	if ( wasOffline && !isOffline )
		d->lastLoginTime = QDateTime::currentDateTime();

	// If we went offline we have to ensure that all of our contacts
	// are online too. Otherwise the "Last Seen" tooltip won't work
	// properly. See bug 266580.
	if ( !wasOffline && isOffline )
		setAllContactsStatus( Kopete::OnlineStatus::Offline );

	if ( wasOffline || newStatus.status() == OnlineStatus::Offline )
	{
		// Wait for twenty seconds until we treat status notifications for contacts
		// as unrelated to our own status change.
		// Twenty seconds may seem like a long time, but just after your own
		// connection it's basically neglectible, and depending on your own
		// contact list's size, the protocol you are using, your internet
		// connection's speed and your computer's speed you *will* need it.
		d->suppressStatusNotification = true;
		d->suppressStatusTimer.setSingleShot( true );
		d->suppressStatusTimer.start( 20000 );
		//the timer is also used to reset the d->connectionTry
	}

	if ( !isOffline )
	{
		d->restoreStatus = newStatus;
		d->restoreMessage.setTitle( myself()->property( Kopete::Global::Properties::self()->statusTitle() ).value().toString() );
		d->restoreMessage.setMessage( myself()->property( Kopete::Global::Properties::self()->statusMessage() ).value().toString() );
	}

/*	kDebug(14010) << "account " << d->id << " changed status. was "
	               << Kopete::OnlineStatus::statusTypeToString(oldStatus.status()) << ", is "
	               << Kopete::OnlineStatus::statusTypeToString(newStatus.status()) << endl;*/
	if ( wasOffline != isOffline )
		emit isConnectedChanged();
}

bool Account::suspend( const Kopete::StatusMessage &reason )
{
	if ( d->suspended )
		return false;

	d->suspendStatus = myself()->onlineStatus();
	if( myself()->onlineStatus().status() == OnlineStatus::Connecting )
		d->suspendStatus = d->restoreStatus;
	setOnlineStatus( OnlineStatus::Offline, reason );
	d->suspended = true;
	return true;
}

bool Account::resume()
{
	if ( !d->suspended )
		return false;

	if ( d->suspendStatus != Kopete::OnlineStatus::Offline )
		setOnlineStatus( d->suspendStatus, d->restoreMessage, Kopete::Account::None );

	return true;
}

void Account::setAllContactsStatus( const Kopete::OnlineStatus &status )
{
	d->suppressStatusNotification = true;
	d->suppressStatusTimer.setSingleShot( true );
	d->suppressStatusTimer.start( 20000 );

	QHashIterator<QString, Contact*> it(d->contacts);
	for (  ; it.hasNext(); ) {
		it.next();

		Contact *c = it.value();
		if ( c )
			c->setOnlineStatus( status );
	}
}

void Account::slotContactPropertyChanged( PropertyContainer * /* contact */,
	const QString &key, const QVariant &old, const QVariant &newVal )
{
	if ( key == Kopete::Global::Properties::self()->statusTitle().key() && old != newVal && isConnected() )
		d->restoreMessage.setTitle( newVal.toString() );
	else if ( key == Kopete::Global::Properties::self()->statusMessage().key() && old != newVal && isConnected() )
		d->restoreMessage.setMessage( newVal.toString() );
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
	QPointer <KDialog> editDialog = new KDialog( parent );
	editDialog->setCaption( i18n( "Edit Account" ) );
	editDialog->setButtons( KDialog::Ok | KDialog::Apply | KDialog::Cancel );

	KopeteEditAccountWidget *m_accountWidget = protocol()->createEditAccountWidget( this, editDialog );
	if ( !m_accountWidget )
	{
		delete editDialog;
		return;
	}
	// FIXME: Why the #### is EditAccountWidget not a QWidget?!? This sideways casting
	//        is braindead and error-prone. Looking at MSN the only reason I can see is
	//        because it allows direct subclassing of designer widgets. But what is
	//        wrong with embedding the designer widget in an empty QWidget instead?
	//        Also, if this REALLY has to be a pure class and not a widget, then the
	//        class should at least be renamed to EditAccountIface instead - Martijn
	QWidget *w = dynamic_cast<QWidget *>( m_accountWidget );
	if ( !w )
	{
		delete editDialog;
		return;
	}
	editDialog->setMainWidget( w );
	if ( editDialog->exec() == QDialog::Accepted )
	{
		if( editDialog && m_accountWidget->validateData() )
			m_accountWidget->apply();
	}

	delete editDialog;
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

} // END namespace Kopete

#include "kopeteaccount.moc"

