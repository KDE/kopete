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
};

Identity::Identity(const QString &id)
{
	d = new Private;
	d->id = id;

	d->configGroup = new KConfigGroup(KGlobal::config(), QString::fromLatin1( "Identity_%1" ).arg( id ));
}

Identity::~Identity()
{
	delete d;
}

QString Identity::identityId() const
{
	return d->id;
}

void Identity::setProperty(const Kopete::ContactPropertyTmpl &tmpl, const QVariant &value)
{
	PropertyContainer::setProperty( tmpl, value );
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

} //END namespace Kopete

#include "kopeteidentity.moc"

