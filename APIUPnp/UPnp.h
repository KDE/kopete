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
		* Returns list of routers detected
		*
		* @return list of routers detected
		*/
// 		QList<Router> routers();
		/**
		* Return the url of a list of the devices has a given position
		*
		* @param i position number
		* @return url device
		*/
		QUrl deviceSettingUrlAt(int i);

		/**
		* Return the router of a list of the devices has a given position
		*
		* @param i position number
		* @return router device
		*/
/*		Router routerAt(int i);	*/	

		/**
		* Check if the URL exists in the list
		*
		* @param url has checked
		* @return true if url exist, false otherwise
		*/
		bool settingUrlExist(QUrl &url);

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
		/*QList<Router> m_routers;*/
					
		UPnp();
};
#endif
