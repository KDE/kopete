/*
    cryptographyguiclient.cpp

    Copyright (c) 2004      by Olivier Goffart        <ogoffart@kde.org>

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
#include "cryptographyguiclient.h"
#include "cryptographyplugin.h"


#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetechatsession.h"
#include "kopeteview.h"

#include <kaction.h>
#include <ktoggleaction.h>
#include <kconfig.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <kmessagebox.h>




#include <QList>
#include <kactioncollection.h>

class CryptographyPlugin;

CryptographyGUIClient::CryptographyGUIClient(Kopete::ChatSession *parent )
 : QObject(parent) , KXMLGUIClient(parent)
{
	if(!parent || parent->members().isEmpty())
	{
		deleteLater(); //we refuse to build this client, it is based on wrong parametters
		return;
	}

	QList<Kopete::Contact*> mb=parent->members();
	Kopete::MetaContact *first=mb.first()->metaContact();

	bool wantCrypto=true, keysAvailable=true;

	foreach( Kopete::Contact *c , parent->members() )
	{
		Kopete::MetaContact *mc=c->metaContact();
		if(!mc)
		{
			deleteLater();
			return;
		}
		if( mc->pluginData( CryptographyPlugin::plugin(), "encrypt_messages"  ) == "off" )
			wantCrypto = false;
		if( mc->pluginData( CryptographyPlugin::plugin(), "gpgKey" ).isEmpty() )
			keysAvailable = false;
	}


	setComponentData( KGenericFactory<CryptographyPlugin>::componentData() );

	m_action=new KToggleAction( KIcon("encrypted"), i18n("Encrypt Messages" ), this );
        actionCollection()->addAction( "cryptographyToggle", m_action );
	m_action->setChecked(wantCrypto && keysAvailable);
	connect( m_action, SIGNAL(triggered(bool)), this, SLOT(slotToggled()) );

	setXMLFile("cryptographychatui.rc");
}


CryptographyGUIClient::~CryptographyGUIClient()
{}

void CryptographyGUIClient::slotToggled()
{
	Kopete::ChatSession *csn = static_cast<Kopete::ChatSession *>( parent() );
	QStringList keyless;

	Kopete::MetaContact *first=0L;
	foreach( Kopete::Contact * c , csn->members() )
	{
		Kopete::MetaContact *mc = c->metaContact();
		if(!mc)
			continue;

		if(!first)
			first=mc;

		if( mc->pluginData( CryptographyPlugin::plugin(), "gpgKey" ).isEmpty() )
			keyless.append(mc->displayName());
	}

	if( m_action->isChecked() && !keyless.isEmpty() )
	{
		QWidget *w = 0;
		if ( csn->view() )
			w = csn->view()->mainWidget();

		KMessageBox::sorry( w,
							i18np("To send encrypted messages to %1, you still need to select a public key for this contact.", "To send encrypted messages to them, you still need to select a public key for each of these contacts:\n%1", keyless.count() , keyless.join( "\n" ) ),
							i18np( "Missing public key", "Missing public keys", keyless.count() ) );

		m_action->setChecked( false );
	}

	first->setPluginData( CryptographyPlugin::plugin() , "encrypt_messages" ,
						  m_action->isChecked() ? "on" : "off" );
}


#include "cryptographyguiclient.moc"

