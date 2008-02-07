#include "upnpRouter.h"

//TODO Possible Evolution: many routers
QList<UPnpRouter> UPnpRouter::allRouters()
{
	d = new upnpRouterPrivate();
	QList<UPnpRouter> routeurs;
	foreach (upnpRouterPrivate rt, d->listRouterPrivate())
	{
		routers.push_back((UPnpRouter)rt);
	}
	return routers;
}


UPnpRouter UPnpRouter::defaultRouter()
{
	//d->UpnpRouterPrivate::routerPrivate();
 	if(d.upnpSearch())
 		d.router();
}

UPnpRouter::UPnpRouter(const QUrl &url)
{
	//d->UpnpRouterPrivate::routerPrivate();
	d->routerSettingsUrl = url;
	if(d->isValid())
		d->router();
}

//TODO constructor by copy
UPnpRouter::UPnpRouter(const UPnpRouter &router):d(router.d)
{
}

//TODO 
UPnpRouter &UPnpRouter::operator=(const UPnpRouter &router)
{
	d = router.d;
	return *this;
}

bool UPnpRouter::isValid() const
{
	return d->isValid();
}

QString UPnpRouter::routerDescription() const
{
	return d->routerDescription();
}

QUrl UPnpRouter::url() const
{
	return d->routerSettingsUrl;
}
void UPnpRouter::setUrl(const QUrl &url)
{
	d->routerSettingsUrl = url;
}

bool UPnpRouter::openPort(quint16 port, const QString &protocol)
{
	return true;
}

bool UPnpRouter::closePort(quint16 port)
{
	return true;
}


