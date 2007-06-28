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
#include "kopeteprotocol.h"

#include <QStringList>
#include <KDebug>
#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfigPtr>
#include <KLocale>

#include <kdeversion.h>

namespace Kopete 
{

class Identity::Private
{
public:
	Private(const QString &i)
	{
		configGroup = new KConfigGroup(KGlobal::config(), QString::fromLatin1( "Identity_%1" ).arg( id ));
		id = i;
	}
	QList<Kopete::Account*> accounts;
	QString id;
	KConfigGroup *configGroup;
	OnlineStatus::StatusType onlineStatus;
};

Identity::Identity(const QString &id)
{
	d = new Private(id);
}

Identity::Identity(const QString &id, Identity &existing)
{
	d = new Private(id);

	QMap<QString,QString> props;
	
	//read properties from the existing identity
	existing.serializeProperties(props);

	//write them in this identity
	deserializeProperties(props);	
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

OnlineStatus::StatusType Identity::onlineStatus() const
{
	return d->onlineStatus;
}

QString Identity::toolTip() const
{

	QString tt = QLatin1String("<qt>");

	QString nick;
	if ( hasProperty(Kopete::Global::Properties::self()->nickName().key()) )
		nick = getProperty(Kopete::Global::Properties::self()->nickName()).value().toString();
	else
		nick = d->id;

	tt+= i18nc( "Identity tooltip information: <nobr>ICON <b>NAME</b><br/></br>",
				"<nobr><img src=\"kopete-identity-icon:%1\"> <b>%2</b><br/<br/>",
				QString(QUrl::toPercentEncoding( d->id )), nick );

	foreach(Account *a, d->accounts)
	{
		Kopete::Contact *self = a->myself();
		tt += i18nc( "Account tooltip information: <nobr>ICON <b>PROTOCOL:</b> NAME (<i>STATUS</i>)<br/>",
					 "<nobr><img src=\"kopete-account-icon:%3:%4\"> <b>%1:</b> %2 (<i>%5</i>)<br/>",
					 a->protocol()->displayName(), a->accountLabel(), QString(QUrl::toPercentEncoding( a->protocol()->pluginId() )),
					 QString(QUrl::toPercentEncoding( a->accountId() )), self->onlineStatus().description() );
	}
	tt += QLatin1String("</qt>");
	return tt;
}

QString Identity::customIcon() const
{
	//TODO implement
	return "identity";
}


KActionMenu* Identity::actionMenu()
{
	//TODO check what layout we want to have for this menu
	// for now if there is only on account, return the account menu
	if (d->accounts.count() == 1)
		return d->accounts.first()->actionMenu();

	// if there is more than one account, add them as submenus of this identity menu
	KActionMenu *menu = new KActionMenu(d->id, this);

	foreach(Account *account, d->accounts)
	{
		KActionMenu *accountMenu = account->actionMenu();
		accountMenu->setIcon( account->myself()->onlineStatus().iconFor(account->myself()) );
		menu->addAction( accountMenu );
	}
	return menu;
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
	Kopete::OnlineStatus::StatusType newStatus = Kopete::OnlineStatus::Unknown; 

	foreach(Account *account, d->accounts)
	{
		// find most significant status
		if ( account->myself()->onlineStatus() > mostSignificantStatus )
			mostSignificantStatus = account->myself()->onlineStatus();
	}

	newStatus = mostSignificantStatus.status();
	if( newStatus != d->onlineStatus )
	{
		emit onlineStatusChanged( this, d->onlineStatus, newStatus );
		d->onlineStatus = newStatus;
	}
}

} //END namespace Kopete

#include "kopeteidentity.moc"

// vim: set noet ts=4 sts=4 sw=4:
