/*
    main.cpp -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>

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
#include <QtDebug>
#include <QtGlobal>
#include <QUrl>
// #include "upnpRouterPrivate.h"
// #include "upnpRouter.h"
int main(int argc, char *argv[])
{
	/*UPnpRouter router = UPnpRouter::defaultRouter();
	if(router.isValid())
		router.openPort(4000,QString("UDP"),QString("test"));*/	
	
	QApplication app(argc, argv);
  	ApplicationWidget fenetre;
  	fenetre.show();

  	return app.exec();
  	
}
