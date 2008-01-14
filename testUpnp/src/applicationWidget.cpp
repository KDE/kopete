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
void ApplicationWidget::envoyer()
{
	int ret;
	QString tmp;
	
	ret = this->upnp->researchDevice();
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

	this->upnp->viewDevice();

	
}

void ApplicationWidget::close()
{
	this->upnp->~UpnpKopete();
	printf("close\n");
}
