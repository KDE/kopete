/*
    main.cpp -

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
// 	IXML_NodeList * gg = ixmlDocument_getElementsByTagNameNS(DescDoc,"*","root/device/deviceType");
// 	//IXML_Node *node = ixmlDocument
// 
// 	//printf("%s\n",util_Xml_nodeValue(ixmlNodeList_item(gg,0)));
// 
// 	//ixmlNode_getNodeValue(ixmlNodeList_item(gg,0)));
// 	ixmlPrintNode(ixmlNodeList_item(gg,0));
// 	printf("%d\n",ixmlNodeList_length(gg));
  	
}
