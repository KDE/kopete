/***************************************************************************
                          kopete.cpp  -  description
                             -------------------
    begin                : Wed Dec 26 03:12:10 CLST 2001
    copyright            : (C) 2001 by Duncan Mac-Vicar Prett
    email                : duncan@puc.cl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kopete.h"
#include <qlayout.h>
#include <kconfig.h>
#include "ui/preferencesdialog.h"

Kopete::Kopete(QWidget *parent, const char *name) : QWidget(parent, name)
{
	QBoxLayout *layout = new QBoxLayout(this,QBoxLayout::TopToBottom);
  QBoxLayout *layout2 = new QBoxLayout(this,QBoxLayout::LeftToRight);
  QBoxLayout *layout3 = new QBoxLayout(this,QBoxLayout::LeftToRight);

	contactlist = new ContactList(this);
	mainbutton = new QPushButton(this);
	
	addbutton = new QPushButton(this);
	globalbutton = new QPushButton(this);
	awaybutton = new QPushButton(this);
	otherbutton = new QPushButton(this);

	statuslabel = new QLabel(this);
  popupmenu = new KPopupMenu(this);
  icqpopupmenu = new KPopupMenu(this);
  msnpopupmenu = new KPopupMenu(this);

	mainbutton->setText("ICQ");

	addbutton->setText("+");
	globalbutton->setText("O");
	awaybutton->setText("N/A");
	otherbutton->setText("-");

	statuslabel->setText("offline");
	
	layout->addLayout(layout3);
	layout3->insertWidget(-1,addbutton);
	layout3->insertWidget(-1,globalbutton);
	layout3->insertWidget(-1,awaybutton);
	layout3->insertWidget(-1,otherbutton);
	
	layout->insertWidget(-1,contactlist);
	layout->addLayout(layout2);
	layout2->insertWidget(-1,mainbutton);
	layout2->insertWidget(-1,statuslabel);
	
	icqpopupmenu->insertTitle("ICQ");
	icqpopupmenu->insertItem("Go Online", this, SLOT(slotICQConnectByMenu()),0);

	msnpopupmenu->insertTitle("MSN Messenger");
	msnpopupmenu->insertItem("Go Online", this, SLOT(slotMSNConnectByMenu()),0);

	popupmenu->insertTitle("Connection");
	popupmenu->insertItem("&ICQ",icqpopupmenu,0,32423);
	popupmenu->insertItem("&MSN",msnpopupmenu,0,32453);
	popupmenu->insertTitle("Options");
	popupmenu->insertItem("&Preferences", this, SLOT(slotPreferences()),0);
	popupmenu->insertItem("&Exit", this, SLOT(slotExit()),0);
	mainbutton->setPopup(popupmenu);
	
}

Kopete::~Kopete()
{
}
/** No descriptions */
void Kopete::slotMSNConnectByMenu(){
}
/** No descriptions */
void Kopete::slotICQConnectByMenu(){
}
/** No descriptions */
void Kopete::slotPreferences()
{

	PreferencesDialog *dialog=new PreferencesDialog(0L,"settings");
  	dialog->show();
	connect(dialog,SIGNAL(closed()),this,SLOT(slotPrefDialogClosed()));
  
}
/** No descriptions */
void Kopete::slotExit()
{

}
/** No descriptions */
void Kopete::slotPrefDialogClosed()
{
}

/** No descriptions */
void Kopete::readOptions()
{
	/*
  KConfig *config = KGlobal::config();
  config->setGroup("General");
	visible = config->readBoolEntry("Visible",true);
	QSize size          = config->readSizeEntry("Geometry");
  config->readBoolEntry("Idle Detection",false);
  config->readNumEntry("MaxIdle",15);

  if(!size.isEmpty())
  {
    resize(size);
  }
  QPoint pos=config->readPointEntry("Position");
  if(!pos.isNull())
  {
    move(pos);
  }	
  config->setGroup("ICQ");
  QString icqNick = config->readEntry( "icqNick" );
  QString icqUIN = config->readEntry( "icqUIN" );
	QString icqPass = config->readEntry( "icqPass" );
	config->setGroup("MSN");
  QString msnNick = config->readEntry( "msnNick" );
  QString msnID = config->readEntry( "msnID" );
	QString msnPass = config->readEntry( "msnPass" );

	autoConnect=-1;
  if ( profiles.count() == 0 )
    slotSettings();
  else
  {
    //Construct menu...
    for ( int i = 0 ; i < profiles.count() ; i++ )
    {
      KAction *tmpaction = new KMsnAction( profiles[i] , //Name
                                        0           , //Accel
                                        this        , //Receiver
                                        SLOT(slotConnector( int ) ),
                                        actionCollection(),//Parent (not sure...)
                                        profiles[i] , //Internal name
                                        i );          //ID, needed for connecting
      //Add actions to the submenu
      fileConnector->insert( tmpaction );
      //And add them to our list...
      actions.append( tmpaction );
      //.. and make sure that they're deleted when we delete them
      actions.setAutoDelete( true );

      if (autoStr == profiles[i])
	autoConnect=i;
    }
  }


*/
}
/** No descriptions */
void Kopete::saveOptions(){
}
