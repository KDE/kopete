/*
    kopeteidentitymanager.cpp - Kopete Identity Manager

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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


namespace Kopete {

class IdentityManager::Private
{
public:
	Identity::List identities;
};

IdentityManager * IdentityManager::s_self = 0L;

IdentityManager * IdentityManager::self()
{
	if ( !s_self )
		s_self = new IdentityManager;

	return s_self;
}


IdentityManager::IdentityManager()
: QObject( qApp )
{
	setObjectName( "KopeteIdentityManager" );
	d = new Private;
}


IdentityManager::~IdentityManager()
{
	s_self = 0L;

	delete d;
}

void IdentityManager::setOnlineStatus( uint category , const QString& awayMessage, uint flags )
{
	Q_UNUSED(flags);
	foreach( Identity *identity ,  d->identities )
	{
		if ( !identity->excludeConnect() )
				identity->setOnlineStatus( category , awayMessage );
	}
}

Identity* IdentityManager::registerIdentity( Identity *identity )
{
	if( !identity || d->identities.contains( identity ) )
		return identity;

	if( identity->identityId().isEmpty() )
	{
		kDebug() << k_funcinfo << "OOps, this shouldn't happen: the identityId should not be empty" << endl;
		identity->deleteLater();
		return 0L;
	}

	// If this identity already exists, do nothing
	foreach( Identity *currident ,  d->identities )
	{
		if ( identity->identityId() == currident->identityId() )
		{
			identity->deleteLater();
			return 0L;
		}
	}

	d->identities.append( identity );

	// Connect to the identity's status changed signal
	connect(identity, SIGNAL(onlineStatusChanged(Kopete::Identity *,
			const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)),
		this, SLOT(slotIdentityOnlineStatusChanged(Kopete::Identity *,
			const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)));

	connect(identity, SIGNAL(identityDestroyed(const Kopete::Identity *)) , this, SLOT( unregisterIdentity(const Kopete::Identity *) ));

	return identity;
}

void IdentityManager::unregisterIdentity( const Identity *identity )
{
	kDebug( 14010 ) << k_funcinfo << "Unregistering identity " << identity->identityId() << endl;
	d->identities.removeAll( const_cast<Identity*>(identity) );
}

const Identity::List& IdentityManager::identities() const
{
	return d->identities;
}

Identity * IdentityManager::findIdentity( const QString &identityId )
{
	foreach( Identity *identity , d->identities )	
	{
		if ( identity->identityId() == identityId )
			return identity;
	}
	return 0L;
}

void IdentityManager::removeIdentity( Identity *identity )
{
	KConfigGroup *configgroup = identity->configGroup();

	// Clean up the identity list
	d->identities.removeAll( identity );

	// Clean up configuration
	configgroup->deleteGroup();
	configgroup->sync();

	delete identity;
}

void IdentityManager::save()
{
	//kDebug( 14010 ) << k_funcinfo << endl;
	foreach( Identity *identity , d->identities )
	{
		KConfigGroup *config = identity->configGroup();

		config->writeEntry( "IdentityId", identity->identityId() );
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
		
		QString identityId = cg.readEntry( "IdentityId" );

		Identity *identity = registerIdentity( new Identity( identityId ) );
		if ( !identity )
		{
			kWarning( 14010 ) << k_funcinfo <<
								 "Failed to create identity for '" << identityId << "'" << endl;
			continue;
		}
		kDebug() << "Created identity " << identityId << endl;
	}
}

void IdentityManager::slotIdentityOnlineStatusChanged(Identity *i,
	const OnlineStatus &oldStatus, const OnlineStatus &newStatus)
{
	//TODO: check if we need to do something more on status changes
	//kDebug(14010) << k_funcinfo << endl;
	emit identityOnlineStatusChanged(i, oldStatus, newStatus);
}

} //END namespace Kopete

#include "kopeteidentitymanager.moc"
// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-mode csands;
