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

#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetechatsession.h"
#include "kopeteview.h"
#include "kopeteuiglobal.h"
#include "kopeteprotocol.h"

#include "kabcpersistence.h"
#include <kabc/addressee.h>
#include <kabc/addressbook.h>
#include "ui_kabckeyselectorbase.h"
#include "exportkeys.h"

#include <kaction.h>
#include <klocale.h>
#include <kgenericfactory.h>
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

	QList<Kopete::Contact*> mb=parent->members();

	bool wantEncrypt=true, wantSign=true, keysAvailable=true;

	foreach ( Kopete::Contact *c , parent->members() )
	{
		Kopete::MetaContact *mc=c->metaContact();
		if ( !mc )
		{
			deleteLater();
			return;
		}
		if ( mc->pluginData ( CryptographyPlugin::plugin(), "encrypt_messages" ) == "off" )
			wantEncrypt = false;
		if ( mc->pluginData ( CryptographyPlugin::plugin(), "sign_messges" ) == "off" )
			wantSign = false;
		if ( mc->pluginData ( CryptographyPlugin::plugin(), "gpgKey" ).isEmpty() )
			keysAvailable = false;
	}


	setComponentData ( KGenericFactory<CryptographyPlugin>::componentData() );

	m_encAction = new KToggleAction ( KIcon ( "encrypted" ), i18n ( "Encrypt Messages" ), this );
	actionCollection()->addAction ( "encryptionToggle", m_encAction );
	m_signAction = new KToggleAction ( KIcon ( "signature" ), i18n ( "Sign Messages" ), this );
	actionCollection()->addAction ( "signToggle", m_signAction );
	m_exportAction = new KAction ( i18n ("Export Contact's Key to Address Book" ), this );
	actionCollection()->addAction ( "export", m_exportAction );

	m_encAction->setChecked ( wantEncrypt && keysAvailable );
	m_signAction->setChecked ( wantSign );

	connect ( m_encAction, SIGNAL ( triggered ( bool ) ), this, SLOT ( slotEncryptToggled() ) );
	connect ( m_signAction, SIGNAL ( triggered ( bool ) ), this, SLOT ( slotSignToggled() ) );
	connect ( m_exportAction, SIGNAL ( triggered ( bool ) ), this, SLOT ( slotExport() ) );

	setXMLFile ( "cryptographychatui.rc" );
}


CryptographyGUIClient::~CryptographyGUIClient()
{}

void CryptographyGUIClient::slotSignToggled()
{
	static_cast<Kopete::ChatSession *> ( parent() )->members().first()->setPluginData ( CryptographyPlugin::plugin(), "sign_messages", m_signAction->isChecked() ? "on" : "off" );

}

void CryptographyGUIClient::slotEncryptToggled()
{
	Kopete::ChatSession *csn = static_cast<Kopete::ChatSession *> ( parent() );
	QStringList keyless;

	Kopete::MetaContact *first=0L;
	foreach ( Kopete::Contact * c , csn->members() )
	{
		Kopete::MetaContact *mc = c->metaContact();
		if ( !mc )
			continue;

		if ( !first )
			first=mc;

		if ( mc->pluginData ( CryptographyPlugin::plugin(), "gpgKey" ).isEmpty() && m_encAction->isChecked() )
		{
			// to grab the public key from KABC (this same code is in crytographyselectuserkey.cpp)
			KABC::Addressee addressee = Kopete::KABCPersistence::self()->addressBook()->findByUid ( mc->kabcId() );

			if ( ! addressee.isEmpty() )
			{
				QStringList keys;
				keys = CryptographyPlugin::getKabcKeys( mc->kabcId() );
				
				// ask user if they want to use key found in address book
				if ( keys.count() == 1 ) {
					if ( KMessageBox::questionYesNo ( csn->view()->mainWidget(), i18n ( QString ("Cryptography plugin has found an encryption key for " + mc->displayName() + " (" + addressee.assembledName() + ')' + " in your KDE address book. Do you want to use key " + keys.first().right ( 8 ).prepend ( "0x" ) + " as this contact's public key?").toLocal8Bit() ), i18n ( "Public Key Found" ) ) == KMessageBox::Yes ) {
						mc->setPluginData ( CryptographyPlugin::plugin(), "gpgKey", keys.first() );
					}
				}
				// ask user if they want to use key found in address book and if so, which one
				if ( keys.count() > 1 )
				{
					KDialog dialog (csn->view()->mainWidget());
					QWidget w (&dialog);
					Ui::KabcKeySelectorUI ui;
					ui.setupUi (&w);
					dialog.setCaption ( i18n ("Public Keys Found") );
					dialog.setButtons ( KDialog::Ok | KDialog::Cancel );
					dialog.setMainWidget (&w);
					ui.label->setText ( i18n ( QString("Cryptography plugin has found multiple encryption keys for " + mc->displayName() + " (" + addressee.assembledName() + ')' + " in your KDE address book. To use one of these keys, select it and choose OK." ).toLocal8Bit() ) );
					for (int i = 0; i < keys.count(); i++)
						ui.keyList->addItem ( new QListWidgetItem ( KIconLoader::global()->loadIconSet ("kgpg-key1-kopete", K3Icon::Small), keys[i].right(8).prepend("0x"), ui.keyList) );
					ui.keyList->addItems (keys);
					if (dialog.exec())
						mc->setPluginData ( CryptographyPlugin::plugin(), "gpgKey", ui.keyList->currentItem()->text());
				}
			}
		}
		if ( mc->pluginData ( CryptographyPlugin::plugin(), "gpgKey" ).isEmpty() )
			keyless.append (mc->displayName());
	}

	if ( m_encAction->isChecked() )
	{
		QString protocol ( csn->protocol()->metaObject()->className() );
		if ( m_encAction->isChecked() )
			if ( ! CryptographyPlugin::supportedProtocols().contains ( protocol ) )
				KMessageBox::information ( csn->view()->mainWidget(), i18n ( "This protocol may not work with messages that are encrypted. This is because encrypted messages are very long, and the server or peer may reject them due to their length. To avoid being signed off or your account being warned or temporarily suspended, turn off encryption." ), i18n ( "Cryptography Unsupported Protocol" ), "Warn about unsupported " + QString ( csn->protocol()->metaObject()->className() ) );

	}

	if ( m_encAction->isChecked() && !keyless.isEmpty() )
	{
		QWidget *w = 0;
		if ( csn->view() )
			w = csn->view()->mainWidget();

		KMessageBox::sorry ( w, i18np ( "To send encrypted messages to %2, you still need to select a public key for this contact.", "To send encrypted messages to them, you still need to select a public key for each of these contacts:\n%2", keyless.count(), keyless.join ( "\n" ) ), i18np ( "Missing public key", "Missing public keys", keyless.count() ) );

		m_encAction->setChecked ( false );
	}

	if ( first )
		first->setPluginData ( CryptographyPlugin::plugin() , "encrypt_messages" ,
		                       m_encAction->isChecked() ? "on" : "off" );
}

void CryptographyGUIClient::slotExport()
{
	Kopete::ChatSession *csn = static_cast<Kopete::ChatSession *> ( parent() );
	ExportKeys dialog (csn, csn->view()->mainWidget() );
	dialog.exec();	
}


#include "cryptographyguiclient.moc"

