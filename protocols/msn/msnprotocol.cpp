/***************************************************************************
                          msnprotocol.cpp  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "msnprotocol.h"
#include "msncontact.h"
#include <msnadd.h>
#include "kopete.h"
#include <msnaddcontactpage.h>

///////////////////////////////////////////////////
//           Constructor & Destructor
///////////////////////////////////////////////////

MSNProtocol::MSNProtocol(): QObject(0, "MSN"), IMProtocol()
{
	// Remember to move all this to init()

	kdDebug() << "\nMSN Plugin Loading\n";
	// Load all ICQ icons from KDE standard dirs
 	initIcons();
	
	kdDebug() << "MSN Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();
	
	kdDebug() << "MSN Protocol Plugin: Setting icon offline\n";
	statusBarIcon->setPixmap(&offlineIcon);

	kdDebug() << "MSN Protocol Plugin: Creating Config Module\n";
	new MSNPreferences(protocolIcon, this);
	
	kdDebug() << "MSN Protocol Plugin: Creating MSN Engine\n";
	engine = new MSN;
	connect(engine, SIGNAL(connectedToMsn(bool)), this, SLOT(slotConnectedToMSN(bool)));
	connect(engine, SIGNAL(userStateChange (QString, QString, QString)), this, SIGNAL(userStateChange (QString, QString, QString) ) );
	//connect(engine, SIGNAL(userStateChange (QString, QString, QString)), this, SLOT(slotUserStateChange (QString, QString, QString) ) );
	//connect(engine, SIGNAL(userStateChange (QString, QString, QString)), this, SLOT(slotInitContacts(QString, QString, QString) ) );
	//connect(engine, SIGNAL(userSetOffline (QString) ), this, SLOT(slotUserSetOffline(QString) ) );
	connect(engine, SIGNAL(newUserFound (QString, QString) ), this, SLOT(slotNewUserFound(QString, QString) ) );
	kdDebug() << "MSN Protocol Plugin: Done\n";

	KGlobal::config()->setGroup("MSN");

	/** Autoconnect if is selected in config */
	if ( KGlobal::config()->readBoolEntry("AutoConnect", "0") )
	{
		Connect();
	}
	
}

MSNProtocol::~MSNProtocol()
{

}

///////////////////////////////////////////////////
//           Plugin Class reimplementation
///////////////////////////////////////////////////

void MSNProtocol::init()
{

}

bool MSNProtocol::unload()
{
	kdDebug() << "MSN Protocol: Unloading...\n";
	kopeteapp->statusBar()->removeWidget(statusBarIcon);
	delete statusBarIcon;
	// heh!
	return 1;
}

///////////////////////////////////////////////////
//           IMProtocol Class reimplementation
///////////////////////////////////////////////////

void MSNProtocol::Connect()
{
	if ( !isConnected() )
	{
		KGlobal::config()->setGroup("MSN");
		kdDebug() << "Attempting to connect to MSN" << endl;
		kdDebug() << "Setting Monopoly mode..." << endl;
		kdDebug() << "Using Micro$oft UserID " << KGlobal::config()->readEntry("UserID", "0") << " with password " << KGlobal::config()->readEntry("Password", "") << endl;
		KGlobal::config()->setGroup("MSN");

		engine->setUser(KGlobal::config()->readEntry("UserID", "")
			,KGlobal::config()->readEntry("Password", "")
			,KGlobal::config()->readEntry("Nick", "")
			);
		engine->slotConnect();
	}
	else
	{
    	kdDebug() << "MSN Plugin: Ignoring Connect request (Already Connected)" << endl;
	}	
}

void MSNProtocol::Disconnect()
{
	if ( isConnected() )
	{
		engine->slotDisconnect();
	}
	else
	{
    	kdDebug() << "MSN Plugin: Ignoring Disconnect request (Im not Connected)" << endl;
	}	
}


bool MSNProtocol::isConnected()
{
	return engine->isConnected;	
}

/** This i used for al protocol selection dialogs */
QPixmap MSNProtocol::getProtocolIcon()
{
	return protocolIcon;
}

AddContactPage *MSNProtocol::getAddContactWidget(QWidget *parent)
{
	return (new MSNAddContactPage(this,parent));	
}

///////////////////////////////////////////////////
//           Internal functions implementation
///////////////////////////////////////////////////

/** No descriptions */
void MSNProtocol::initIcons()
{
	KIconLoader *loader = KGlobal::iconLoader();

	protocolIcon = QPixmap(loader->loadIcon("msn_protocol", KIcon::User));
	onlineIcon = QPixmap(loader->loadIcon("msn_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("msn_offline", KIcon::User));
	awayIcon = QPixmap(loader->loadIcon("msn_away", KIcon::User));
	naIcon = QPixmap(loader->loadIcon("msn_na", KIcon::User));
}

/** No descriptions */
void MSNProtocol::slotConnected()
{
		statusBarIcon->setPixmap(&onlineIcon);
}

void MSNProtocol::slotDisconnected()
{
		statusBarIcon->setPixmap(&offlineIcon);
}

void MSNProtocol::slotConnectedToMSN(bool c)
{
		if (c)
		{
			slotConnected();
		}
		else
		{
			slotDisconnected();
		}
}

void MSNProtocol::slotUserStateChange (QString st1, QString st2, QString st3)
{
	kdDebug() << "MSN Plugin: User State change " << st1 << " " << st2 << " " << st3 <<"\n";
}

void MSNProtocol::slotInitContacts (QString status, QString userid, QString nick)
{
	kdDebug() << "MSN Plugin: User State change " << status << " " << userid << " " << nick <<"\n";
	if ( status == "NLN" )
	{
		MSNContact *newContact = new MSNContact(kopeteapp->contactList()->onlineBranch, userid, nick, this);
		newContact->setPixmap(0,onlineIcon);
	}
}


void MSNProtocol::slotUserSetOffline (QString str)
{
	kdDebug() << "MSN Plugin: User Set Offline " << str << "\n";
		
}

void MSNProtocol::slotNewUserFound (QString userid, QString nick)
{
	kdDebug() << "MSN Plugin: User found " << userid << " " << nick <<"\n";
	MSNContact *newContact = new MSNContact(kopeteapp->contactList()->offlineBranch, userid, nick, this);
	newContact->setPixmap(0,offlineIcon);		

}		