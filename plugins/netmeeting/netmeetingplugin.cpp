/*
    netmeetingplugin.cpp

    Copyright (c) 2003 by Olivier Goffart <ogoffart@tiscalinet.be>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "netmeetingplugin.h"

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kaction.h>

#include "pluginloader.h"
#include "kopetemessagemanagerfactory.h"

#include "msnmessagemanager.h"
#include "msnprotocol.h"
#include "msncontact.h"

#include "netmeetinginvitation.h"
#include "netmeetingguiclient.h"


K_EXPORT_COMPONENT_FACTORY( kopete_netmeeting, KGenericFactory<NetMeetingPlugin>( "kopete_netmeeting" )  );

NetMeetingPlugin::NetMeetingPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: KopetePlugin( KGlobal::instance(), parent, name )
{
	if(MSNProtocol::protocol())
		slotPluginLoaded(MSNProtocol::protocol());
	else
		connect(LibraryLoader::self() , SIGNAL(pluginLoaded(KopetePlugin*) ), this, SLOT(slotPluginLoaded(KopetePlugin*)));


	connect( KopeteMessageManagerFactory::factory(), SIGNAL( messageManagerCreated( KopeteMessageManager * )) , SLOT( slotNewKMM( KopeteMessageManager * ) ) );
	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QIntDict<KopeteMessageManager> sessions = KopeteMessageManagerFactory::factory()->sessions();
	QIntDictIterator<KopeteMessageManager> it( sessions );
	for ( ; it.current() ; ++it )
	{
		slotNewKMM(it.current());
	}
}

NetMeetingPlugin::~NetMeetingPlugin()
{

}

void NetMeetingPlugin::slotPluginLoaded(KopetePlugin *p)
{
	if(p->pluginId()=="MSNProtocol")
	{
		connect( p , SIGNAL(invitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNMessageManager*  , MSNContact* )) ,
			this, SLOT( slotInvitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNMessageManager*  , MSNContact* )));
	}
}

void NetMeetingPlugin::slotNewKMM(KopeteMessageManager *KMM)
{
	MSNMessageManager *msnMM=dynamic_cast<MSNMessageManager*>(KMM);
	if(msnMM)
	{
		connect(this , SIGNAL( destroyed(QObject*)) ,
			new NetMeetingGUIClient(msnMM)
		 , SLOT(deleteLater()));
	}
}


void NetMeetingPlugin::slotInvitation(MSNInvitation*& invitation,  const QString &bodyMSG , long unsigned int /*cookie*/ , MSNMessageManager* msnMM , MSNContact* c )
{
	if(!invitation &&  bodyMSG.contains(NetMeetingInvitation::applicationID()))
	{
		invitation=new NetMeetingInvitation(true,c,msnMM);
		invitation->parseInvitation(bodyMSG);
	}
}

#include "netmeetingplugin.moc"
