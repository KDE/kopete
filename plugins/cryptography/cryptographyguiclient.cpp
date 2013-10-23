/*
    cryptographyguiclient.cpp

    Copyright (c) 2004      by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

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
#include "cryptographysettings.h"

#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsession.h"
#include "ui/kopeteview.h"
#include "kopeteuiglobal.h"
#include "kopeteprotocol.h"
#include "kabcpersistence.h"

#include <kabc/addressee.h>
#include <kabc/addressbook.h>
#include "exportkeys.h"

#include <kaction.h>
#include <klocalizedstring.h>
#include <kaboutdata.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <QList>
#include <kactioncollection.h>

class CryptographyPlugin;

CryptographyGUIClient::CryptographyGUIClient ( Kopete::ChatSession *parent )
		: QObject ( parent ) , KXMLGUIClient ( parent )
{
	if ( !parent || parent->members().isEmpty() )
	{
		deleteLater(); //we refuse to build this client, it is based on wrong parametters
		return;
	}

	KAboutData aboutData ( "kopete_cryptography", 0, ki18n ( "Cryptography" ) , "1.3.0" );
	setComponentData ( KComponentData ( &aboutData ) );

	setXMLFile ( "cryptographychatui.rc" );

	QList<Kopete::Contact*> mb=parent->members();
	bool wantEncrypt = false, wantSign = false, keysAvailable = false;

	// if any contacts have previously been encrypted/signed to, enable that now
	foreach ( Kopete::Contact *c , parent->members() )
	{
		Kopete::MetaContact *mc = c->metaContact();
		if ( !mc )
		{
			deleteLater();
			return;
		}
		if ( c->pluginData ( CryptographyPlugin::plugin(), "encrypt_messages" ) == "on" )
			wantEncrypt = true;
		if ( c->pluginData ( CryptographyPlugin::plugin(), "sign_messages" ) == "on" )
			wantSign = true;
		if ( ! ( mc->pluginData ( CryptographyPlugin::plugin(), "gpgKey" ).isEmpty() ) )
			keysAvailable = true;
	}

	m_encAction = new KToggleAction ( KIcon ( "document-encrypt" ), i18nc ( "@action toggle action", "Encrypt Messages" ), this );
	actionCollection()->addAction ( "encryptionToggle", m_encAction );
	m_signAction = new KToggleAction ( KIcon ( "document-sign" ), i18nc ( "@action toggle action", "Sign Messages" ), this );
	actionCollection()->addAction ( "signToggle", m_signAction );
	m_exportAction = new KAction ( i18nc ( "@action toggle action", "Export Contacts' Keys to Address Book" ), this );
	actionCollection()->addAction ( "export", m_exportAction );

	m_encAction->setChecked ( wantEncrypt && keysAvailable );
	m_signAction->setChecked ( wantSign );

	slotEncryptToggled();
	slotSignToggled();

	connect ( m_encAction, SIGNAL (triggered(bool)), this, SLOT (slotEncryptToggled()) );
	connect ( m_signAction, SIGNAL (triggered(bool)), this, SLOT (slotSignToggled()) );
	connect ( m_exportAction, SIGNAL (triggered(bool)), this, SLOT (slotExport()) );
}


CryptographyGUIClient::~CryptographyGUIClient()
{}

// set the pluginData to reflect new setting
void CryptographyGUIClient::slotSignToggled()
{
	if ( m_signAction->isChecked() ) {
		if ( CryptographySettings::privateKeyFingerprint().isEmpty() ) {
			KMessageBox::sorry ( Kopete::UI::Global::mainWidget(),
			                     i18nc ( "@info", "You have not selected a private key for yourself, so signing is not possible. Please select a private key in the Cryptography preferences dialog." ),
			                     i18n ( "No Private Key" ) );
			m_signAction->setChecked ( false );
		}
	}
	static_cast<Kopete::ChatSession *> ( parent() )->members().first()->setPluginData
	( CryptographyPlugin::plugin(), "sign_messages", m_signAction->isChecked() ? "on" : "off" );

}

void CryptographyGUIClient::slotEncryptToggled()
{
	Kopete::ChatSession *csn = static_cast<Kopete::ChatSession *> ( parent() );
		
	if ( m_encAction->isChecked() )
	{
		QStringList keyless;

		QWidget *w = 0;
		if ( csn->view() )
			w = csn->view()->mainWidget();

		foreach ( Kopete::Contact * c , csn->members() )
		{
			Kopete::MetaContact *mc = c->metaContact();
			if ( !mc )
				continue;

			// if encrypting and we don't have a key, look in address book
			if ( mc->pluginData ( CryptographyPlugin::plugin(), "gpgKey" ).isEmpty() )
			{
				// to grab the public key from KABC (this same code is in crytographyselectuserkey.cpp)
				KABC::Addressee addressee = Kopete::KABCPersistence::self()->addressBook()->findByUid
				                            ( mc->metaContactId() );

				if ( ! addressee.isEmpty() )
				{
					QStringList keys;
					keys = CryptographyPlugin::getKabcKeys ( mc->metaContactId() );

					// ask user if they want to use key found in address book
					KABC::Addressee tempAddressee = Kopete::KABCPersistence::self()->
					                                addressBook()->findByUid ( mc->metaContactId() );
					mc->setPluginData ( CryptographyPlugin::plugin(), "gpgKey",
					                    CryptographyPlugin::kabcKeySelector ( mc->displayName(), tempAddressee.assembledName(), keys, w ) );
				}
			}
			if ( mc->pluginData ( CryptographyPlugin::plugin(), "gpgKey" ).isEmpty() )
				keyless.append ( mc->displayName() );
		}

		// if encrypting and using unsupported protocols, warn user

		QString protocol ( csn->protocol()->metaObject()->className() );
		if ( ! CryptographyPlugin::supportedProtocols().contains ( protocol ) ) {
			KMessageBox::information ( w, i18nc ( "@info", "This protocol may not work with messages that are encrypted. This is because encrypted messages are very long, and the server or peer may reject them due to their length. To avoid being signed off or your account being warned or temporarily suspended, turn off encryption." ),
					i18n ( "Cryptography Unsupported Protocol" ),
			                           "Warn about unsupported " + QString ( csn->protocol()->metaObject()->className() ) );
		}

		// we can't encrypt if we don't have every single key we need
		if ( !keyless.isEmpty() )
		{
			KMessageBox::sorry ( w, i18ncp ( "@info",  "You need to select a public key for %2 to send encrypted messages to that contact.", "You need to select a public key for the following meta-contacts to send encrypted messages to them:\n%2",
			                                keyless.count(), keyless.join ( "\n" ) ),
			                     i18np ( "Missing public key", "Missing public keys", keyless.count() ) );

			m_encAction->setChecked ( false );
		}
	}
	
	// finally, set the pluginData to reflect new settings
	if ( csn->members().first() )
		csn->members().first()->setPluginData ( CryptographyPlugin::plugin() , "encrypt_messages" ,
		                       m_encAction->isChecked() ? "on" : "off" );
}

// put up dialog to allow user to choose which keys to export to address book
void CryptographyGUIClient::slotExport()
{
	Kopete::ChatSession *csn = qobject_cast<Kopete::ChatSession *> ( parent() );
	QList <Kopete::MetaContact*> mcs;
	foreach ( Kopete::Contact* c, csn->members() )
	mcs.append ( c->metaContact() );
	QPointer <ExportKeys> dialog = new ExportKeys ( mcs, csn->view()->mainWidget() );
	dialog->exec();
	delete dialog;
}

#include "cryptographyguiclient.moc"

