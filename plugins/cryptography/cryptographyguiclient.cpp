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
#include "kopeteuiglobal.h"
#include "kopeteprotocol.h"

#include <kaction.h>
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

	bool wantEncrypt=true, wantSign=true, keysAvailable=true;

	foreach( Kopete::Contact *c , parent->members() )
	{
		Kopete::MetaContact *mc=c->metaContact();
		if(!mc)
		{
			deleteLater();
			return;
		}
		if( mc->pluginData( CryptographyPlugin::plugin(), "encrypt_messages"  ) == "off" )
			wantEncrypt = false;
		if ( mc->pluginData( CryptographyPlugin::plugin(), "sign_messges" ) == "off" )
			wantSign = false;
		if( mc->pluginData( CryptographyPlugin::plugin(), "gpgKey" ).isEmpty() )
			keysAvailable = false;
	}


	setComponentData( KGenericFactory<CryptographyPlugin>::componentData() );

	m_encAction = new KToggleAction( KIcon("encrypted"), i18n("Encrypt Messages" ), this );
        actionCollection()->addAction( "encryptionToggle", m_encAction );
	m_signAction = new KToggleAction ( KIcon ("signature"), i18n("Sign Messages" ), this );
	actionCollection()->addAction ("signToggle", m_signAction );
	
	m_encAction->setChecked(wantEncrypt && keysAvailable);
	m_signAction->setChecked(wantSign);
	
	connect( m_encAction, SIGNAL(triggered(bool)), this, SLOT(slotEncryptToggled()) );
	connect( m_signAction, SIGNAL(trigged(bool)), this, SLOT(slotSignToggled()) );

	setXMLFile("cryptographychatui.rc");
}


CryptographyGUIClient::~CryptographyGUIClient()
{}

void CryptographyGUIClient::slotSignToggled()
{
	static_cast<Kopete::ChatSession *>(parent())->members().first()->setPluginData ( CryptographyPlugin::plugin(), "sign_messages", m_signAction->isChecked() ? "on" : "off" );
	
}

void CryptographyGUIClient::slotEncryptToggled()
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

	if ( m_encAction->isChecked() )
	{
		QString protocol ( csn->protocol()->metaObject()->className());
		if ( m_encAction->isChecked() )
			if ( ! CryptographyPlugin::supportedProtocols().contains (protocol) )
				if (KMessageBox::warningYesNo ( 0L, i18n ("This protocol may not work with messages that are encrypted. Do you still want to encrypt messages?"), i18n ("Cryptography Plugin"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "Dont warn about unsupported protocols") == KMessageBox::No )
					m_encAction->setChecked (false);
		
	}
	
	if( m_encAction->isChecked() && !keyless.isEmpty() )
	{
		QWidget *w = 0;
		if ( csn->view() )
			w = csn->view()->mainWidget();

		KMessageBox::sorry( w, i18np("To send encrypted messages to %2, you still need to select a public key for this contact.", "To send encrypted messages to them, you still need to select a public key for each of these contacts:\n%2", keyless.count() , keyless.join( "\n" ) ), i18np( "Missing public key", "Missing public keys", keyless.count() ) );

		m_encAction->setChecked( false );
	}
	

        if (first)
            first->setPluginData( CryptographyPlugin::plugin() , "encrypt_messages" ,
						  m_encAction->isChecked() ? "on" : "off" );
}


#include "cryptographyguiclient.moc"

