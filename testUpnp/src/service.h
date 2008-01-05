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
		IXML_Document *m_xmlDocService;
		QList<Action> m_actionList;

	public:
		Service();
		Service(QString serviceType, QString serviceId, QString controlURL, QString eventSubURL, QString URLdocXml);
		
		QString serviceType();
		QString serviceId();
		QString controlURL();
		QString eventSubURL();
		
		IXML_Document * xmlDocService();
		QList<Action> actionList();

		void setServiceType(QString serviceType);
		void setServiceId(QString serviceId);
		void setControlURL(QString controlURL);
		void setEventSubURL(QString eventSubURL);
		void setXmlDocService(QString URLdocXml);
		
		void addAllActions();
		void addActionList(IXML_Node * actionNode);
	    
		void viewActionList();
};

#endif
