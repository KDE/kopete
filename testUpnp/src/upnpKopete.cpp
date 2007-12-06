#include "upnpKopete.h"
#include "applicationWidget.h"
UpnpKopete *UpnpKopete::uniqueInstance = NULL;

int kopeteCallbackEventHandler( Upnp_EventType EventType, void *Event, void *Cookie );


UpnpKopete::UpnpKopete()
{
	int ret;
	this->ListDescDoc.erase(this->ListDescDoc.begin(),this->ListDescDoc.end());
	this->deviceType="upnp:rootdevice";

	this->device_handle =-1;

	this->hostIp = NULL;
	this->destPort = 0;
	
	ret = UpnpInit(this->hostIp,this->destPort);
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
	
	this->hostIp = UpnpGetServerIpAddress();
	this->destPort = UpnpGetServerPort();
	printf("UPnP Initialized %s:%d\n",this->hostIp,this->destPort);

	//int (*ptrCallbackEventHandler)( Upnp_EventType , void *, void * )= &kopeteCallbackEventHandler;

	printf("Registering Control Point\n");
    	ret = UpnpRegisterClient(kopeteCallbackEventHandler,&device_handle, &device_handle );

	if( UPNP_E_SUCCESS != ret )
	{
        	printf( "Error registering Control point: %d\n", ret );
        	UpnpFinish();
        }
	printf("Control Point Registered\n");

//     	ret = this->researchDevice();
// 	if(ret != RESEARCH_SUCCESS)
// 	{
// 		printf("Error research device %d\n",ret);
// 		UpnpFinish();
// 	} 
// 	printf("Success research device\n");


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

char* UpnpKopete::getHostIp()
{
	return this->hostIp;
}

unsigned short UpnpKopete::getDestPort()
{
	return this->destPort;
}



int UpnpKopete::researchDevice()
{
	int ret;
	//TvCtrlPointRemoveAll(  );
	//text_mess->append("Research device");
	ret = UpnpSearchAsync( device_handle, 1, this->deviceType, NULL );
    	if( UPNP_E_SUCCESS != ret ) 
	{
        	printf("Error sending search request%d", ret);
		return RESEARCH_ERROR;
	}
	printf("deviceType %d\n",device_handle);

    	return RESEARCH_SUCCESS;
}

void UpnpKopete::addDevice(IXML_Document * DescDoc,char *location)
{
	char *deviceType=NULL;
	char *UDN=NULL;
	char *DescDocURL=NULL;
	char *modelDescription=NULL;
}

void UpnpKopete::addXMLDescDoc (IXML_Document * DescDoc, char *location)
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
		if(strcmp(tmp.location ,location)==0)
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

void  UpnpKopete::viewXMLDescDoc()
{
	char * chaine="";
	DocXML tmp;
	this->ListDescDoc.begin();
	printf("size %d\n",this->ListDescDoc.size());
	for(int i =0; i < this->ListDescDoc.size(); i++)
	{	
		tmp = this->ListDescDoc.last();
		printf("tmp.location %s\n",tmp.location);

	}

}

int kopeteCallbackEventHandler( Upnp_EventType EventType, void *Event, void *Cookie )
{
	int ret = 0;
	UpnpKopete * getUpnp = UpnpKopete::getInstance();
	
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
                	}

                	if( DescDoc )
                    		ixmlDocument_free( DescDoc );

			break;
		}
        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
		printf("UPNP_DISCOVERY_SEARCH_TIMEOUT\n");break;
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





