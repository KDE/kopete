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
#include "kopete.moc"

#include <qvaluelist.h>
#include <qlist.h>
#include <qlayout.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kcrash.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

#include <preferencesdialog.h>
#include <kopetewindow.h>

#include "ui/appearanceconfig.h"
#include "plugin.h"
#include "pluginloader.h"
#include "pluginmodule.h"
#include <addwizardimpl.h>
#include "systemtray.h"

class Plugins;

Kopete::Kopete(): KUniqueApplication(true, true, true)
{
	//plugins = new PluginManager;
	allConnected = false;
    initEmoticons();

	mLibraryLoader = new LibraryLoader();
	mIconLoader = KGlobal::iconLoader();
	mPref = new PreferencesDialog();
	mPref->hide();

	Plugins *blah = new Plugins(this);

	mainwindow = new KopeteWindow();
	setMainWidget(mainwindow);

	mAppearance = new AppearanceConfig(mainwindow);

	KConfig *config=KGlobal::config();
	config->setGroup("");
	/* Ups! the user dont have plugins selected. */
	if (!config->hasKey("Modules"))
	{
		QStringList modules;
		modules.append("icq.plugin");
		modules.append("msn.plugin");
		config->writeEntry("Modules", modules);
	}

	tray = new KopeteSystemTray();
	tray->getContextMenu()->insertSeparator();
	mainwindow->actionAddContact->plug( tray->getContextMenu() );
	tray->getContextMenu()->insertSeparator();
	mainwindow->actionConnect->plug( tray->getContextMenu() );
	mainwindow->actionDisconnect->plug( tray->getContextMenu() );
	mainwindow->actionPrefs->plug( tray->getContextMenu() );
	tray->getContextMenu()->insertSeparator();

	/* Ok, load saved plugins */
	loadPlugins();
}


Kopete::~Kopete()
{
	kdDebug() << "Kopete::~Kopete()" << endl;

	if (mPref)
		delete mPref;

	if (mLibraryLoader)
		delete mLibraryLoader;

	if (tray)
	{
		delete tray;
	}

	if (mainwindow)
		delete mainwindow;


	// Only use this if cant find crash cause :-)
	//KCrash::setCrashHandler(Kopete::cleverKCrashHack);
	kdDebug() << "END OF Kopete::~Kopete()" << endl;
}


void Kopete::slotPreferences()
{
	mPref->show();
	mPref->raise();
}


/** No descriptions */
void Kopete::slotExit()
{
	kdDebug() << "Kopete::slotExit()" << endl;
	quit();
}


/** No descriptions */
void Kopete::readOptions()
{
	kdDebug() << "Kopete::readOptions()" << endl;
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
void Kopete::saveOptions()
{
}

/** Connect all loaded protocol plugins */
void Kopete::slotConnectAll()
{
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
    for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "Kopete: Connect All: "<<(*i).name << endl;
		Plugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;				
		IMProtocol *prot =  static_cast<IMProtocol*>(tmpprot);
		if ( !(prot->isConnected()))
		{
			prot->Connect();
		}
	}
}

/** Disconnect all loaded protocol plugins */
void Kopete::slotDisconnectAll()
{
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
    for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "Kopete: Disconnect All: "<<(*i).name << endl;
		Plugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		IMProtocol *prot =  static_cast<IMProtocol*>(tmpprot);
		if (prot->isConnected())
		{
			prot->Disconnect();
		}
	}
}

/** No descriptions */
void Kopete::slotAboutPlugins()
{
	AboutPlugins *aboutPl;
	aboutPl = new AboutPlugins(mainwindow);
	aboutPl->show();
}

/** Add a contact through Wizard */
void Kopete::slotAddContact()
{
	AddWizardImpl *tmpdialog = new AddWizardImpl( mainWindow() );
	tmpdialog->show();
}

/** No descriptions */
void Kopete::slotSetAway()
{
}

/** No descriptions */
void Kopete::initPlugins()
{
}

/** No descriptions */
void Kopete::loadPlugins()
{
	mLibraryLoader->loadAll();
}

void Kopete::cleverKCrashHack(int)
{
	// do nothing

	// Understand that the KDE libraries have a memory leak, and
	// the playlist cannot be unloaded without causing a crash
	// in QApplication::windowMapper() or something similar.
	// this is just to prevent the KCrash window from appearing
	// and bugging the user regularly

	// someone fix the libraries.
	kdDebug() << "Crashed.\n" << endl;
	_exit(255);
}

