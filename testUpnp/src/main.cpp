#include "applicationWidget.h"
#include <QApplication>
#include <QtGui>

#include <stdio.h>
#include <string.h>

#include <upnp/ixml.h>
#include <upnp/FreeList.h>
#include <upnp/iasnprintf.h>
#include <upnp/ithread.h>
#include <upnp/LinkedList.h>
#include <upnp/ThreadPool.h>
#include <upnp/TimerThread.h>
#include <upnp/upnpconfig.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "util_Xml.h"

#include <QList>
#include <QString>

//#include "sample_util.h"

int main(int argc, char *argv[])
{
  	
	QApplication app(argc, argv);
  	ApplicationWidget fenetre;
  	fenetre.show();

  	return app.exec();
// 	int ret;
// 	IXML_Document *DescDoc = ixmlLoadDocument("XML/gatedesc.xml");
// 	if(DescDoc == NULL)
// 	{
// 		printf("error loadDocument\n");
// 	}
// 	IXML_NodeList * listxml = ixmlDocument_getElementsByTagName(DescDoc,"deviceType");
// 	printf("nombre de node %d\n",ixmlNodeList_length(listxml));
// 	IXML_Node* node = ixmlNodeList_item(listxml,1);
// 	printf("node : %s URI: %s\n",ixmlNode_getNodeName(node),ixmlNode_getNamespaceURI(node));
// 	IXML_Node* parent =ixmlNode_getParentNode(node);
// 	printf("parent : %s\n",ixmlNode_getNodeName(parent));
// 	IXML_Document *Desparent = ixmlNode_getOwnerDocument(parent);
// ;
// 	IXML_NodeList * listDevice = ixmlDocument_getElementsByTagName(Desparent,"device");
// 	IXML_Node* nodes = ixmlNodeList_item(listDevice,0);
// 	
// 	IXML_NodeList* child = ixmlNode_getChildNodes(nodes);
// 	for(int i = 0;i<ixmlNodeList_length(child);i++)
// 	{
// 		IXML_Node* n = ixmlNodeList_item(child,i);
// 		
// 		printf("node %d : %s : %s\n",i,ixmlNode_getNodeName(n),util_Xml_nodeValue(n));
// 		
// 
// 		
// 	}

	
	

};
/*
IXML_Node * searchTagParentDevice(IXML_Document* DescDoc, char * deviceType)
{
	IXML_NodeList * parent = ixmlDocument_getElementsByTagName(DescDoc,"root");
}

//return the number of node that is a name "device"
QList<int> searchTagDevice(IXML_Node * node)
{
	QList<int> listretour = NULL;
	listretour.begin();

	IXML_NodeList* child = ixmlNode_getChildNodes(node);
	for(int i = 0;i<ixmlNodeList_length(child);i++)
	{
		IXML_Node* n = ixmlNodeList_item(child,i);
		if(strcmp(ixmlNode_getNodeName(n),"device")==0)
		{
			printf("node %d : %s\n",i,ixmlNode_getNodeName(node));
			listretour.append(i);
		}
	}
}

//return the number of node that is a name "deviceList"
QList<int> searchTagDevice(IXML_Node * node)
{
	QList<int> listretour = NULL;
	listretour.begin();

	IXML_NodeList* child = ixmlNode_getChildNodes(node);
	for(int i = 0;i<ixmlNodeList_length(child);i++)
	{
		IXML_Node* n = ixmlNodeList_item(child,i);
		if(strcmp(ixmlNode_getNodeName(n),"deviceList")==0)
		{
			printf("node %d : %s\n",i,ixmlNode_getNodeName(node));
			listretour.append(i);
		}
	}
}*/
