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

#include <qlayout.h>
#include <kconfig.h>

#include <kopete.h>
#include <preferencesdialog.h>
#include <kopetewindow.h>
#include "plugin.h"
#include "pluginloader.h"
#include "pluginmodule.h"

class Plugins;

Kopete::Kopete(): KUniqueApplication(true, true, true)
{
	plugins = new PluginManager;
	mLibraryLoader = new LibraryLoader;
	mPref=new PreferencesDialog;
	//new Plugins(this);

	mainwindow = new KopeteWindow;
	setMainWidget(mainwindow);
	
  mPref->hide();
	
	
	
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
/** No descriptions */
void Kopete::slotAboutPlugins()
{
	AboutPlugins *aboutPl;
	aboutPl = new AboutPlugins(mainwindow);
	aboutPl->show();
	
}
/** No descriptions */
void Kopete::slotAddContact()
{
}

/** No descriptions */
void Kopete::slotSetAway()
{
}/** No descriptions */
void Kopete::initPlugins()
{
	
}
/** No descriptions */
void Kopete::loadPlugins()
{
	mLibraryLoader->loadAll();
}
