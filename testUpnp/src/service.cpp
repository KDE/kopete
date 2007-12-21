#include "service.h"

Service::Service(char *serviceType, char *serviceId, char *controlURL, char *eventSubURL,char *URLdocXml)
{
	printf("## entree constructeur service ##\n");
	int ret;

	this->serviceType = serviceType;
	this->serviceId = serviceId;
	this->controlURL = controlURL;
	this->eventSubURL = eventSubURL;
	this->xmlDocService = ixmlLoadDocument(URLdocXml);

	if((ret=UpnpDownloadXmlDoc(URLdocXml,&(this->xmlDocService))) != UPNP_E_SUCCESS ) 
	{
		printf("Error obtaining device description from %s -- error = %d",URLdocXml, ret);
	}
	/*initialisation
		parcours du fichier xml
		ajout des actions présentes */
		//creation action
		//recuperer le nom du fichier xml par parcours du nodelist
	IXML_NodeList * nodelist = ixmlDocument_getElementsByTagName(this->xmlDocService,"action");
	IXML_Node * actionNode;
	for(int i =0; i<ixmlNodeList_length(nodelist);i++)
	{
		actionNode = ixmlNodeList_item(nodelist,i);
 		addActionList(actionNode);
	}
	printf("## sortie constructeur service ##\n");
}

void Service::addActionList(IXML_Node * actionNode)
{
	//creation d'une action à partir d'actionNode
	//on récupère les paramètre et on les passe au constructeur Action
	printf("## add action list ##\n");
	char* actionName;
	char* argumentName;
	char* argumentDirection;
	char* argumentRelatedStableVariable;
	
	//creating action
	IXML_NodeList * actionNodeList = ixmlNode_getChildNodes(actionNode);
	IXML_Node * node = ixmlNodeList_item(actionNodeList,0);
	printf("action Node : %s\n",util_Xml_nodeValue(node));
	Action action = Action(util_Xml_nodeValue(node));

	//tester la taille de l'actionNodeList
	if (ixmlNodeList_length(actionNodeList) == 2)
	{
		IXML_Node * argnode = ixmlNodeList_item(actionNodeList,1);
		IXML_NodeList * argNodeList = ixmlNode_getChildNodes(argnode);
	
		//tester la taille de la liste
		for(int i=1;i< ixmlNodeList_length(argNodeList);i++)
		{
			IXML_Node * subargnode = ixmlNodeList_item(argNodeList,i);
			IXML_NodeList * subargNodeList = ixmlNode_getChildNodes(subargnode);

			IXML_Node * argnamenode = ixmlNodeList_item(subargNodeList,0);
			IXML_Node * argdirectionnode = ixmlNodeList_item(subargNodeList,1);
			IXML_Node * argrelatednode = ixmlNodeList_item(subargNodeList,2);
	
			argumentName = util_Xml_nodeValue(argnamenode);
			argumentDirection = util_Xml_nodeValue(argdirectionnode);
			argumentRelatedStableVariable = util_Xml_nodeValue(argrelatednode);
	
			action.addArgument(argumentName,argumentDirection,argumentRelatedStableVariable);
		}
	}


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
IXML_Document * Service::getXmlDocService(){return this->xmlDocService;}
QList<Action> Service::getActionList(){return this->actionList;}

void Service::viewActionList()
{
	printf("## affichage actions ##\n");
	actionList.begin();
	for(int i =0; i < actionList.size(); i++)
	{	
		Action tmp = actionList.last();
		printf("%s\n",tmp.getName());
	}
}