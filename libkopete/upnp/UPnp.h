/*
    UPnp.h -

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

#ifndef _UPNP_H_
#define _UPNP_H_

#include <QtDebug>
#include <QtGlobal>
#include <QList>
#include <QHostAddress>
#include <QUrl>

#include <upnp/upnp.h>
#include "service.h"


class UPnp
{
	public:
		/**
		* Create a object upnp unique
		*/
		static UPnp * upnp();
	
		/**
		* Search all devices on network
		*/
		bool searchDevices();	

		/**
		* Return address of the user
		*
		* @return address of the user
		*/
		QHostAddress hostAddress();

		/**
		* Return the communication port
		*
		* @return the communication port
		*/
		quint16 port();

		/**
		* Returns list of devices detected
		*
		* @return list of devices detected
		*/
		QList<QUrl> devicesSettingUrl();

		/**
		* Return the url of a list of the devices has a given position
		*
		* @param i position number
		* @return url device
		*/
		QUrl deviceSettingUrlAt(int i);	

		/**
		* Check if the URL exists in the list
		*
		* @param url has checked
		* @return true if url exist, false otherwise
		*/
		bool settingUrlExist(QUrl &url);

		/**
		* Check if the upnp is valid
		*
		* @return true if upnp is valid, false otherwise
		*/
		bool isValid();

		/**
		* Check if the URL is valid
		*
		* @param url has checked
		* @return true if url is valid, false otherwise
		*/
		bool isValidUrl(QUrl &url);

		/**
		* Adding url in the list
		*
		* @param url has adding
		*/
		void addSettingUrl(QUrl &url);

		static int upnpCallbackEventHandler( Upnp_EventType EventType, void *Event, void *Cookie );

		/**
		* Sending action router
		*
		* @return true if action was successful, false otherwise
		* TODO Put asynchronous
		* Warning
		*/
		bool sendAction(QUrl deviceSettingUrl, QString serviceType, QString serviceControl, QString nameAction, QList<QString> paramNameAction,QList<QString> paramValueAction);

	private:
		static UPnp * m_upnp;
		QString deviceType;
		QHostAddress m_hostAddress;
		quint16 m_port;
		
		UpnpClient_Handle m_device_handle;
		QList<QUrl> m_devicesSettingUrl;
					
		UPnp();
};
#endif
