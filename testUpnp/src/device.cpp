#include "device.h"

Device::Device()
{
	this->listDevice.erase(this->listDevice.begin(),this->listDevice.end());
	this->listService.erase(this->listService.begin(),this->listService.end());
}

Device::Device(QString deviceType,
QString friendlyName,
QString manufacturer,
QString manufacturerURL,
QString modelName,
QString UDN,
QString modelDescription,
QString modelNumber,
QString serialNumber,
QString presentationURL,
QString UPC,
QString DescDocURL)
{
	printf("CONSTRUCTEUR DEVICE\n");
	
	this->m_deviceType = deviceType;
	this->m_friendlyName=friendlyName;
	this->m_manufacturer=manufacturer;
	this->m_manufacturerURL=manufacturerURL;
	this->m_modelName=modelName;
	this->m_UDN=UDN;
	this->m_modelDescription=modelDescription;
	this->m_modelNumber=modelNumber;
	this->m_serialNumber=serialNumber;
	this->m_presentationURL=presentationURL;
	this->m_UPC=UPC;
	this->m_DescDocURL=DescDocURL;	
	
	this->listDevice.erase(this->listDevice.begin(),this->listDevice.end());
	this->listService.erase(this->listService.begin(),this->listService.end());	
}

QString Device::deviceType(){return this->m_deviceType;}
QString Device::friendlyName(){return this->m_friendlyName;}
QString Device::manufacturer(){return this->m_manufacturer;}
QString Device::manufacturerURL(){return this->m_manufacturerURL;}
QString Device::modelName(){return this->m_modelName;}
QString Device::UDN(){return this->m_UDN;}
QString Device::modelDescription(){return this->m_modelDescription;}
QString Device::modelNumber(){return this->m_modelNumber;}
QString Device::serialNumber(){return this->m_serialNumber;}
QString Device::presentationURL(){return this->m_presentationURL;}
QString Device::UPC(){return this->m_UPC;}
QString Device::descDocURL(){return this->m_DescDocURL;}

void Device::setDeviceType(QString deviceType){this->m_deviceType = deviceType;}
void Device::setFriendlyName(QString friendlyName){this->m_friendlyName = friendlyName;}
void Device::setManufacturer(QString manufacturer){this->m_manufacturer = manufacturer;}
void Device::setManufacturerURL(QString manufacturerURL){this->m_manufacturerURL = manufacturerURL;}
void Device::setModelName(QString modelName){this->m_modelName = modelName;}
void Device::setUDN(QString UDN){this->m_UDN = UDN;}
void Device::setModelDescription(QString modelDescription){this->m_modelDescription = modelDescription;}
void Device::setModelNumber(QString modelNumber){this->m_modelNumber = modelNumber;}
void Device::setSerialNumber(QString serialNumber){this->m_serialNumber = serialNumber;}
void Device::setPresentationURL(QString presentationURL){this->m_presentationURL = presentationURL;}
void Device::setUPC(QString UPC){this->m_UPC = UPC;}
void Device::setDescDocURL(QString descDocURL){this->m_DescDocURL = descDocURL;}

QList<Device> Device::getListDevice(){return this->listDevice;}
QList<Service> Device::getListService(){return this->listService;}

void Device::addDevice (Device device)
{
	bool find=false;
	this->listDevice.begin();
	for(int i=0; i<this->listDevice.size() && !find; i++)
	{
		if(device.deviceType() == this->listDevice.last().deviceType())
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
		Service service_tmp = this->listService.at(i);
		if(service.serviceType() == service_tmp.serviceType())
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

void Device::showDevice()
{
	printf("================================================================================\n");
	printf("===                           DEVICE  DESCRIPTION                            ===\n");
	printf("================================================================================\n");
	printf("	Device type : %s\n",this->deviceType().toLatin1().data());
	printf("	Friendly name : %s\n",this->friendlyName().toLatin1().data());
	printf("	Manufacturer : %s\n",this->manufacturer().toLatin1().data());
	printf("	Manufacturer URL : %s\n",this->manufacturerURL().toLatin1().data());
	printf("	Model name : %s\n",this->modelName().toLatin1().data());
	printf("	UDN : %s\n",this->UDN().toLatin1().data());
	printf("	Model description : %s\n",this->modelDescription().toLatin1().data());
	printf("	Model number : %s\n",this->modelNumber().toLatin1().data());
	printf("	Serial number : %s\n",this->serialNumber().toLatin1().data());
	printf("	Presentation URL : %s\n",this->presentationURL().toLatin1().data());
	printf("	UPC : %s\n",this->UPC().toLatin1().data());
	printf("	DescDocURL : %s\n",this->descDocURL().toLatin1().data());
	printf("================================================================================\n\n");
}

void Device::showDeviceList()
{
	this->listDevice.begin();
	for (int i=0; i<this->listDevice.size();i++)
	{	
		Device device_tmp = this->listDevice.at(i);
		printf("device %d : %s\n",i,device_tmp.deviceType().toLatin1().data());
	}
}

bool Device::isEmpty()
{
	bool empty = true;
	if(this->deviceType().isEmpty()==false)
	{
		empty = false;
	}
	if(this->friendlyName().isEmpty()==false)
	{
		empty = false;
	}
	if(this->manufacturer().isEmpty()==false)
	{
		empty = false;
	}
	if(this->manufacturerURL().isEmpty()==false)
	{
		empty = false;
	}
	if(this->modelName().isEmpty()==false)
	{
		empty = false;
	}
	if(this->UDN().isEmpty()==false)
	{
		empty = false;
	}
	if(this->modelDescription().isEmpty()==false)
	{
		empty = false;
	}
	if(this->modelNumber().isEmpty()==false)
	{
		empty = false;
	}
	if(this->serialNumber().isEmpty()==false)
	{
		empty = false;
	}
	if(this->presentationURL().isEmpty()==false)
	{
		empty = false;
	}
	if(this->UPC().isEmpty()==false)
	{
		empty = false;
	}
	if(this->descDocURL().isEmpty()==false)
	{
		empty = false;
	}
	if(this->getListDevice().isEmpty()==false)
	{
		empty = false;
	}
	if(this->getListService().isEmpty()==false)
	{
		empty = false;
	}
	return empty;
}