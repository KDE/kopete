/*
    upnpRouter.h -

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
/**
*	@author CASTAN Romain <romaincastan@gmail.com>
*	@author DEMAY Bertrand <bertranddemay@gmail.com>
*/

#include <QtDebug>
#include <QUrl>
#include <QString>
#include <QList>

#include "upnpRouterPrivate.h"

class UPnpRouter
{
public:
	/**
	* Retrieves all the routers 
	*
	* @return the list of the routers
	*/
	static QList<UPnpRouter> allRouters();
	
	/**
	*
	* Constructs a router default
	*
	*/	
	static UPnpRouter defaultRouter();

	/**
	* Constructs a router for a given setting url
	*
	* @param url
	*/
	UPnpRouter(const QUrl &url);

	UPnpRouter();
	
	/**
	* Constructs a copy of a router
	*
	* @param router the router to copy
	*/
	UPnpRouter(const UPnpRouter &router);
 
	UPnpRouter &operator=(const UPnpRouter &router);

	/**
	* Indicates if this router is valid
	*
	* @return true if this router is valid, false otherwise
	*/
	bool isValid() const;
	
	/**
	* Retrieves all description of a router
	*
	* @return string containt a description of a router
	*/
	QString routerDescription() const;

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


private:
	UpnpRouterPrivate d;
};

