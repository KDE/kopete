#include "service.h"

Service::Service(){;}


Service::Service(QString serviceType, QString serviceId, QString controlURL, QString eventSubURL, QString URLdocXml)
{
	printf("## entree constructeur service ##\n");

	this->m_serviceType = serviceType;
	this->m_serviceId = serviceId;
	this->m_controlURL = controlURL;
	this->m_eventSubURL = eventSubURL;
	this->m_xmlDocService = ixmlLoadDocument(URLdocXml.toLatin1().data());
	
	printf("## sortie constructeur service ##\n");
}


QString Service::serviceType(){return this->m_serviceType;}
QString Service::serviceId(){return this->m_serviceId;}
QString Service::controlURL(){return this->m_controlURL;}
QString Service::eventSubURL(){return this->m_eventSubURL;}
IXML_Document * Service::xmlDocService(){return this->m_xmlDocService;}
QList<Action> Service::actionList(){return this->m_actionList;}

void Service::setServiceType(QString serviceType){this->m_serviceType = serviceType;}
void Service::setServiceId(QString serviceId){this->m_serviceId = serviceId;}
void Service::setControlURL(QString controlURL){this->m_controlURL = controlURL;}
void Service::setEventSubURL(QString eventSubURL){this->m_eventSubURL = eventSubURL;}
void Service::setXmlDocService(QString URLdocXml){this->m_xmlDocService = ixmlLoadDocument(URLdocXml.toLatin1().data());}

void Service::addAllActions()
{
	IXML_NodeList * nodelist = ixmlDocument_getElementsByTagName(this->m_xmlDocService,"action");
	IXML_Node * actionNode;
	for(int i =0; i<ixmlNodeList_length(nodelist);i++)
	{
		actionNode = ixmlNodeList_item(nodelist,i);
 		addActionList(actionNode);
	}
}

void Service::addActionList(IXML_Node * actionNode)
{
	//we create an action from an actionNode
	//we get parameters and give them to the Action constructor
	printf("## add action list ##\n");
	QString actionName;
	QString argumentName;
	QString argumentDirection;
	QString argumentRelatedStableVariable;
	
	//creating action
	IXML_NodeList * actionNodeList = ixmlNode_getChildNodes(actionNode);
	IXML_Node * node = ixmlNodeList_item(actionNodeList,0);
	printf("action Node : %s\n",util_Xml_nodeValue(node));
	Action action = Action(QString(util_Xml_nodeValue(node)));

	//test of the actionNodeList size
	if (ixmlNodeList_length(actionNodeList) == 2)
	{
		IXML_Node * argnode = ixmlNodeList_item(actionNodeList,1);
		IXML_NodeList * argNodeList = ixmlNode_getChildNodes(argnode);
	
		// test of the list size
		for(int i=1;i< ixmlNodeList_length(argNodeList);i++)
		{
			IXML_Node * subargnode = ixmlNodeList_item(argNodeList,i);
			IXML_NodeList * subargNodeList = ixmlNode_getChildNodes(subargnode);

			IXML_Node * argnamenode = ixmlNodeList_item(subargNodeList,0);
			IXML_Node * argdirectionnode = ixmlNodeList_item(subargNodeList,1);
			IXML_Node * argrelatednode = ixmlNodeList_item(subargNodeList,2);
	
			argumentName = QString(util_Xml_nodeValue(argnamenode));
			argumentDirection = QString(util_Xml_nodeValue(argdirectionnode));
			argumentRelatedStableVariable = QString(util_Xml_nodeValue(argrelatednode));
	
			action.addArgument(argumentName,argumentDirection,argumentRelatedStableVariable);
		}
	}


	bool find=false;
	this->m_actionList.begin();
	
	for(int i=0;i<this->m_actionList.size() && !find;i++)
	{
		
		if(this->m_actionList.last().name() == action.name())
		{
			find = true;
		}
	}
	if(find==false)
	{
		this->m_actionList.append(action);
	}
	else
	{
		printf("the action exist\n");
	}

}

void Service::viewActionList()
{
	printf("## affichage actions ##\n");
	this->m_actionList.begin();
	for(int i =0; i < this->m_actionList.size(); i++)
	{	
		Action tmp = this->m_actionList.last();
		printf("%s\n",tmp.name().toLatin1().data());
	}
}
