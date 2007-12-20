#include "device.h"
Device::Device(char * deviceType,
char * friendlyName,
char * manufacturer,
char * manufacturerURL,
char * modelName,
char * UDN,
char * modelDescription,
char * modelNumber,
char * serialNumber,
char * presentationURL,
char * UPC,
char * DescDocURL,
IXML_Node * xmlDoc)
{
	printf("CONSTRUCTEUR DEVICE\n");
	
	this->deviceType = deviceType;
	this->friendlyName=friendlyName;
	this->manufacturer=manufacturer;
	this->manufacturerURL=manufacturerURL;
	this->modelName=modelName;
	this->UDN=UDN;
	this->modelDescription=modelDescription;
	this->modelNumber=modelNumber;
	this->serialNumber=serialNumber;
	this->presentationURL=presentationURL;
	this->UPC=UPC;
	this->DescDocURL=DescDocURL;	
	
	this->listDevice.erase(this->listDevice.begin(),this->listDevice.end());
	this->listService.erase(this->listService.begin(),this->listService.end());
	this->xmlDoc = xmlDoc;
	
	

	//add under device at level one
	//this->searchAndAddUnderDevices();
	
}

char* Device::getDeviceType(){return this->deviceType;}
char* Device::getFriendlyName(){return this->friendlyName;}
char* Device::getManufacturer(){return this->manufacturer;}
char* Device::getManufacturerURL(){return this->manufacturerURL;}
char* Device::getModelName(){return this->modelName;}
char* Device::getUDN(){return this->UDN;}
char* Device::getModelDescription(){return this->modelDescription;}
char* Device::getModelNumber(){return this->modelNumber;}
char* Device::getSerialNumber(){return this->serialNumber;}
char* Device::getPresentationURL(){return this->presentationURL;}
char* Device::getUPC(){return this->UPC;}
char* Device::getDescDocURL(){return this->DescDocURL;}






QList<Device> Device::getListDevice(){return this->listDevice;}
QList<Service> Device::getListService(){return this->listService;}
IXML_Node* Device::getXmlDoc(){return this->xmlDoc;}


void Device::addDevice (Device device)
{
	bool find=false;
	this->listDevice.begin();
	for(int i=0; i<this->listDevice.size() && !find; i++)
	{
		if(strcmp(device.getDeviceType(),this->listDevice.last().getDeviceType())==0)
		{
			find=true;
		}
	}
	if(find==false)
	{
		this->listDevice.append(device);
	}	
	else
	{
		printf("Device exist\n");
	}
}
void Device::addService (Service service)
{
	bool find=false;
	this->listService.begin();
	for(int i=0; i<this->listService.size() && !find; i++)
	{
		if(strcmp(service.getServiceType(),this->listService.last().getServiceType())==0)
		{
			find=true;
		}
	}
	if(find==false)
	{
		this->listService.append(service);
	}	
	else
	{
		printf("Service exist\n");
	}
}

void Device::searchAndAddUnderDevices()
{
	char *deviceType= NULL;
	char *friendlyName= NULL;
	char *manufacturer= NULL;
	char *manufacturerURL= NULL;
	char *modelName= NULL;
	char *UDN= NULL;
	char *modelDescription= NULL;
	char *modelNumber= NULL;
	char *serialNumber= NULL;
	char *presentationURL= NULL;
	char *UPC= NULL;
	char *DescDocURL= NULL;
	IXML_Node * xmlDoc = NULL;


	IXML_Document *parent = (IXML_Document*)this->xmlDoc;
	/*Device underDevice = NULL;*/	

	IXML_NodeList * deviceList = ixmlDocument_getElementsByTagName(parent,"deviceList");
	//verifie si il y a des deviceList
	if(ixmlNodeList_length(deviceList) > 0)
	{
		//Recupere le premier deviceList
		IXML_Node* node = ixmlNodeList_item(deviceList,0);
		//je recupere les devices de la device list
		IXML_NodeList* devices = ixmlNode_getChildNodes(node);
		for(int i = 0;i<ixmlNodeList_length(devices);i++)
		{
			//recuperer le node principale d'un device
			IXML_Node* childDevice = ixmlNodeList_item(devices,i);
			//recupere la liste des info du device
			IXML_NodeList * nChild = ixmlNode_getChildNodes(childDevice);
			for(int j=0;j<ixmlNodeList_length(nChild);j++)
			{
				IXML_Node* n = ixmlNodeList_item(nChild,j);
				if(strcmp(ixmlNode_getNodeName(n),"deviceType")==0)
				{
					deviceType = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"friendlyName")==0)
				{
					friendlyName = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"manufacturer")==0)
				{
					manufacturer = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"manufacturerURL")==0)
				{
					manufacturerURL = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"modelName")==0)
				{
					modelName = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"UDN")==0)
				{
					UDN = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"modelDescription")==0)
				{
					modelDescription = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"modelNumber")==0)
				{
					modelNumber = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"serialNumber")==0)
				{
					serialNumber = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"presentationURL")==0)
				{
					presentationURL = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"UPC")==0)
				{
					UPC = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"DescDocURL")==0)
				{
					DescDocURL = util_Xml_nodeValue(n);
				}
				if(strcmp(ixmlNode_getNodeName(n),"modelDescription")==0)
				{
					modelDescription = util_Xml_nodeValue(n);
				}
			}
			//xmlDoc = ixmlNode_getOwnerDocument(childDevice);
			//create device
				//Device underDevice = Device(deviceType,friendlyName,manufacturer,manufacturerURL,modelName,UDN,modelDescription,modelNumber,serialNumber,presentationURL,UPC,DescDocURL,xmlDoc);
			
				//add device in the list
				//this->addDevice(underDevice);
				
		}
	}
}

void Device::showDevice()
{
	printf("================================================================================\n");
	printf("===                           DEVICE  DESCRIPTION                            ===\n");
	printf("================================================================================\n");
	printf("	Device type : %s\n",this->getDeviceType());
	printf("	Friendly name : %s\n",this->getFriendlyName());
	printf("	Manufacturer : %s\n",this->getManufacturer());
	printf("	Manufacturer URL : %s\n",this->getManufacturerURL());
	printf("	Model name : %s\n",this->getModelName());
	printf("	UDN : %s\n",this->getUDN());
	printf("	Model description : %s\n",this->getModelDescription());
	printf("	Model number : %s\n",this->getModelNumber());
	printf("	Serial number : %s\n",this->getSerialNumber());
	printf("	Presentation URL : %s\n",this->getPresentationURL());
	printf("	UPC : %s\n",this->getUPC());
	printf("	DescDocURL : %s\n",this->getDescDocURL());
	printf("	XML : \n%s\n",ixmlPrintNode(this->getXmlDoc()));
// 	if(this->getXmlDoc()!=NULL)
// 	{
// 		printf("okkkkk\n");
// 		IXML_NodeList* nl=ixmlDocument_getElementsByTagName(this->xmlDoc,"device" );
// 		printf("nombre de node %d\n",ixmlNodeList_length(nl));
// 		IXML_Node* nln = ixmlNodeList_item(nl,0);
// 		printf("%s\n",ixmlPrintNode(nln));
// 	}
// 	else
// 	{
// 		printf("merde\n");
// 	}
	printf("================================================================================\n\n");
}

void Device::showDeviceList()
{
	this->listDevice.begin();
	for (int i=0; i<this->listDevice.size();i++)
	{	
		printf("device %d : %s\n",i,this->listDevice.last().getDeviceType());
	}
}
