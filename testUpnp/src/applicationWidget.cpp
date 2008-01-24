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

ApplicationWidget::ApplicationWidget(QWidget *parent): QMainWindow(parent)
{
	setupUi(this);
	this->upnp = UpnpKopete::getInstance();
	connect(btOpen, SIGNAL(clicked()),this, SLOT(openPort()));
	connect(btDelete, SIGNAL(clicked()),this, SLOT(deletePort()));
	connect(btEnvoi, SIGNAL(clicked()),this, SLOT(envoyer()));
	connect(actionQuitter, SIGNAL(clicked()),this, SLOT(close()));
	connect(btStatus,SIGNAL(clicked()),this, SLOT(statusInfos()));
	connect(btMapping,SIGNAL(clicked()),this, SLOT(portMapping()));
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
			this->upnp->openPort(QString("Test application"), port);
			text_mess->append("Port open : "+l_port->text());
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
			this->upnp->deletePort(port);
			text_mess->append("Port delete : "+l_port->text());
		}
		else
		{
			text_mess->append("the value isn't number");
		}
	}
}
void ApplicationWidget::statusInfos()
{
	text_mess->append("Status infos");
	this->upnp->statusInfo();
}

void ApplicationWidget::portMapping()
{
	text_mess->append("Port Mapping");
	this->upnp->portMapping();
}

void ApplicationWidget::envoyer()
{
	int ret;
	QString tmp;
	
	ret = this->upnp->upnpConnect();
	text_mess->append("######################################");
	text_mess->append("Research Device ...");

	if(ret != 0)
	{
		printf("Error research device %d\n",ret);
		UpnpFinish();
	}

	text_mess->append("Device location :");
	text_mess->append(this->upnp->descDoc());	
	text_mess->append("######################################");

	//this->upnp->viewDevice();

	
}



void ApplicationWidget::close()
{
	this->upnp->~UpnpKopete();
	printf("close\n");
}
