#include "upnpRouterPrivate.h"

UpnpRouterPrivate::UpnpRouterPrivate(){}
UpnpRouterPrivate::UpnpRouterPrivate(const QUrl &url)
{
	router = new Router(url);
}

UpnpRouterPrivate UpnpRouterPrivate::upnpRouterPrivate(const QUrl &url)
{
	UPnp::upnp();
	UpnpRouterPrivate router = UpnpRouterPrivate(url);
	return router;
}

UPnp *UpnpRouterPrivate::d=NULL;

QList<UpnpRouterPrivate> UpnpRouterPrivate::listRouterPrivate()
{
	if(d==NULL)
	{
		d = UPnp::upnp();
	}
	static QList<UpnpRouterPrivate> routerPrivate;
	d->searchDevices();
	QList<QUrl> list = d->devicesSettingUrl();
	foreach (QUrl url, list)
	{
		UpnpRouterPrivate router = UpnpRouterPrivate(url);
		routerPrivate.push_back ( router );
	}
	return routerPrivate;
}

UpnpRouterPrivate::UpnpRouterPrivate()
{
	if(d==NULL)
	{
		d = UPnp::upnp();
	}
}

UpnpRouterPrivate::UpnpRouterPrivate(QUrl &url)
{
	if(d==NULL)
	{
		d = UPnp::upnp();
	}
	m_routerSettingUrl = url;	

	QString serviceType;
	QString serviceId;
	QString controlURL;
	QString eventSubURL;
	QUrl serviceSettingsUrl;
	
	//Load document XML
	IXML_Document * xmlRouter = ixmlLoadDocument(url.toString().toLatin1().data());
	
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
		service = Service();
		service.setServiceType(serviceType);
		service.setServiceId(serviceId);
		service.setControlURL(controlURL);
		service.setEventSubURL(eventSubURL);
		service.setXmlDocService(serviceSettingsUrl.toString());

		service.addAllActions();
	}
}

UpnpRouterPrivate::UpnpRouterPrivate(const UpnpRouterPrivate &router)
{
	this->router = router.router;
}
		
UpnpRouterPrivate &UpnpRouterPrivate::operator=(const UpnpRouterPrivate &router)
{
	this->router = router.router;
	return *this;
}

QString UpnpRouterPrivate::routerDescription() const
{
	return router->routerDescription();
}

/**
* TODO finish to implementation of isEmpty
*/
bool UpnpRouterPrivate::isEmpty()
{
	bool empty = true;
	/*if(this->m_routerType.isEmpty()==false)
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
	}*/

	return empty;
}

bool UpnpRouterPrivate::isValid() const
{
	return this->router->isValid();
}

bool UpnpRouterPrivate::openPort(quint16 port, const QString &typeProtocol, const QString &protocol)
{
	UPnp *d = UPnp::upnp();
	bool send = false;
	QList<QString> paramNameAction;
	QList<QString> paramValueAction;

	char c_port[50];

	paramNameAction.append(QString("NewRemoteHost"));
	paramNameAction.append(QString("NewExternalPort"));
	paramNameAction.append(QString("NewProtocol"));
	paramNameAction.append(QString("NewInternalPort"));
	paramNameAction.append(QString("NewInternalClient"));
	paramNameAction.append(QString("NewEnabled"));
	paramNameAction.append(QString("NewPortMappingDescription"));
	paramNameAction.append(QString("NewLeaseDuration"));
	
	sprintf(c_port,"%d",port);

	
	paramValueAction.append(QString(""));
	paramValueAction.append(c_port);
	paramValueAction.append(typeProtocol);
	paramValueAction.append(c_port);
	paramValueAction.append(d->hostAddress().toString());
	paramValueAction.append(QString("1"));
	paramValueAction.append(protocol);
	paramValueAction.append(QString("0"));
 
	if(d->sendAction(router->routerSettingUrl(),router->service().serviceType(), router->service().controlURL(),QString("AddPortMapping"),paramNameAction,paramValueAction))
	{
		send = true;
	}
	return send;
}

bool UpnpRouterPrivate::closePort(quint16 port, const QString &typeProtocol)
{
	UPnp *d = UPnp::upnp();
	bool send = false;
	QList<QString> paramNameAction;
	QList<QString> paramValueAction;

	char c_numPort[50];

	paramNameAction.append(QString("NewRemoteHost"));
	paramNameAction.append(QString("NewExternalPort"));
	paramNameAction.append(QString("NewProtocol"));

	sprintf(c_numPort,"%d",port);

	paramValueAction.append(QString(""));
	paramValueAction.append(c_numPort);
	paramValueAction.append(typeProtocol);

	if(d->sendAction(router->routerSettingUrl(),router->service().serviceType(), router->service().controlURL(),QString("DeletePortMapping"),paramNameAction,paramValueAction))
		send = true;
	return true;
}

