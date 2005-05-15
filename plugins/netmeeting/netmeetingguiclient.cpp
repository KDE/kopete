/*
    netmeetingguiclient.cpp

    Kopete NetMeeting plugin

    Copyright (c) 2003-2004 by Olivier Goffart <ogoffart @ kde.org>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qvariant.h>

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "msnchatsession.h"
#include "msncontact.h"

#include "netmeetingguiclient.h"
#include "netmeetinginvitation.h"

class NetMeetingPlugin;

NetMeetingGUIClient::NetMeetingGUIClient( MSNChatSession *parent,  const char *name )
: QObject( parent, name ) , KXMLGUIClient(parent)
{
	setInstance(KGenericFactory<NetMeetingPlugin>::instance());
	m_manager=parent;

	new KAction( i18n( "Invite to Use NetMeeting" ), 0, this, SLOT( slotStartInvitation() ), actionCollection() , "netmeeting" ) ;

	setXMLFile("netmeetingchatui.rc");
}

NetMeetingGUIClient::~NetMeetingGUIClient()
{

}

void NetMeetingGUIClient::slotStartInvitation()
{
	QPtrList<Kopete::Contact> c=m_manager->members();
	NetMeetingInvitation *i=new NetMeetingInvitation(false, static_cast<MSNContact*>(c.first()),m_manager);
	m_manager->initInvitation(i);
}

#include "netmeetingguiclient.moc"

// vim: set noet ts=4 sts=4 sw=4:

