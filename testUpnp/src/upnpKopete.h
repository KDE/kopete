#ifndef _UPNPKOPETE_H_
#define _UPNPKOPETE_H_


#include <stdio.h>
#include <string.h>

#include <ixml.h>
#include <FreeList.h>
#include <ithread.h>
#include <LinkedList.h>
#include <ThreadPool.h>
#include <TimerThread.h>
#include <upnpconfig.h>
#include <upnp.h>
#include <upnptools.h>

#include <QList>
#include <QString>
#include <QChar>

#include "device.h"
#include "sample_util.h"

#include "util_Xml.h"

#define RESEARCH_SUCCESS 0
#define RESEARCH_ERROR 1
#define ACTION_SUCCESS 0
#define ACTION_ERROR 1

class UpnpKopete 
{
	public:
		~UpnpKopete();
		
		static UpnpKopete* getInstance();
		QString hostIp();
		unsigned short destPort();
		
		QString routeurLocation();
		QString descDoc();
		Device mainDevices();
		void viewDevice();

		void setRouteurLocation(QString routeurLocation);
		void setDescDoc(QString url);

		void addDevice(IXML_Document * DescDoc,QString location);
		//different actions
		int upnpConnect();
		int openPort(QString nameProtocol, int numPort);
		int deletePort(int numPort);
		int statusInfo();
		int portMapping();
	private:
		
		static UpnpKopete * m_uniqueInstance;		
		
		QString m_routeurLocation;
		QString m_deviceType;
		QString m_hostIp;
		unsigned short m_destPort;
		QString m_descDoc;
		UpnpClient_Handle m_device_handle;
		Device m_mainDevices;	

		UpnpKopete();
		
		IXML_Document * sendAction(QString nameAction, QList<QString> paramNameAction,QList<QString> paramValueAction);
};
#endif
