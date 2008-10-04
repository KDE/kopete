/*
    applicationWidget.cpp -

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
#include <QList> 
#include <QMessageBox>

ApplicationWidget::ApplicationWidget(QWidget *parent): QMainWindow(parent)
{
	setupUi(this);
	connect(btOpen, SIGNAL(clicked()),this, SLOT(openPort()));
	connect(btDelete, SIGNAL(clicked()),this, SLOT(deletePort()));
	connect(bt_search, SIGNAL(clicked()),this, SLOT(search()));
	router = UPnpRouter::allRouters();

}
void ApplicationWidget::search()
{
	
	qDebug()<<"avant router isValid";
	if(!router.isEmpty())
	{
		foreach(UPnpRouter r,router)
			text_mess->append("Router Url : "+r.url().toString());
	}
	else
	{
		qDebug()<<"message";
		int box = QMessageBox::warning(this, "UPnp Probleme", "No upnp device found");
	}
}

void ApplicationWidget::openPort()
{
	bool val;
	int port;
	if(l_port->text().isEmpty())
	{
		text_mess->append("Choose port");
	}
	else
	{
		port=l_port->text().toInt(&val, 10);
		if(val==true)
		{
			foreach(UPnpRouter r, router)
			{ 
				if(r.isValid())
				{
					r.openPort(port, QString("TCP"), QString("test"));
					text_mess->append("Port open : "+l_port->text());
				}
			}
		}
		else
		{
			text_mess->append("the value isn't number");
		}
	}
}

void ApplicationWidget::deletePort()
{
	bool val;
	int port;
	if(l_port->text().isEmpty())
	{
		text_mess->append("Choose port");
	}
	else
	{
		port=l_port->text().toInt(&val, 10);
		if(val==true)
		{ 
			foreach(UPnpRouter r, router)
			{ 
				if (r.isValid())
				{
					r.closePort(port, QString("TCP"));
					text_mess->append("Port delete : "+l_port->text());
				}
			}
		}
		else
		{
			text_mess->append("the value isn't number");
		}
	}
}
