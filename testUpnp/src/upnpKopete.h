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
#include <QString>

#include "device.h"
#include "sample_util.h"

#include "util_Xml.h"

#define RESEARCH_SUCCESS 0
#define RESEARCH_ERROR 1

// typedef struct {
// 	char deviceType[250];
// 	char UDN[250];
// 	char DescDocURL[250];
// 	char modelDescription[250];
// 	char ServicesDocURL[250];
// }Device;


typedef struct{
	IXML_Document * DescDoc;
	QString location;
}DocXML;

// typedef struct DeviceNodeAlias {
// 	Device device;
// // 	DeviceNodeAlias * ssDevice;
// 	DeviceNodeAlias * next;
// }DeviceNode;

class UpnpKopete 
{
	public:
		~UpnpKopete();
		
		static UpnpKopete* getInstance();
		QString getHostIp();
		unsigned short getDestPort();
		QList<Device> mainDevices();
      
		QString routeurLocation();
		void setRouteurLocation(QString routeurLocation);
		int researchDevice();

		void addDevice(IXML_Document * DescDoc,QString location);
		void addXMLDescDoc (IXML_Document * DescDoc,QString location);
		
		QList<QString> viewXMLDescDoc();
		void viewListDevice();

		//different actions
		void openPort(QString nameProtocol, int numPort);
		void deletePort(int numPort);
		

	private:
		
		static UpnpKopete * uniqueInstance;		
		
		QString m_routeurLocation;
		QString deviceType;
		QString hostIp;
		unsigned short destPort;
		

		QList<DocXML> ListDescDoc;
		
		UpnpClient_Handle device_handle;
		
		
		QList<Device> m_mainDevices;	

		UpnpKopete();
		void sendAction(QString nameAction, QList<QString> paramNameAction,QList<QString> paramValueAction);
};
#endif
