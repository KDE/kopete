#include "upnpKopete.h"
#include "applicationWidget.h"
UpnpKopete *UpnpKopete::uniqueInstance = NULL;

int kopeteCallbackEventHandler( Upnp_EventType EventType, void *Event, void *Cookie );


UpnpKopete::UpnpKopete()
{
	int ret;
	this->ListDescDoc.erase(this->ListDescDoc.begin(),this->ListDescDoc.end());
	this->deviceType=QString("upnp:rootdevice");

	this->device_handle =-1;
	this->destPort = 0;
	
	ret = UpnpInit(NULL,this->destPort);
	switch(ret)
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
	
	this->hostIp = QString(UpnpGetServerIpAddress());
	this->destPort = UpnpGetServerPort();
	printf("UPnP Initialized %s:%d\n",this->hostIp.toLatin1().data(),this->destPort);

	printf("Registering Control Point\n");
    	ret = UpnpRegisterClient(kopeteCallbackEventHandler,&device_handle, &device_handle );

	if( UPNP_E_SUCCESS != ret )
	{
        	printf( "Error registering Control point: %d\n", ret );
        	UpnpFinish();
        }
	printf("Control Point Registered\n");
}


UpnpKopete::~UpnpKopete()
{
	delete(this);
}

UpnpKopete* UpnpKopete::getInstance()
{
	if (uniqueInstance == NULL)
	{
		uniqueInstance = new UpnpKopete();
	}
	return uniqueInstance;
}

QString UpnpKopete::getHostIp()
{
	return this->hostIp;
}

unsigned short UpnpKopete::getDestPort()
{
	return this->destPort;
}

QList<Device> UpnpKopete::mainDevices()
{
	return this->m_mainDevices;
}

int UpnpKopete::researchDevice()
{
	int ret;
	ret = UpnpSearchAsync( device_handle, 1, this->deviceType.toLatin1().data() , NULL );
    	if( UPNP_E_SUCCESS != ret ) 
	{
        	printf("Error sending search request%d", ret);
		return RESEARCH_ERROR;
	}
	printf("deviceType %d\n",device_handle);

    	return RESEARCH_SUCCESS;
}

