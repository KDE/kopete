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
	int ret;
// 	IXML_Document *DescDoc = ixmlLoadDocument("XML/gatedesc.xml");
// 	if(DescDoc == NULL)
// 	{
// 		printf("error loadDocument\n");
// 	}
// 	IXML_NodeList * gg = ixmlDocument_getElementsByTagNameNS(DescDoc,"*","root/device/deviceType");
// 	//IXML_Node *node = ixmlDocument
// 
// 	//printf("%s\n",util_Xml_nodeValue(ixmlNodeList_item(gg,0)));
// 
// 	//ixmlNode_getNodeValue(ixmlNodeList_item(gg,0)));
// 	ixmlPrintNode(ixmlNodeList_item(gg,0));
// 	printf("%d\n",ixmlNodeList_length(gg));
  	
}
