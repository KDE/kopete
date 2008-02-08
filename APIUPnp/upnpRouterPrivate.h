#include <QtDebug>
#include <QHostAddress>
#ifndef _UPNPROUTERPRIVATE_H_
#define _UPNPROUTERPRIVATE_H_

#include <QUrl>
#include <QString>
#include <QList>
#include <QHostAddress>
#include "service.h"
#include "util_Xml.h"
#include "UPnp.h"

class UpnpRouterPrivate
{

	public:
		UpnpRouterPrivate();
	
		/**
		* Return all router detected on network
		*/
		QList<UpnpRouterPrivate> listRouterPrivate();

		/**
		* Constructor UpnpRouterPrivate
		*
		* @param url url setting router
		*/
		UpnpRouterPrivate(QUrl &url);
		
		/**
		* Description of router
		*
		* @return description of router
		*/
		QString routerDescription();

		/**
		* Check if router is valid
		*
		* @return true if router is valid, false otherwise
		*/
		bool isValid();

		/**
		* Check if router is empty
		*
		* @return true if router is empty, false otherwise
		*/
		bool isEmpty();

		/**
		* Check if the URL is valid
		*
		* @param url has checked
		* @return true if url is valid, false otherwise
		*/
		bool isValidUrl(QUrl &url);

		/**
		* Opens port on the router default
		*
		* @param port port number
		* @param typeProtocol choose between TCP and UDP
		* @param protocol name of protocol
		*/
		bool openPort(quint16 port, const QString &typeProtocol, const QString &protocol);
		
		/**
		* Delete port on the router default
		*
		* @param port port number
		* @param typeProtocol choose between TCP and UDP
		*/
		bool closePort(quint16 port, const QString &typeProtocol);
	
	private:
		
		QUrl m_routerSettingUrl;
		QHostAddress routerAddress;
		Service service;
		
		QString m_routerType;
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
		static UPnp *d;
};
#endif
