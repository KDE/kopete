/*
    cryptographyguiclient.cpp

    Copyright (c) 2004 by Olivier Goffart        <ogoffart @ kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "cryptographyguiclient.h"
#include "cryptographyplugin.h"


#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetemessagemanager.h"

#include <kaction.h>
#include <kconfig.h>
#include <klocale.h>
#include <kgenericfactory.h>

class CryptographyPlugin;

CryptographyGUIClient::CryptographyGUIClient(Kopete::ChatSession *parent )
 : QObject(parent) , KXMLGUIClient(parent)
{
	if(!parent || parent->members().isEmpty())
	{
		deleteLater(); //we refuse to build this client, it is based on wrong parametters
		return;
	}

	QPtrList<Kopete::Contact> mb=parent->members();
	m_first=mb.first()->metaContact();

	if(!m_first)
	{
		deleteLater(); //we refuse to build this client, it is based on wrong parametters
		return;
	}

	setInstance( KGenericFactory<CryptographyPlugin>::instance() );


	m_action=new KToggleAction( i18n("Encrypt Messages" ), QString::fromLatin1( "encrypted" ), 0, this, SLOT(slotToggled()), actionCollection() , "cryptographyToggle" );
	m_action->setChecked( m_first->pluginData( CryptographyPlugin::plugin() , "encrypt_messages") != QString::fromLatin1("off") ) ;

	setXMLFile("cryptographychatui.rc");
}


CryptographyGUIClient::~CryptographyGUIClient()
{}

void CryptographyGUIClient::slotToggled()
{
	m_first->setPluginData(CryptographyPlugin::plugin() , "encrypt_messages" ,
		m_action->isChecked() ? "on" : "off" );
}


#include "cryptographyguiclient.moc"

