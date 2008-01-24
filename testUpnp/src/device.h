/*
    device.h -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>
    Copyright (c) 2007-2008 by Julien Hubatzeck   <reineur31@gmail.com>
    Copyright (c) 2007-2008 by Michel Saliba      <msalibaba@gmail.com>

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

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "service.h"
#include <upnp/ixml.h>
#include <upnp/FreeList.h>
#include <upnp/ithread.h>
#include <upnp/LinkedList.h>
#include <upnp/ThreadPool.h>
#include <upnp/TimerThread.h>
#include <upnp/upnpconfig.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include "util_Xml.h"

#include <QList>
#include <QString>

class Device
{

	private:
		QString m_deviceType;
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

		QString m_DescDocURL;
		
		QList<Device> listDevice;
		QList<Service> listService;

	public:
		Device();
		Device(QString deviceType,
			QString friendlyName,
			QString manufacturer,
			QString manufacturerURL,
			QString modelName,
			QString UDN,
			QString modelDescription,
			QString modelNumber,
			QString serialNumber,
			QString presentationURL,
			QString UPC,
			QString DescDocURL);

		QString deviceType();
		QString friendlyName();
		QString manufacturer();
		QString manufacturerURL();
		QString modelName();
		QString UDN();
		QString modelDescription();
		QString modelNumber();
		QString serialNumber();
		QString presentationURL();
		QString UPC();
		QString descDocURL();

		void setDeviceType(QString deviceType);
		void setFriendlyName(QString friendlyName);
		void setManufacturer(QString manufacturer);
		void setManufacturerURL(QString manufacturerURL);
		void setModelName(QString modelName);
		void setUDN(QString UDN);
		void setModelDescription(QString modelDescription);
		void setModelNumber(QString modelNumber);
		void setSerialNumber(QString serialNumber);
		void setPresentationURL(QString presentationURL);
		void setUPC(QString UPC);
		void setDescDocURL(QString descDocURL);
		
		QList<Device> getListDevice();
		QList<Service> getListService();
		

		void addDevice (Device device);
		void addService (Service service);
		void searchAndAddUnderDevices();
		void showDevice();
		void showDeviceList(); 
		bool isEmpty();
};


#endif
