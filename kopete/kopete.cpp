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

Kopete::Kopete(): KUniqueApplication(true, true, true)
{
	mainwidget = new QWidget(0);
	setMainWidget(mainwidget);
	mPref=new PreferencesDialog(0);
	aboutPl = new AboutPlugins(mainwidget);
  mPref->hide();
  aboutPl->hide();
	
	plugins = new PluginManager();
	QBoxLayout *layout = new QBoxLayout(mainwidget,QBoxLayout::TopToBottom);
  QBoxLayout *layout2 = new QBoxLayout(mainwidget,QBoxLayout::LeftToRight);
  QBoxLayout *layout3 = new QBoxLayout(mainwidget,QBoxLayout::LeftToRight);

	contactlist = new ContactList(mainwidget);
	mainbutton = new QPushButton(mainwidget);
	
	addbutton = new QPushButton(mainwidget);
	globalbutton = new QPushButton(mainwidget);
	awaybutton = new QPushButton(mainwidget);
	otherbutton = new QPushButton(mainwidget);

	statuslabel = new QLabel(mainwidget);
  popupmenu = new KPopupMenu(mainwidget);

	mainbutton->setText("Kopete");

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
	
	popupmenu->insertTitle("Kopete");
	popupmenu->insertItem("&Connect", this, SLOT(slotConnectAll()),0);
	popupmenu->insertSeparator();
	popupmenu->insertItem("&Preferences", this, SLOT(slotPreferences()),0);
	popupmenu->insertItem("About Plugins", aboutPl, SLOT(show()),0);
	popupmenu->insertSeparator();
	popupmenu->insertItem("&Exit", this, SLOT(slotExit()),0);
	
	mainbutton->setPopup(popupmenu);
	mainwidget->show();
	
}

Kopete::~Kopete()
{
}

void Kopete::slotPreferences()
{
  mPref->show();
  mPref->raise();
}
/** No descriptions */
void Kopete::slotExit()
{
	quit();
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
/** No descriptions */
void Kopete::slotConnectAll()
{
}
