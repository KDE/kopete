/*
    upnpRouter.cpp -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "upnpRouter.h"


QList<UPnpRouter> UPnpRouter::allRouters()
{
	QList<UPnpRouter> routers;
	UPnp *d = UPnp::upnp();
	d->searchDevices();
	foreach (QUrl url, d->devicesSettingUrl())
	{
		qDebug()<<"rentre";
		UPnpRouter upnpRouter = UPnpRouter(url);
		routers.push_back(upnpRouter);
	}

	return routers;
}

UPnpRouter UPnpRouter::defaultRouter()
{
	UPnp *u = UPnp::upnp();
	UPnpRouter router;
	u->searchDevices();
	if(u->isValid())
	{
		router = UPnpRouter(u->devicesSettingUrl().first());
	}
	return router;
}

UPnpRouter::UPnpRouter(const QUrl &url)
{
	d = UpnpRouterPrivate(url);
}

UPnpRouter::UPnpRouter()
{
	d = UpnpRouterPrivate();
}

//TODO constructor by copy
UPnpRouter::UPnpRouter(const UPnpRouter &router)
{
	d = router.d;
}

//TODO 
UPnpRouter &UPnpRouter::operator=(const UPnpRouter &router)
{
	d = router.d;
	return *this;
}

bool UPnpRouter::isValid() const
{		
	return d.isValid();
}

QString UPnpRouter::routerDescription() const
{
	return d.routerDescription();
}

QUrl UPnpRouter::url() const
{
	return d.router->routerSettingUrl();
}

bool UPnpRouter::openPort(quint16 port, const QString &typeProtocol, const QString &protocol)
{
	bool val = false;
	if(d.openPort(port, typeProtocol, protocol))
	{
		qDebug()<< "true";
		val = true;
	}
	return val;
}

bool UPnpRouter::closePort(quint16 port, const QString &typeProtocol)
{
	return d.closePort(port, typeProtocol);
}


