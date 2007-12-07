#ifndef _UPNPKOPETE_H_
#define _UPNPKOPETE_H_


#include <stdio.h>
#include <string.h>

#include <upnp/ixml.h>
#include <upnp/FreeList.h>
#include <upnp/iasnprintf.h>
#include <upnp/ithread.h>
#include <upnp/LinkedList.h>
#include <upnp/ThreadPool.h>
#include <upnp/TimerThread.h>
#include <upnp/upnpconfig.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include <QList>

#include "sample_util.h"

#define RESEARCH_SUCCESS 0
#define RESEARCH_ERROR 1

typedef struct {
	char deviceType[250];
	char UDN[250];
	char DescDocURL[250];
	char modelDescription[250];
}Device;

typedef struct{
	IXML_Document * DescDoc;
	char * location;
}DocXML;

typedef struct DeviceNodeAlias {
	Device device;
	DeviceNodeAlias * next;
}DeviceNode;



class UpnpKopete 
{
	public:
		~UpnpKopete();
		
		static UpnpKopete* getInstance();
		char * getHostIp();
		unsigned short getDestPort();
		int researchDevice();
		void addDevice(IXML_Document * DescDoc,char *location);
		void addXMLDescDoc (IXML_Document * DescDoc, char *location);
		QList<char *> viewXMLDescDoc();
		
		//int kopeteCallbackEventHandler( Upnp_EventType EventType, void *Event, void *Cookie );

	private:
		
		static UpnpKopete * uniqueInstance;		
		char * hostIp;
		unsigned short destPort;
		

		QList<DocXML> ListDescDoc;
		
		UpnpClient_Handle device_handle;
		
		char * deviceType;
		DeviceNode device;	

		UpnpKopete();	

		
		

};



#endif
