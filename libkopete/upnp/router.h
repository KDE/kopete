/*
    router.h -

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

#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <QtDebug>
#include <QtGlobal>
#include <QList>
#include <QHostAddress>
#include <QUrl>
#include <QString>

#include "service.h"
#include <upnp/ixml.h>
class Router
{
	public:

		/**
		* Constructor UpnpRouterPrivate
		*
		* @param url url setting router
		*/
		Router(const QUrl &url);

		QUrl routerSettingUrl() const;
		
		/**
		* Description of router
		*
		* @return description of router
		*/
		QString routerDescription();

		/**
		* Check if router is valid
		*
		* @return true if router is valid, false otherwise
		*/
		bool isValid();

		/**
		* Check if router is empty
		*
		* @return true if router is empty, false otherwise
		*/
		bool isEmpty();

		Service service();
	
	private:
			
		QUrl m_routerSettingUrl;
		QHostAddress routerAddress;
		Service m_service;
		
		QString m_routerType;
		QString m_friendlyName;
		QString m_manufacturer;
		QString m_manufacturerURL;
		QString m_modelName;
		QString m_UDN;
		QString m_modelDescription;
		QString m_modelNumber;
		QString m_serialNumber;
		QString m_presentationURL;
		QString m_UPC;

};

#endif