void UpnpKopete::addDevice(IXML_Document * DescDoc,QString location)
{
	printf("#############ADD DEVICE###########\n");
	printf("location : %s\n",location.toLatin1().data());
	QString deviceType;
	QString friendlyName;
	QString manufacturer;
	QString manufacturerURL;
	QString modelName;
	QString UDN;
	QString modelDescription;
	QString modelNumber;
	QString serialNumber;
	QString presentationURL;
	QString UPC;
	QString DescDocURL;

	QString serviceType;
	QString serviceId;
	QString controlURL;
	QString eventSubURL;
	QString UrlDocXml;

	IXML_Document *parent = DescDoc;
	IXML_Node * nodeDevice;
	

	IXML_NodeList * deviceList = ixmlDocument_getElementsByTagName(parent,"deviceType");
	//verifie si il y a des deviceList
	if(ixmlNodeList_length(deviceList) > 0)
	{
		int posDevice;
		for(int i = 0;i<ixmlNodeList_length(deviceList);i++)
		{
			if(strcmp(util_Xml_nodeValue(ixmlNodeList_item(deviceList,i)),"urn:schemas-upnp-org:device:WANConnectionDevice:1")==0)
			{
				nodeDevice = ixmlNodeList_item(deviceList,i);
				posDevice = i;
			}
		}
		//Recupere le premier deviceList
		IXML_Node* node = ixmlNodeList_item(deviceList,posDevice);
		//je recupere les devices de la device list
		IXML_NodeList* nChild = ixmlNode_getChildNodes(ixmlNode_getParentNode(node));
		for(int j = 0;j<ixmlNodeList_length(nChild);j++)
		{
			//recupere la liste des info du device
	
			IXML_Node* n = ixmlNodeList_item(nChild,j);
			if(strcmp(ixmlNode_getNodeName(n),"deviceType")==0)
			{
				deviceType=QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"friendlyName")==0)
			{
				friendlyName = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"manufacturer")==0)
			{
				manufacturer =QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"manufacturerURL")==0)
			{
				manufacturerURL = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"modelName")==0)
			{
				modelName = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"UDN")==0)
			{
				UDN = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"modelDescription")==0)
			{
				modelDescription = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"modelNumber")==0)
			{
				modelNumber = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"serialNumber")==0)
			{
				serialNumber = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"presentationURL")==0)
			{
				presentationURL = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"UPC")==0)
			{
				UPC = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"modelDescription")==0)
			{
				modelDescription = QString(util_Xml_nodeValue(n));
			}
			if(strcmp(ixmlNode_getNodeName(n),"serviceList")==0)
			{
				IXML_NodeList* nList = ixmlNode_getChildNodes(n);
				IXML_Node* nService = ixmlNodeList_item(nList,0);
				
				if(strcmp(ixmlNode_getNodeName(nService),"service")==0)
				{
					IXML_NodeList* nServiceList = ixmlNode_getChildNodes(nService);
					for(int m=0;m<ixmlNodeList_length(nServiceList);m++)
					{
						IXML_Node* nServiceItem = ixmlNodeList_item(nServiceList,m);
						if(strcmp(ixmlNode_getNodeName(nServiceItem),"serviceType")==0)
						{
							serviceType = QString(util_Xml_nodeValue(nServiceItem));
						}
						if(strcmp(ixmlNode_getNodeName(nServiceItem),"serviceId")==0)
						{
							serviceId = QString(util_Xml_nodeValue(nServiceItem));
						}
						if(strcmp(ixmlNode_getNodeName(nServiceItem),"controlURL")==0)
						{
							controlURL = QString(util_Xml_nodeValue(nServiceItem));
						}
						if(strcmp(ixmlNode_getNodeName(nServiceItem),"eventSubURL")==0)
						{
							eventSubURL = QString(util_Xml_nodeValue(nServiceItem));
						}
						if(strcmp(ixmlNode_getNodeName(nServiceItem),"SCPDURL")==0)
						{
							int indice = location.indexOf (QString('/'),0, Qt::CaseInsensitive) ;
							
							UrlDocXml = location.left(indice);
							UrlDocXml.append(util_Xml_nodeValue(nServiceItem));		
						}
					}
				}
				
			}
		}
				
		Device underDevice = Device();
		underDevice.setDeviceType(deviceType);
		underDevice.setFriendlyName(friendlyName);
		underDevice.setManufacturer(manufacturer);
		underDevice.setManufacturerURL(manufacturerURL);
		underDevice.setModelName(modelName);
		underDevice.setUDN(UDN);
		underDevice.setModelDescription(modelDescription);
		underDevice.setModelNumber(modelNumber);
		underDevice.setSerialNumber(serialNumber);
		underDevice.setPresentationURL(presentationURL);
		underDevice.setUPC(UPC);
		underDevice.setDescDocURL(location);


		//adding device service
		Service service = Service();
		service.setServiceType(serviceType);
		service.setServiceId(serviceId);
		service.setControlURL(controlURL);
		service.setEventSubURL(eventSubURL);
		service.setXmlDocService(UrlDocXml);
		
		underDevice.addService(service);
		service.viewActionList();


		//add device in the list maindevice
		bool find=false;
		this->m_mainDevices.begin();
		for(int i=0; i<this->m_mainDevices.size() && !find; i++)
		{
			if(underDevice.deviceType() == this->m_mainDevices.last().deviceType())
			{
				find=true;
			}
		}
		if(find==false)
		{
			this->m_mainDevices.append(underDevice);
		}	
		else
		{
			printf("Device main exist\n");
		}
				
	}


}

void UpnpKopete::addXMLDescDoc (IXML_Document * DescDoc, QString location)
{
	bool find = false;
	DocXML doc;
	DocXML tmp;
	doc.DescDoc = DescDoc;
	doc.location = location;
	
	//on verifie si le doc n existe pas
	this->ListDescDoc.begin();
	while(!this->ListDescDoc.isEmpty() && !find)
	{	
		tmp = this->ListDescDoc.last();
		//printf("tmp.location %s\n",tmp.location);
		if(tmp.location == location)
		{
			find = true;
		}

	}

	if(find ==false)
	{
		printf("ajout doc\n");
		this->ListDescDoc.append(doc);
	}
}

