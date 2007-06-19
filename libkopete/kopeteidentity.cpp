/*
    kopeteidentity.cpp - Kopete Identity

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kemail.net>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetepropertycontainer.h"
#include "kopeteidentity.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"

#include <QStringList>
#include <KDebug>
#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfigPtr>

#include <kdeversion.h>

namespace Kopete 
{

class Identity::Private
{
public:
	QList<Kopete::Account*> accounts;
	QString id;
	KConfigGroup *configGroup;
	OnlineStatus onlineStatus;
};

Identity::Identity(const QString &id)
{
	d = new Private;
	d->id = id;

	d->configGroup = new KConfigGroup(KGlobal::config(), QString::fromLatin1( "Identity_%1" ).arg( id ));
}

Identity::~Identity()
{
	emit identityDestroyed(this);

	delete d->configGroup;
	delete d;
}

QString Identity::identityId() const
{
	return d->id;
}

Kopete::UI::InfoPage::List Identity::customInfoPages() const
{
	// TODO implement
	Kopete::UI::InfoPage::List list;
	return list;
}

bool Identity::excludeConnect() const
{
	//TODO implement
	return false;
}

void Identity::setOnlineStatus( uint category, const QString &awayMessage )
{
	OnlineStatusManager::Categories katgor=(OnlineStatusManager::Categories)category;

	foreach( Account *account ,  d->accounts )
	{
		Kopete::OnlineStatus status = OnlineStatusManager::self()->onlineStatus(account->protocol() , katgor);
		if ( !account->excludeConnect() )
			account->setOnlineStatus( status , awayMessage );
	}
}

OnlineStatus Identity::onlineStatus() const
{
	//TODO implement
	return d->onlineStatus;
}

QString Identity::toolTip() const
{
	//TODO implement
	return QString("Identity %1").arg(d->id);
}

QString Identity::customIcon() const
{
	//TODO implement
	return "identity";
}


KActionMenu* Identity::actionMenu()
{
	//TODO implement
	return new KActionMenu(d->id, this);
}

void Identity::addAccount( Kopete::Account *account )
{
	// check if we already have this account
	if ( d->accounts.indexOf( account ) != -1 )
		return;

	d->accounts.append( account );
	//TODO implement the signals for status changes and so
}

void Identity::removeAccount( Kopete::Account *account )
{
	//TODO disconnect signals and so on
	d->accounts.removeAll( account );
}

KConfigGroup *Identity::configGroup() const
{
	return d->configGroup;
}

void Identity::load()
{
	QStringList properties = d->configGroup->readEntry("Properties", QStringList());
	QMap<QString,QString> props;

	foreach(QString prop, properties)
	{
		props[ prop ] = d->configGroup->readEntry( prop, QString() );
	}
}

void Identity::save()
{
	// FIXME: using kconfig for now, but I guess this is going to change
	d->configGroup->writeEntry("Properties", properties());
	
	QMap<QString,QString> props;
	serializeProperties(props);
	QMap<QString,QString>::iterator it;
	for (it = props.begin(); it != props.end();  ++it)
	{
		d->configGroup->writeEntry(it.key(), *it);
	}
}

void Identity::updateOnlineStatus()
{
	Kopete::OnlineStatus mostSignificantStatus;

	foreach(Account *account, d->accounts)
	{
		// find most significant status
		if ( account->myself()->onlineStatus() > mostSignificantStatus )
			mostSignificantStatus = account->myself()->onlineStatus();
	}

	if( mostSignificantStatus != d->onlineStatus )
	{
		emit onlineStatusChanged( this, d->onlineStatus, mostSignificantStatus );
		d->onlineStatus = mostSignificantStatus;
	}
}

} //END namespace Kopete

#include "kopeteidentity.moc"

// vim: set noet ts=4 sts=4 sw=4:
