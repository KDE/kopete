/*
    service.h -

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

#ifndef _SERVICE_H_
#define _SERVICE_H_

#include "util_Xml.h"
#include <upnp/ixml.h>
#include <upnp/FreeList.h>

#include <upnp/ithread.h>
#include <upnp/LinkedList.h>
#include <upnp/ThreadPool.h>
#include <upnp/TimerThread.h>
#include <upnp/upnpconfig.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "action.h"
#include <QString>

class Service
{
	private:
		QString m_serviceType;
		QString m_serviceId;
		QString m_controlURL;
		QString m_eventSubURL;
		QString m_xmlDocService;
		QList<Action> m_actionList;

	public:
		// Constructor without argument
		Service();
		
		// Getters
		QString serviceType();
		QString serviceId();
		QString controlURL();
		QString eventSubURL();
		QString xmlDocService();
		QList<Action>* actionList();

		// Setters
		void setServiceType(QString serviceType);
		void setServiceId(QString serviceId);
		void setControlURL(QString controlURL);
		void setEventSubURL(QString eventSubURL);
		void setXmlDocService(QString URLdocXml);
		
		// Method which add all action of the service
		void addAllActions();
		void addActionList(IXML_Node * actionNode);
		Action* getActionAt(int i);
		bool existAction(QString nameAction);
		Action* actionByName(QString nameAction);
		void addAction(Action action);
		void viewService();
		void viewActionList();
		/**
		* TODO finish implementation
		*/
		bool isEmpty();
};

#endif
