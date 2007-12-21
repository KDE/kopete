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


class Service
{
	private:
		char *serviceType;
		char *serviceId;
		char *controlURL;
		char *eventSubURL;
		IXML_Document *xmlDocService;
		QList<Action> actionList;

	public:
		Service(char *serviceType, char *serviceId, char *controlURL, char *eventSubURL,char *URLdocXml);
		void addActionList(IXML_Node * actionNode);
		char * getServiceType();
		char * getServiceId();
		char * getControlURL();
		char * getEventSubURL();
		char * getDocXml();
		IXML_Document * getXmlDocService();
		QList<Action> getActionList();
		void viewActionList();
};

#endif
