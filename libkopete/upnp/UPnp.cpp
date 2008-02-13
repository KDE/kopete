/*
    UPnp.cpp -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>

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

#include "UPnp.h"
#include <upnp/ixml.h>

UPnp *UPnp::m_upnp = NULL;
		
UPnp *UPnp::upnp()
{
	if(m_upnp == NULL)
		m_upnp = new UPnp();
	return m_upnp;
}

bool UPnp::searchDevices()
{
	bool search = true;
	/**
	*	Search device
	*/
	int error = UpnpSearchAsync( m_device_handle, 1, deviceType.toLatin1().data() , NULL );
    	if( error != UPNP_E_SUCCESS ) 
	{
        	printf("Error sending search request%d", error);
		search = true;
	}
	return search;
}

QHostAddress UPnp::hostAddress()
{
	return m_hostAddress;
}

quint16 UPnp::port()
{
	return m_port;
}

QList<QUrl> UPnp::devicesSettingUrl()
{
	return m_devicesSettingUrl;
}

// QList<Router> routers()
// {
// 	return m_routers;
// }

QUrl UPnp::deviceSettingUrlAt(int i)
{
	return m_devicesSettingUrl.at(i);
}			

// Router routerAt(int i)
// {
// 	return m_routers.at(i);
// }

bool UPnp::settingUrlExist(QUrl &url)
{
	return m_devicesSettingUrl.contains(url); 
}

bool UPnp::isValidUrl(QUrl &url)
{
	bool valid = false;
	IXML_Document * doc = NULL;
	if(UpnpDownloadXmlDoc(url.toString().toLatin1().data(),&doc) == UPNP_E_SUCCESS)
		valid = true;
	if( doc )
		ixmlDocument_free( doc );
	return valid;	
}

void UPnp::addSettingUrl(QUrl &url)
{
	m_devicesSettingUrl.push_back(url);
}

UPnp::UPnp()
{
	int error;
	deviceType = QString("upnp:rootdevice");
	this->m_device_handle =-1;
	this->m_port = 0;
	
	/**
	*	INITIALISATION
	*/
	error = UpnpInit(NULL,this->m_port);
	switch(error)
	{
		case UPNP_E_SUCCESS: printf("The operation UpnpInit completed successfully\n");break;
		case UPNP_E_OUTOF_MEMORY: printf("Insufficient resources exist to initialize the SDK\n");
					UpnpFinish();break;
		case UPNP_E_INIT: printf("The SDK is already initialized\n");
					UpnpFinish();break;
		case UPNP_E_INIT_FAILED: printf("The SDK initialization failed for an unknown reason\n");
					UpnpFinish();break;
		case UPNP_E_SOCKET_BIND: printf("An error occurred binding a socket\n");
					UpnpFinish();break;
		case UPNP_E_LISTEN: printf("An error occurred listening to a socket\n");
					UpnpFinish();break;
		case UPNP_E_OUTOF_SOCKET: printf("An error ocurred creating a socket\n");
					UpnpFinish();break;
		case UPNP_E_INTERNAL_ERROR: printf("An internal error ocurred\n");
					UpnpFinish();break;
		default: UpnpFinish();break;
	}
	
	this->m_hostAddress = QHostAddress(QString(UpnpGetServerIpAddress()));
	this->m_port = UpnpGetServerPort();
	printf("UPnP Initialized %s:%d\n",this->m_hostAddress.toString().toLatin1().data(),this->m_port);

	printf("Registering Control Point\n");
    	error = UpnpRegisterClient(upnpCallbackEventHandler,&m_device_handle, &m_device_handle );

	if( error != UPNP_E_SUCCESS )
	{
        	printf( "Error registering Control point: %d\n", error );
        	UpnpFinish();
        }
	printf("Control Point Registered\n");
}


int UPnp::upnpCallbackEventHandler( Upnp_EventType EventType, void *Event, void *Cookie )
{
	int response = 0;
	UPnp *upnp = UPnp::upnp();
	switch ( EventType ) {
		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
			qDebug () << "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE\n";break;
		case UPNP_DISCOVERY_SEARCH_RESULT:
			{
				qDebug () << "UPNP_DISCOVERY_SEARCH_RESULT\n";
				struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)Event;
	
				if( d_event->ErrCode != UPNP_E_SUCCESS ) 
				{
					qDebug () << "Error in Discovery Callback --" << d_event->ErrCode << "\n";
					response = -1;
				}
				QUrl url = QUrl(QString(d_event->Location));
				if(upnp->isValidUrl(url))
				{
					if(!upnp->settingUrlExist(url))
						upnp->addSettingUrl(url);
				}
				break;
			}
		case UPNP_CONTROL_ACTION_REQUEST:
			qDebug () << "UPNP_CONTROL_ACTION_REQUEST\n";break;
		case UPNP_DISCOVERY_SEARCH_TIMEOUT:
		{
			response = -1;
			printf("UPNP_DISCOVERY_SEARCH_TIMEOUT\n");
			break;
		}
	}	
	
	return response;
}

/**
* TODO finished implementation
*/
bool UPnp::sendAction(QUrl deviceSettingUrl, QString serviceType, QString serviceControl, QString nameAction, QList<QString> paramNameAction,QList<QString> paramValueAction)
{
	bool find = false;
	int ret;
	IXML_Document *actionNode = NULL;
	IXML_Document *response = NULL;
	
	if(this->isValidUrl(deviceSettingUrl) /*&& !service.isEmpty()*/)
	{
		//if(service.existAction(nameAction)==true)
		//{
			if(paramNameAction.isEmpty())
			{
				actionNode = UpnpMakeAction( nameAction.toLatin1().data(), serviceType.toLatin1().data(), 0,NULL );
			}
			else
			{
				for( int k=0; k < paramNameAction.size(); k++ ) 
				{
					QString nameActionP = paramNameAction.at(k);
					QString valueActionP = paramValueAction.at(k);
					 
					if( UpnpAddToAction( &actionNode, nameAction.toLatin1().data(), serviceType.toLatin1().data(),nameActionP.toLatin1().data(),valueActionP.toLatin1().data()) != UPNP_E_SUCCESS ) 
					{
						printf("Erreur UpnpAddToAction\n");
					}
				}
		
			}
			//WARNING testing deviceSettingUrl
			char * relURL = (char *)malloc( strlen( deviceSettingUrl.toString().toLatin1().data() ) + strlen(serviceControl.toLatin1().data() ) + 1 );
			
			ret = UpnpResolveURL( deviceSettingUrl.toString().toLatin1().data(), serviceControl.toLatin1().data(), relURL );
// 			qDebug () << "url" << relURL;
			if( ret != UPNP_E_SUCCESS )		
			{
				printf("Erreur UpnpResolveURL : %d\n",ret);
			}

			ret = UpnpSendActionAsync( 
				m_device_handle,relURL,
				serviceType.toLatin1().data(),
				NULL, 
				actionNode,
				upnpCallbackEventHandler,
				NULL );
// 			ret = UpnpSendAction( 
// 					m_device_handle,relURL,
// 				 	serviceType.toLatin1().data(),
// 					NULL, 
// 					actionNode,
// 					&response);
						
			if( ret != UPNP_E_SUCCESS ) 
			{
				printf("Erreur UpnpSendActionAsync\n");
			}
			//delete(relURL);
			if( actionNode )
				ixmlDocument_free( actionNode );
			
			find = true;
		//}
	}
	return find;
}



