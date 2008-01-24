/*
    service.cpp -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>
    Copyright (c) 2007-2008 by Julien Hubatzeck   <reineur31@gmail.com>
    Copyright (c) 2007-2008 by Michel Saliba      <msalibaba@gmail.com>

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

#include "service.h"

Service::Service()
{
}


QString Service::serviceType()
{
	return this->m_serviceType;
}
QString Service::serviceId()
{
	return this->m_serviceId;
}
QString Service::controlURL()
{
	return this->m_controlURL;
}
QString Service::eventSubURL()
{
	return this->m_eventSubURL;
}
QString Service::xmlDocService()
{
	return this->m_xmlDocService;
}
QList<Action>* Service::actionList()
{
	return &(this->m_actionList);
}


void Service::setServiceType(QString serviceType)
{
	this->m_serviceType = serviceType;
}
void Service::setServiceId(QString serviceId)
{
	this->m_serviceId = serviceId;
}
void Service::setControlURL(QString controlURL)
{
	this->m_controlURL = controlURL;
}
void Service::setEventSubURL(QString eventSubURL)
{
	this->m_eventSubURL = eventSubURL;
}
void Service::setXmlDocService(QString URLdocXml)
{
	this->m_xmlDocService =URLdocXml;
}

void Service::addAllActions()
{
	IXML_Document * doc ;
	if((UpnpDownloadXmlDoc(this->m_xmlDocService.toLatin1().data(),&doc)) != UPNP_E_SUCCESS ) 
	{
		printf("Error \n");exit(1);
	}
	IXML_NodeList * nodelist = ixmlDocument_getElementsByTagName(doc,"action");
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
	QString actionName;
	QString argumentName;
	QString argumentDirection;
	QString argumentRelatedStableVariable;
	
	//creating action
	IXML_NodeList * actionNodeList = ixmlNode_getChildNodes(actionNode);
	IXML_Node * node = ixmlNodeList_item(actionNodeList,0);
	Action action = Action(QString(util_Xml_nodeValue(node)));

	//test of the actionNodeList size
	if (ixmlNodeList_length(actionNodeList) == 2)
	{
		IXML_Node * argnode = ixmlNodeList_item(actionNodeList,1);
		IXML_NodeList * argNodeList = ixmlNode_getChildNodes(argnode);
	
		// test of the list size
		for(int i=0;i< ixmlNodeList_length(argNodeList);i++)
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
		Action action_tmp = this->m_actionList.at(i);	
		if(action_tmp.name() == action.name())
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


bool Service::existAction(QString nameAction)
{
	bool find = false;	
	for(int i=0;i<this->actionList()->size(); i++)
	{
		Action action_tmp = this->actionList()->at(i);	
		if(action_tmp.name() == nameAction)
		{
			find = true;
		}
	}
	return find;
}

Action Service::actionByName(QString nameAction)
{
	Action action_tmp = Action();
	bool find = false;	
	for(int i=0;i<this->m_actionList.size() && !find; i++)
	{
		action_tmp = this->m_actionList.at(i);	
		if(action_tmp.name() == nameAction)
		{
			find = true;
			
		}
	}
	return action_tmp;
}

void Service::viewActionList()
{
	printf("## Displaying actions ##\n");
	for(int i =0; i < this->m_actionList.size(); i++)
	{
		Action action = this->m_actionList.at(i);
		printf("%s \n",action.name().toLatin1().data());
		//action.viewListArgument();
	}
}
