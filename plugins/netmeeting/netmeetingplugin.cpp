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

#include "msnmessagemanager.h"
#include "msnprotocol.h"
#include "msncontact.h"

#include "netmeetinginvitation.h"


K_EXPORT_COMPONENT_FACTORY( kopete_netmeeting, KGenericFactory<NetMeetingPlugin> );

NetMeetingPlugin::NetMeetingPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
	: KopetePlugin( parent, name )
{
	m_actions=0L;
	if(MSNProtocol::protocol())
		slotPluginLoaded(MSNProtocol::protocol());
	else
		connect(LibraryLoader::pluginLoader() , SIGNAL(pluginLoaded(KopetePlugin*) ), this, SLOT(slotPluginLoaded(KopetePlugin*)));
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


KActionCollection *NetMeetingPlugin::customChatActions(KopeteMessageManager *kmm)
{
	delete m_actions;

	m_currentKopeteMessageManager=dynamic_cast<MSNMessageManager*>(kmm);

	if(!m_currentKopeteMessageManager)
		return 0L;

	m_actions = new KActionCollection( this );
	m_actions->insert(  new KAction( i18n( "Invite to use NetMeeting" ), 0, this, SLOT( slotStartInvitation() ), this ) );

	return m_actions;
}

void NetMeetingPlugin::slotStartInvitation()
{
	QPtrList<KopeteContact> c=m_currentKopeteMessageManager->members();
	NetMeetingInvitation *i=new NetMeetingInvitation(false, static_cast<MSNContact*>(c.first()),m_currentKopeteMessageManager);
	m_currentKopeteMessageManager->initInvitation(i);
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
