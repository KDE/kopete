#include "applicationWidget.h"
#include <QList> 

ApplicationWidget::ApplicationWidget(QWidget *parent): QMainWindow(parent)
{
	setupUi(this);
	this->upnp = UpnpKopete::getInstance();
	//connect(btEnvoi, SIGNAL(clicked()),this, SLOT(afficher()));
	connect(btEnvoi, SIGNAL(clicked()),this, SLOT(envoyer()));
	connect(actionQuitter, SIGNAL(clicked()),this, SLOT(close()));
}


void ApplicationWidget::afficher()
{
	/*QString mess = text_ecrit->toPlainText();
	text_mess->append(mess);
	text_ecrit->setPlainText("");*/
	
}

void ApplicationWidget::envoyer()
{
	int ret;
	char * tmp;
	ret = this->upnp->researchDevice();
	text_mess->append("######################################");
	text_mess->append("Research Device ...");
	
	if(ret != 0)
	{
		printf("Error research device %d\n",ret);
		UpnpFinish();
	} 
	QList<char *> liste = this->upnp->viewXMLDescDoc();
	for(int i =0; i < liste.size(); i++)
	{	
		tmp = liste.last();
		text_mess->append("Device location :");
		text_mess->append(tmp);
	}	
	text_mess->append("######################################");

	this->upnp->viewListDevice();
	
}

void ApplicationWidget::close()
{
	this->upnp->~UpnpKopete();
	printf("close\n");
}

// void ApplicationWidget::envoyer()
// {
// 	printf("ENVOYER\n");
// 	int port = 0;
// 	char * ip_adresse_use = NULL;
// 	int res;
// 
// 	//initialisation
// 	res = UpnpInit(ip_adresse_use,port);
// 	if(res != UPNP_E_SUCCESS)
// 	{
// 		printf("Erreur UpnpInit : %d\n",res);
// 		UpnpFinish();
// 		//return UPNP_E_SOCKET_ERROR;
// 	}
// 	printf("getServerPort : %d\n",UpnpGetServerPort());
// 	printf("getServerIP : %s\n",UpnpGetServerIpAddress());
// 
// 	
// 	
// // 	res = UpnpRegisterClient(MessCtrlPointCallbackEventHandler, &ctrlpt_handle, &ctrlpt_handle);
// // 	if(res !=UPNP_E_SUCCESS)
// // 	{
// // 		printf("Erreur UpnpRegisterClient : %d\n",res);
// // 		UpnpFinish();
// // 	}
// 	
// 
// 	UpnpFinish();
// }

// (*Upnp_FunPtr) ApplicationWidget::MessCtrlPointCallbackEventHandler(Upnp_EventType EventType, void* Event, void* Cookie)
// {
// 	struct Upnp_Discovery * d_event = (struct Upnp_Discovery *) Event;	
// 
// 	switch(EventType)
// 	{
// 		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE: 
// 			printf("UPNP_DISCOVERY_ADVERTISEMENT_ALIVE\n");
// 			break;
// 		case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE: 
// 			printf("UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE\n");
// 			break;
// 		case UPNP_DISCOVERY_SEARCH_RESULT: 
// 			printf("UPNP_DISCOVERY_SEARCH_RESULT\n");
// 			printf("d_event->Location : %s\n",d_event->Location);
// 			break;
// 		case UPNP_DISCOVERY_SEARCH_TIMEOUT: 
// 			printf("UPNP_DISCOVERY_SEARCH_TIMEOUT\n");
// 			break; 
// 	}
// }
