/*
    netmeetingplugin.cpp

    Copyright (c) 2003-2004 by Olivier Goffart <ogoffart@tiscalinet.be>

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
#include <kdeversion.h>
#include <kaboutdata.h>

#include "kopetepluginmanager.h"
#include "kopetemessagemanagerfactory.h"

#include "msnmessagemanager.h"
#include "msnprotocol.h"
#include "msncontact.h"

#include "netmeetinginvitation.h"
#include "netmeetingguiclient.h"


#if KDE_IS_VERSION(3,2,90)
static const KAboutData aboutdata("kopete_netmeeting", I18N_NOOP("NetMeeting") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( kopete_netmeeting, KGenericFactory<NetMeetingPlugin>( &aboutdata )  )
#else
K_EXPORT_COMPONENT_FACTORY( kopete_netmeeting, KGenericFactory<NetMeetingPlugin>( "kopete_netmeeting" )  )
#endif

NetMeetingPlugin::NetMeetingPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: Kopete::Plugin( KGlobal::instance(), parent, name )
{
	if(MSNProtocol::protocol())
		slotPluginLoaded(MSNProtocol::protocol());
	else
		connect(Kopete::PluginManager::self() , SIGNAL(pluginLoaded(Kopete::Plugin*) ), this, SLOT(slotPluginLoaded(Kopete::Plugin*)));


	connect( KopeteMessageManagerFactory::factory(), SIGNAL( messageManagerCreated( Kopete::MessageManager * )) , SLOT( slotNewKMM( Kopete::MessageManager * ) ) );
	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QIntDict<Kopete::MessageManager> sessions = KopeteMessageManagerFactory::factory()->sessions();
	QIntDictIterator<Kopete::MessageManager> it( sessions );
	for ( ; it.current() ; ++it )
	{
		slotNewKMM(it.current());
	}
}

NetMeetingPlugin::~NetMeetingPlugin()
{

}

void NetMeetingPlugin::slotPluginLoaded(Kopete::Plugin *p)
{
	if(p->pluginId()=="MSNProtocol")
	{
		connect( p , SIGNAL(invitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNMessageManager*  , MSNContact* )) ,
			this, SLOT( slotInvitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNMessageManager*  , MSNContact* )));
	}
}

void NetMeetingPlugin::slotNewKMM(Kopete::MessageManager *KMM)
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
