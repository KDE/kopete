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

class Device
{

	private:
		char * deviceType;
		char * friendlyName;
		char * manufacturer;
		char * manufacturerURL;
		char * modelName;
		char * UDN;
		char * modelDescription;
		char * modelNumber;
		char * serialNumber;
		char * presentationURL;
		char * UPC;

		char * DescDocURL;

		IXML_Node* xmlDoc;
		
		QList<Device> listDevice;
		QList<Service> listService;

	public:
		Device(char * deviceType,
			char * friendlyName,
			char * manufacturer,
			char * manufacturerURL,
			char * modelName,
			char * UDN,
			char * modelDescription,
			char * modelNumber,
			char * serialNumber,
			char * presentationURL,
			char * UPC,
			char * DescDocURL,
			IXML_Node * xmlDoc);

		char* getDeviceType();
		char* getFriendlyName();
		char* getManufacturer();
		char* getManufacturerURL();
		char* getModelName();
		char* getUDN();
		char* getModelDescription();
		char* getModelNumber();
		char* getSerialNumber();
		char* getPresentationURL();
		char* getUPC();
		char* getDescDocURL();
		IXML_Node* getXmlDoc();
		
		
		
		QList<Device> getListDevice();
		QList<Service> getListService();
		

		void addDevice (Device device);
		void addService (Service service);
		void searchAndAddUnderDevices();
		void showDevice();
		void showDeviceList(); 
};


#endif
