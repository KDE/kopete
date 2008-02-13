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


//TODO Possible Evolution: many routers
// QList<UPnpRouter> UPnpRouter::allRouters()
// {
// 	QList<UPnpRouter> routers;
// 	QList<UpnpRouterPrivate> listRouters = UpnpRouterPrivate::listRouterPrivate();
// 	foreach (UpnpRouterPrivate rt, listRouters)
// 	{
// 		QUrl urlsetting = rt.router->routerSettingUrl();	
// // 		UPnpRouter upnpRouter = UPnpRouter(urlsetting);
// // 		routers.push_back(upnpRouter);
// 	}
// 	return routers;
// }


UPnpRouter UPnpRouter::defaultRouter()
{
	UpnpRouterPrivate routerP = UpnpRouterPrivate::defaultRouter();
	UPnpRouter router = UPnpRouter(routerP.router->routerSettingUrl());
	return router;
}

UPnpRouter::UPnpRouter(const QUrl &url)
{
	d = UpnpRouterPrivate::upnpRouterPrivate(url);
}

UPnpRouter::UPnpRouter(){}

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

// QUrl UPnpRouter::url() const
// {
// 	return d.routerSettingsUrl;
// }
// void UPnpRouter::setUrl(const QUrl &url)
// {
// 	d.routerSettingsUrl = url;
// }

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


