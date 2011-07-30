/*
    kopeteidentitymanager.cpp - Kopete Identity Manager

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2007         Will Stephenson        <wstephenson@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteidentitymanager.h"
#include "kopeteidentity.h"

#include <QApplication>
#include <KDebug>
#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>
#include <KSharedConfigPtr>
#include <KLocale>


namespace Kopete {

class IdentityManager::Private
{
public:
	Private()
	{
		defaultIdentity = 0;
	}
	Identity::List identities;
	Identity * defaultIdentity;
};

IdentityManager * IdentityManager::s_self = 0L;

IdentityManager * IdentityManager::self()
{
	if ( !s_self )
		s_self = new IdentityManager;

	return s_self;
}


IdentityManager::IdentityManager()
: QObject( qApp ), d(new Private())
{
	setObjectName( "KopeteIdentityManager" );
}


IdentityManager::~IdentityManager()
{
	s_self = 0L;

	delete d;
}

void IdentityManager::setOnlineStatus( uint category , const Kopete::StatusMessage &statusMessage, uint flags )
{
	Q_UNUSED(flags);
	foreach( Identity *identity ,  d->identities )
	{
		if ( !identity->excludeConnect() )
			identity->setOnlineStatus( category , statusMessage );
	}
}

Identity* IdentityManager::registerIdentity( Identity *identity )
{
	if( !identity || d->identities.contains( identity ) )
		return identity;

	// If this identity already exists, do nothing
	foreach( Identity *currident,  d->identities )
	{
		if ( identity->id() == currident->id() )
		{
			identity->deleteLater();
			return 0L;
		}
	}

	d->identities.append( identity );

	// Connect to the identity's status changed signal
	connect(identity, SIGNAL(onlineStatusChanged(Kopete::Identity*)),
		this, SLOT(slotIdentityOnlineStatusChanged(Kopete::Identity*)));

	connect(identity, SIGNAL(identityDestroyed(const Kopete::Identity*)) , this, SLOT(unregisterIdentity(const Kopete::Identity*)));

	emit identityRegistered( identity );

	return identity;
}

void IdentityManager::unregisterIdentity( const Identity *identity )
{
	kDebug( 14010 ) << "Unregistering identity " << identity->id();
	d->identities.removeAll( const_cast<Identity*>(identity) );

	emit identityUnregistered( identity );
}

const Identity::List& IdentityManager::identities() const
{
	return d->identities;
}

Identity *IdentityManager::findIdentity( const QString &id )
{
	foreach( Identity *identity , d->identities )	
	{
		if ( identity->id() == id )
			return identity;
	}
	return 0L;
}

Identity *IdentityManager::defaultIdentity()
{
	Identity *ident = 0;

	if (d->defaultIdentity)
		ident = findIdentity(d->defaultIdentity->id());

	if (ident)
		return ident;

	// if the identity set as the default identity does not exist, try using another one
	
	// if there is no identity registered, create a default identity
	if (!d->defaultIdentity)
	{
		ident = new Identity(i18nc("Label for the default identity, used by users to group their instant messaging accounts", "Default Identity"));
		ident = registerIdentity(ident);
		emit defaultIdentityChanged( ident );
		setDefaultIdentity( ident );
		if (ident)
			return ident;
	}
	else
	{
		// use any identity available
		ident = d->identities.first();
		setDefaultIdentity( ident );
		return ident;
	}
	return 0;
}

void IdentityManager::setDefaultIdentity( Identity *identity )
{
	Q_ASSERT(identity);

	// if the default identity didn't change, just return
	if (identity == d->defaultIdentity)
		return;

	// if the given identity is not registered, does nothing
	if (d->identities.indexOf( identity ) == -1)
		return;

	d->defaultIdentity = identity;
	save();
	emit defaultIdentityChanged( identity );
}

void IdentityManager::removeIdentity( Identity *identity )
{
	KConfigGroup *configgroup = identity->configGroup();

	// Clean up the identity list
	d->identities.removeAll( identity );

	// Clean up configuration
	configgroup->deleteGroup();
	configgroup->sync();
	if (d->defaultIdentity == identity) {
		d->defaultIdentity = 0;
	}
	delete identity;
}

void IdentityManager::save()
{
	// save the default identity
	KConfigGroup group = KGlobal::config()->group("IdentityManager");
	group.writeEntry("DefaultIdentity", d->defaultIdentity->id());

	//kDebug( 14010 );
	foreach( Identity *identity, d->identities )
	{
		KConfigGroup *config = identity->configGroup();

		config->writeEntry( "Id", identity->id() );
		config->writeEntry( "Label", identity->label() );
		identity->save();
	}

	KGlobal::config()->sync();
}

void IdentityManager::load()
{
	// Iterate over all groups that start with "Identity_" as those are identities.
	KSharedConfig::Ptr config = KGlobal::config();

	QStringList identityGroups = config->groupList().filter( QRegExp( QString::fromLatin1( "^Identity_" ) ) );
	for ( QStringList::Iterator it = identityGroups.begin(); it != identityGroups.end(); ++it )
	{
		KConfigGroup cg( config, *it );
		
		QString identityId = cg.readEntry( "Id" );
		QString label = cg.readEntry( "Label" );

		Identity *identity = registerIdentity( new Identity( identityId, label ) );
		if ( !identity )
		{
			kWarning( 14010 ) <<
								 "Failed to create identity for '" << identityId << "'" << endl;
			continue;
		}
		kDebug() << "Created identity " << identityId;
	}

	// get the default identity
	KConfigGroup group = config->group("IdentityManager");
	Identity * storedDefault = findIdentity( group.readEntry("DefaultIdentity", QString()) );
	if ( storedDefault ) {
		d->defaultIdentity = storedDefault;
	}

	// just to make sure the default identity gets created when there is no identity registered
	defaultIdentity();
}

void IdentityManager::slotIdentityOnlineStatusChanged(Identity *i)
{
	//TODO: check if we need to do something more on status changes
	//kDebug(14010);
	emit identityOnlineStatusChanged(i);
}

} //END namespace Kopete

#include "kopeteidentitymanager.moc"
// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-mode csands;