QList<QString>  UpnpKopete::viewXMLDescDoc()
{
	QList<QString> chaine;
	DocXML tmp;
	this->ListDescDoc.begin();
	printf("size %d\n",this->ListDescDoc.size());
	for(int i =0; i < this->ListDescDoc.size(); i++)
	{	
		tmp = this->ListDescDoc.last();
		printf("tmp.location %s\n",tmp.location.toLatin1().data());
		chaine.append(tmp.location);
	}
	return chaine;

}

void UpnpKopete::viewListDevice()
{
	this->m_mainDevices.begin();
	if(this->m_mainDevices.size()==0)
	{
		printf("aucun device\n");
	}
	else
	{
		for(int i=0;i<this->m_mainDevices.size();i++)
		{
			Device d = this->m_mainDevices.last();
			d.showDevice();
		}
	}
}

int kopeteCallbackEventHandler( Upnp_EventType EventType, void *Event, void *Cookie )
{
	int ret = 0;
 	UpnpKopete * getUpnp = UpnpKopete::getInstance();
// 
// 	IXML_Document *DescDoc = ixmlLoadDocument("XML/gatedesc.xml");
// 	if(DescDoc == NULL)
// 	{
// 		printf("error loadDocument\n");
// 	}
// 	getUpnp->addXMLDescDoc(DescDoc, "XML/gatedesc.xml");
// 	getUpnp->addDevice(DescDoc, "XML/gatedesc.xml");
// 	if( DescDoc )
//         	ixmlDocument_free( DescDoc );
	
    switch ( EventType ) {
            /*
               SSDP Stuff 
             */
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		printf("UPNP_DISCOVERY_ADVERTISEMENT_ALIVE\n");break;
        case UPNP_DISCOVERY_SEARCH_RESULT:
		{
			printf("UPNP_DISCOVERY_SEARCH_RESULT\n");
			struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)Event;

			IXML_Document *DescDoc = NULL;
                	int ret;

                	if( d_event->ErrCode != UPNP_E_SUCCESS ) 
			{
                    		printf( "Error in Discovery Callback -- %d",d_event->ErrCode );
                	}

                	if((ret = UpnpDownloadXmlDoc(d_event->Location,&DescDoc)) != UPNP_E_SUCCESS ) 
			{
                    		printf("Error obtaining device description from %s -- error = %d",d_event->Location, ret);
                	}
			else
			{
				
				printf("Location URL %s\n",d_event->Location);
                    		//TvCtrlPointAddDevice( DescDoc, d_event->Location,d_event->Expires );
				getUpnp->addXMLDescDoc(DescDoc, d_event->Location);
				getUpnp->addDevice(DescDoc, d_event->Location);
                	}

                	if( DescDoc )
                    		ixmlDocument_free( DescDoc );

			break;
		}
        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
	{
		printf("UPNP_DISCOVERY_SEARCH_TIMEOUT\n");
		break;
	}
        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
		printf("UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE\n");break;
        case UPNP_CONTROL_ACTION_COMPLETE:
		printf("UPNP_CONTROL_ACTION_COMPLETE\n");break;
        case UPNP_CONTROL_GET_VAR_COMPLETE:
		printf("UPNP_CONTROL_GET_VAR_COMPLETE\n");break;
        case UPNP_EVENT_RECEIVED:
		printf("UPNP_EVENT_RECEIVED\n");break;
        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
		printf("UPNP_EVENT_SUBSCRIBE_COMPLETE\n");break;
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
		printf("UPNP_EVENT_UNSUBSCRIBE_COMPLETE\n");break;
        case UPNP_EVENT_RENEWAL_COMPLETE:
		printf("UPNP_EVENT_RENEWAL_COMPLETE\n");break;
        case UPNP_EVENT_AUTORENEWAL_FAILED:
		printf("UPNP_EVENT_AUTORENEWAL_FAILED\n");break;
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
		printf("UPNP_EVENT_SUBSCRIPTION_EXPIRED\n");break;
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
		printf("UPNP_EVENT_SUBSCRIPTION_REQUEST\n");break;
        case UPNP_CONTROL_GET_VAR_REQUEST:
		printf("UPNP_CONTROL_GET_VAR_REQUEST\n");break;
        case UPNP_CONTROL_ACTION_REQUEST:
		printf("UPNP_CONTROL_ACTION_REQUEST\n");break;
    }	

    return ret;
}





