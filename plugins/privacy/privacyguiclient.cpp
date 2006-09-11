/*
    privacyguiclient.cpp

    Copyright (c) 2006 by Andre Duffeck        <andre@duffeck.de>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "privacyguiclient.h"
#include "privacyplugin.h"

#include "kopetechatsession.h"
#include "kopetecontact.h"
#include "kopetepluginmanager.h"

#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kicon.h>

#include <QList>

class PrivacyPlugin;

PrivacyGUIClient::PrivacyGUIClient(Kopete::ChatSession *parent)
 : QObject(parent), KXMLGUIClient(parent)
{
	setInstance(KGenericFactory<PrivacyPlugin>::instance());

	m_manager = parent;

	// Refuse to build this client, it is based on wrong parameters
	if(!m_manager || m_manager->members().isEmpty())
		deleteLater();

	QList<Kopete::Contact*> mb=m_manager->members();

	actionAddToWhiteList = new KAction( KIcon("privacy_whitelist"), i18n("Add to WhiteList" ),
		actionCollection(), "addToWhiteList" );
	connect( actionAddToWhiteList, SIGNAL(triggered(bool)), this, SLOT(slotAddToWhiteList()) );
	actionAddToBlackList = new KAction( KIcon("privacy_blacklist"), i18n("Add to BlackList" ),
		actionCollection(), "addToBlackList" );
	connect( actionAddToBlackList, SIGNAL(triggered(bool)), this, SLOT(slotAddToBlackList()) );
	actionAddToWhiteList->setEnabled(true);
	actionAddToBlackList->setEnabled(true);

	setXMLFile("privacychatui.rc");
}


PrivacyGUIClient::~PrivacyGUIClient()
{
}


void PrivacyGUIClient::slotAddToBlackList()
{
	kDebug(14313) << k_funcinfo << endl;
	Kopete::Plugin *plugin = Kopete::PluginManager::self()->plugin("kopete_privacy");
	if( !plugin )
		return;
	kDebug(14313) << k_funcinfo << "Plugin found" << endl;
	
	QList<Kopete::Contact*> members = m_manager->members();

	QList< Kopete::Contact *> list;
	foreach( Kopete::Contact *contact, members )
	{
		if( !(contact == m_manager->myself()) )
			list.append( contact );
	}

	static_cast<PrivacyPlugin *>(plugin)->addContactsToBlackList( list );
}

void PrivacyGUIClient::slotAddToWhiteList()
{
	kDebug(14313) << k_funcinfo << endl;
	Kopete::Plugin *plugin = Kopete::PluginManager::self()->plugin("kopete_privacy");
	if( !plugin )
		return;
	
	QList<Kopete::Contact*> members = m_manager->members();

	QList< Kopete::Contact *> list;
	foreach( Kopete::Contact *contact, members )
	{
		if( !(contact == m_manager->myself()) )
			list.append( contact );
	}

	static_cast<PrivacyPlugin *>(plugin)->addContactsToWhiteList( list );
}

#include "privacyguiclient.moc"
