#include "service.h"

Service::Service(char *serviceType, char *serviceId, char *controlURL, char *eventSubURL,char *docXml)
{
	int ret;

	this->serviceType = serviceType;
	this->serviceId = serviceId;
	this->controlURL = controlURL;
	this->eventSubURL = eventSubURL;
	this->docXml = docXml;

	if((ret=UpnpDownloadXmlDoc(docXml,&(this->xmlDoc))) != UPNP_E_SUCCESS ) 
	{
		printf("Error obtaining device description from %s -- error = %d",docXml, ret);
	}
}

void Service::addActionList(Action action)
{
	bool find=false;
	this->actionList.begin();
	
	for(int i=0;i<this->actionList.size() && !find;i++)
	{
		
		if(strcmp(this->actionList.last().getName(),action.getName())==0)
		{
			find = true;
		}
	}
	if(find==false)
	{
		this->actionList.append(action);
	}
	else
	{
		printf("the action exist\n");
	}

}
char * Service::getServiceType(){return this->serviceType;}
char * Service::getServiceId(){return this->serviceId;}
char * Service::getControlURL(){return this->controlURL;}
char * Service::getEventSubURL(){return this->eventSubURL;}
char * Service::getDocXml(){return this->docXml;}
IXML_Document * Service::getXmlDoc(){return this->xmlDoc;}
QList<Action> Service::getActionList(){return this->actionList;}
