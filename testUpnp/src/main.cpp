#include "applicationWidget.h"
#include <QApplication>
#include <QtGui>

#include <stdio.h>
#include <string.h>

#include <ixml.h>
#include <FreeList.h>
#include <ithread.h>
#include <LinkedList.h>
#include <ThreadPool.h>
#include <TimerThread.h>
#include <upnpconfig.h>
#include <upnp.h>
#include <upnptools.h>

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
// 	IXML_NodeList * gg = ixmlDocument_getElementsByTagNameNS(DescDoc,"*","root/device/deviceType");
// 	//IXML_Node *node = ixmlDocument
// 
// 	//printf("%s\n",util_Xml_nodeValue(ixmlNodeList_item(gg,0)));
// 
// 	//ixmlNode_getNodeValue(ixmlNodeList_item(gg,0)));
// 	ixmlPrintNode(ixmlNodeList_item(gg,0));
// 	printf("%d\n",ixmlNodeList_length(gg));
  	
}
