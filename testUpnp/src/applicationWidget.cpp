#include "applicationWidget.h"
#include <QList> 

ApplicationWidget::ApplicationWidget(QWidget *parent): QMainWindow(parent)
{
	setupUi(this);
	this->upnp = UpnpKopete::getInstance();
	//connect(btEnvoi, SIGNAL(clicked()),this, SLOT(afficher()));
	connect(btEnvoi, SIGNAL(clicked()),this, SLOT(envoyer()));
	connect(actionQuitter, SIGNAL(clicked()),this, SLOT(close()));
}


void ApplicationWidget::afficher()
{
	/*QString mess = text_ecrit->toPlainText();
	text_mess->append(mess);
	text_ecrit->setPlainText("");*/
	
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

// 	QList<Device> device = this->upnp->getMainDevices();
// 	for(int i=0; i<device.size();i++)
// 	{
// 		QTreeWidgetItem* item = new QTreeWidgetItem(treeDevice);
// 		item->setText(i, tr("Device"));
// 		//treeDevice->insertTopLevelItem(i,item);
// 	}
	QList<QString> liste = this->upnp->viewXMLDescDoc();
	for(int i =0; i < liste.size(); i++)
	{	
		tmp = liste.last();
		text_mess->append("Device location :");
		text_mess->append(tmp);
	}	
	text_mess->append("######################################");

	this->upnp->viewListDevice();
	
}

void ApplicationWidget::close()
{
	this->upnp->~UpnpKopete();
	printf("close\n");
}
