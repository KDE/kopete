/*
    router.cpp -

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

#include "router.h"
#include "util_Xml.h"
#include <upnp/upnp.h>

Router::Router()
{

}

/**
* Constructor UpnpRouterPrivate
*
* @param url url setting router
*/
Router::Router(const QUrl &url)
{
	m_routerSettingUrl = url;	

	QString serviceType;
	QString serviceId;
	QString controlURL;
	QString eventSubURL;
	QUrl serviceSettingsUrl;
	
	//Load document XML
	IXML_Document * xmlRouter = NULL; 
	if(UpnpDownloadXmlDoc(m_routerSettingUrl.toString().toLatin1().data(),&xmlRouter)==UPNP_E_SUCCESS)
	{

		IXML_NodeList * devicesList = ixmlDocument_getElementsByTagName(xmlRouter,(DOMString)"deviceType");
		
		if(ixmlNodeList_length(devicesList) > 0)
		{
			int posDevice=0;
			for(unsigned int i = 0;i<ixmlNodeList_length(devicesList);i++)
			{
				if(strcmp(util_Xml_nodeValue(ixmlNodeList_item(devicesList,i)),"urn:schemas-upnp-org:device:WANConnectionDevice:1")==0)
				{
					//nodeDevice = ixmlNodeList_item(devicesList,i);
					posDevice = i;
				}
			}
			
			//get the first devicelist
			IXML_Node* node_router = ixmlNodeList_item(devicesList,posDevice);
			//getting all devices from the devicelist
			IXML_NodeList* router_Child = ixmlNode_getChildNodes(ixmlNode_getParentNode(node_router));
			for(int j = 0;j<(int)ixmlNodeList_length(router_Child);j++)
			{
				//getting all data list from device
				IXML_Node* child = ixmlNodeList_item(router_Child,j);
				if(strcmp(ixmlNode_getNodeName(child),"deviceType")==0)
				{
					m_routerType=QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"friendlyName")==0)
				{
					m_friendlyName = QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"manufacturer")==0)
				{
					m_manufacturer =QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"manufacturerURL")==0)
				{
					m_manufacturerURL = QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"modelName")==0)
				{
					m_modelName = QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"UDN")==0)
				{
					m_UDN = QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"modelDescription")==0)
				{
					m_modelDescription = QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"modelNumber")==0)
				{
					m_modelNumber = QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"serialNumber")==0)
				{
					m_serialNumber = QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"presentationURL")==0)
				{
					m_presentationURL = QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"UPC")==0)
				{
					m_UPC = QString(util_Xml_nodeValue(child));
				}
				if(strcmp(ixmlNode_getNodeName(child),"serviceList")==0)
				{
					IXML_NodeList* service_list = ixmlNode_getChildNodes(child);
					IXML_Node* nService = ixmlNodeList_item(service_list,0);
					
					if(strcmp(ixmlNode_getNodeName(nService),"service")==0)
					{
						IXML_NodeList* service_description = ixmlNode_getChildNodes(nService);
						for(int m=0;m<(int)ixmlNodeList_length(service_description);m++)
						{
							IXML_Node* service_Item = ixmlNodeList_item(service_description,m);
							if(strcmp(ixmlNode_getNodeName(service_Item),"serviceType")==0)
							{
								serviceType = QString(util_Xml_nodeValue(service_Item));
							}
							if(strcmp(ixmlNode_getNodeName(service_Item),"serviceId")==0)
							{
								serviceId = QString(util_Xml_nodeValue(service_Item));
							}
							if(strcmp(ixmlNode_getNodeName(service_Item),"controlURL")==0)
							{
								controlURL = QString(util_Xml_nodeValue(service_Item));
							}
							if(strcmp(ixmlNode_getNodeName(service_Item),"eventSubURL")==0)
							{
								eventSubURL = QString(util_Xml_nodeValue(service_Item));
							}
							if(strcmp(ixmlNode_getNodeName(service_Item),"SCPDURL")==0)
							{
								int indice = m_routerSettingUrl.toString().lastIndexOf (QString('/'),-1, Qt::CaseSensitive) ;
								
								QString ssUrl = m_routerSettingUrl.toString().left(indice);
								
								if(QString(util_Xml_nodeValue(service_Item)).at(0)!=QChar('/'))
								{
									ssUrl.push_back('/');
								}
								ssUrl.append(util_Xml_nodeValue(service_Item));
								serviceSettingsUrl = QUrl(ssUrl);
							}
						}
					}
					
				}
			}
	
			//adding device service
			m_service = Service();
			m_service.setServiceType(serviceType);
			m_service.setServiceId(serviceId);
			m_service.setControlURL(controlURL);
			m_service.setEventSubURL(eventSubURL);
			m_service.setXmlDocService(serviceSettingsUrl.toString());
			m_service.addAllActions();
		}
	}
}

QUrl Router::routerSettingUrl() const
{
	return m_routerSettingUrl;
}

QString Router::routerDescription()
{
	QString desc;
	desc.append("         Router        \n");
	desc.append(" url : "+m_routerSettingUrl.toString()+"\n");
	desc.append(" type : "+m_routerType+"\n");
	desc.append(" friendly name : "+m_friendlyName+"\n");
	desc.append(" manufacturer : "+m_manufacturer+"\n");
	desc.append(" manufacturer url : "+m_manufacturerURL+"\n");
	desc.append(" model name : "+m_modelName+"\n");
	desc.append(" UDN : "+m_UDN+"\n");
	desc.append(" model description : "+m_modelDescription+"\n");
	desc.append(" model number : "+m_modelNumber+"\n");
	desc.append(" serial number : "+m_serialNumber+"\n");
	desc.append(" presentation url : "+m_presentationURL+"\n");
	desc.append(" UPC : "+m_UPC+"\n");
	return desc;
}

bool Router::isValid() const
{
	bool valid = false;
 
	IXML_Document * xmlRouter = NULL;
	if(UpnpDownloadXmlDoc(m_routerSettingUrl.toString().toLatin1().data(),&xmlRouter)==UPNP_E_SUCCESS)
	{

		IXML_NodeList * devicesList = ixmlDocument_getElementsByTagName(xmlRouter,(DOMString)"deviceType");
		if(ixmlNodeList_length(devicesList) > 0)
		{
			for(unsigned int i = 0;i<ixmlNodeList_length(devicesList) && !valid;i++)
			{
				if(strcmp(util_Xml_nodeValue(ixmlNodeList_item(devicesList,i)),"urn:schemas-upnp-org:device:WANConnectionDevice:1")==0)
				{
					valid = true;
				}
			}
		}
	}
	return valid;	
}

/**
* Check if router is empty
*
* @return true if router is empty, false otherwise
*/
bool Router::isEmpty() const
{
	bool empty = true;
	if(this->m_routerSettingUrl.isEmpty()==false)
	{
		empty = false;
	}	
	if(this->m_routerType.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_friendlyName.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_manufacturer.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_manufacturerURL.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_modelName.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_UDN.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_modelDescription.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_modelNumber.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_serialNumber.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_presentationURL.isEmpty()==false)
	{
		empty = false;
	}
	if(this->m_UPC.isEmpty()==false)
	{
		empty = false;
	}
	
	return empty;
}

Service Router::service()
{
	return m_service;
}

