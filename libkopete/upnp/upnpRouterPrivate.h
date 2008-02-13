/*
    upnpRouterPrivate.h -

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

#ifndef _UPNPROUTERPRIVATE_H_
#define _UPNPROUTERPRIVATE_H_

#include <QtDebug>
#include <QUrl>
#include <QString>
#include <QList>
#include <QHostAddress>
#include "UPnp.h"
#include "router.h"

class UpnpRouterPrivate
{

	public:
		UpnpRouterPrivate();

		/**
		* Constructs a copy of a router
		*
		* @param router the router to copy
		*/
		UpnpRouterPrivate(const UpnpRouterPrivate &router);
		
		UpnpRouterPrivate &operator=(const UpnpRouterPrivate &router);
		
		QString routerDescription() const;
		/**
		* Check if router is empty
		*
		* @return true if router is empty, false otherwise
		*/
		bool isEmpty();

		/**
		* Check if router is valid
		*
		* @return true if router is valid, false otherwise
		*/
		bool isValid() const;

		/**
		* Opens port on the router default
		*
		* @param port port number
		* @param typeProtocol choose between TCP and UDP
		* @param protocol name of protocol
		*/
		bool openPort(quint16 port, const QString &typeProtocol, const QString &protocol);
		
		/**
		* Delete port on the router default
		*
		* @param port port number
		* @param typeProtocol choose between TCP and UDP
		*/
		bool closePort(quint16 port, const QString &typeProtocol);

		
		Router *router;
		UpnpRouterPrivate(const QUrl &url);
};
#endif