void Kopete::initEmoticons()
{
	KStandardDirs dir;
	KConfig *config=KGlobal::config();
    config->setGroup("Appearance");
    mEmoticonTheme = config->readEntry("EmoticonTheme", "Default");
	/* Happy emoticons */
	/* :-) */
	mEmoticons.smile = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/smile.mng");
	if ( mEmoticons.smile.isNull() )
		mEmoticons.smile = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/smile.png");
	/* ;-) */
	mEmoticons.wink = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/wink.mng");
    if ( mEmoticons.wink.isNull() )
		mEmoticons.wink = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/wink.png");
	/* :-P */
	mEmoticons.tongue = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/tongue.mng");
	if ( mEmoticons.tongue.isNull() )
		mEmoticons.tongue = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/tongue.png");
	/* :-D*/
	mEmoticons.biggrin = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/bigrin.mng");
	if ( mEmoticons.biggrin.isNull() )
		mEmoticons.biggrin = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/biggrin.png");
	
	/* Sad emoticons */
	mEmoticons.unhappy = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/unhappy.mng");
	if ( mEmoticons.unhappy.isNull() )
		mEmoticons.unhappy = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/unhappy.png");
	
	mEmoticons.cry = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/cry.mng");
    if ( mEmoticons.cry.isNull() )
		mEmoticons.cry = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/cry.png");
	
	/* Surprise */
	mEmoticons.oh = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/oh.mng");
    if ( mEmoticons.oh.isNull() )
		mEmoticons.oh = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/oh.png");
	
}

QString Kopete::parseEmoticons( QString message )
{
	if ( !mEmoticons.smile.isNull() )
	{
		message = message.replace(QRegExp(":-\\)"),"<img src=\""+mEmoticons.smile+"\">");
		message = message.replace(QRegExp(":\\)"),"<img src=\""+mEmoticons.smile+"\">");
	}
	if ( !mEmoticons.wink.isNull() )
	{
		message = message.replace(QRegExp(";-\\)"),"<img src=\""+mEmoticons.wink+"\">");
		message = message.replace(QRegExp(";\\)"),"<img src=\""+mEmoticons.wink+"\">");
    }
	if ( !mEmoticons.tongue.isNull() )
	{
		message = message.replace(QRegExp(":p"),"<img src=\""+mEmoticons.tongue+"\">");
		message = message.replace(QRegExp(":P"),"<img src=\""+mEmoticons.tongue+"\">");
		message = message.replace(QRegExp(":-p"),"<img src=\""+mEmoticons.tongue+"\">");
		message = message.replace(QRegExp(":-P"),"<img src=\""+mEmoticons.tongue+"\">");
	}
	if ( !mEmoticons.biggrin.isNull() )
	{
		message = message.replace(QRegExp(":D"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":d"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":-D"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":-d"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":>"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":->"),"<img src=\""+mEmoticons.biggrin+"\">");
	}
    if ( !mEmoticons.unhappy.isNull() )
	{
		message = message.replace(QRegExp(":-\\("),"<img src=\""+mEmoticons.unhappy+"\">");
		message = message.replace(QRegExp(":\\("),"<img src=\""+mEmoticons.unhappy+"\">");
    }
	if ( !mEmoticons.cry.isNull() )
	{
		message = message.replace(QRegExp(":'-\\("),"<img src=\""+mEmoticons.cry+"\">");
		message = message.replace(QRegExp(":'\\("),"<img src=\""+mEmoticons.cry+"\">");
    }
	if ( !mEmoticons.oh.isNull() )
	{
		message = message.replace(QRegExp(":o"),"<img src=\""+mEmoticons.oh+"\">");
		message = message.replace(QRegExp(":O"),"<img src=\""+mEmoticons.oh+"\">");
		message = message.replace(QRegExp(":-o"),"<img src=\""+mEmoticons.oh+"\">");
		message = message.replace(QRegExp(":-O"),"<img src=\""+mEmoticons.oh+"\">");
	}
	#warning "TODO: Sleep emoticon parsing pending"
	
	return message;
}

QString Kopete::parseHTML( QString message )
{
}

QString Kopete::parseURL( QString message )
{
}
