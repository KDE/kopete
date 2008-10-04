#ifndef _UPNPROUTERPRIVATE_H_
#define _UPNPROUTERPRIVATE_H_

#include <QtDebug>
#include <QUrl>
#include <QString>
#include <QList>
#include <QHostAddress>
#include "UPnp.h"
#include "router.h"

class UpnpRouterPrivate
{

	public:
		UpnpRouterPrivate();
	
		/**
		* Return all router detected on network
		*/
		QList<UpnpRouterPrivate> listRouterPrivate();

		static UpnpRouterPrivate defaultRouter();
		
		static UpnpRouterPrivate upnpRouterPrivate(const QUrl &url);

		/**
		* Constructs a copy of a router
		*
		* @param router the router to copy
		*/
		UpnpRouterPrivate(const UpnpRouterPrivate &router);
		
		UpnpRouterPrivate &operator=(const UpnpRouterPrivate &router);
		
		QString routerDescription() const;
		/**
		* Check if router is empty
		*
		* @return true if router is empty, false otherwise
		*/
		bool isEmpty();

		/**
		* Check if router is valid
		*
		* @return true if router is valid, false otherwise
		*/
		bool isValid() const;

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

// 		UpnpRouterPrivate &operator=(const UpnpRouterPrivate &router);
		
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
