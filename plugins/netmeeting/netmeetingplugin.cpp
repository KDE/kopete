/*
    netmeetingplugin.cpp

    Copyright (c) 2003-2004 by Olivier Goffart <ogoffart @ kde.org>

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
#include "kopetechatsessionmanager.h"

#include "msnchatsession.h"
#include "msnprotocol.h"
#include "msncontact.h"

#include "netmeetinginvitation.h"
#include "netmeetingguiclient.h"


static const KAboutData aboutdata("kopete_netmeeting", I18N_NOOP("NetMeeting") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( kopete_netmeeting, KGenericFactory<NetMeetingPlugin>( &aboutdata )  )

NetMeetingPlugin::NetMeetingPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: Kopete::Plugin( KGlobal::instance(), parent, name )
{
	if(MSNProtocol::protocol())
		slotPluginLoaded(MSNProtocol::protocol());
	else
		connect(Kopete::PluginManager::self() , SIGNAL(pluginLoaded(Kopete::Plugin*) ), this, SLOT(slotPluginLoaded(Kopete::Plugin*)));


	connect( Kopete::ChatSessionManager::self(), SIGNAL( chatSessionCreated( Kopete::ChatSession * )) , SLOT( slotNewKMM( Kopete::ChatSession * ) ) );
	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QValueList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	for (QValueListIterator<Kopete::ChatSession*> it= sessions.begin(); it!=sessions.end() ; ++it)
	{
		slotNewKMM(*it);
	}
}

NetMeetingPlugin::~NetMeetingPlugin()
{

}

void NetMeetingPlugin::slotPluginLoaded(Kopete::Plugin *p)
{
	if(p->pluginId()=="MSNProtocol")
	{
		connect( p , SIGNAL(invitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNChatSession*  , MSNContact* )) ,
			this, SLOT( slotInvitation(MSNInvitation*& ,  const QString & , long unsigned int , MSNChatSession*  , MSNContact* )));
	}
}

void NetMeetingPlugin::slotNewKMM(Kopete::ChatSession *KMM)
{
	MSNChatSession *msnMM=dynamic_cast<MSNChatSession*>(KMM);
	if(msnMM)
	{
		connect(this , SIGNAL( destroyed(QObject*)) ,
			new NetMeetingGUIClient(msnMM)
		 , SLOT(deleteLater()));
	}
}


void NetMeetingPlugin::slotInvitation(MSNInvitation*& invitation,  const QString &bodyMSG , long unsigned int /*cookie*/ , MSNChatSession* msnMM , MSNContact* c )
{
	if(!invitation &&  bodyMSG.contains(NetMeetingInvitation::applicationID()))
	{
		invitation=new NetMeetingInvitation(true,c,msnMM);
		invitation->parseInvitation(bodyMSG);
	}
}

#include "netmeetingplugin.moc"
