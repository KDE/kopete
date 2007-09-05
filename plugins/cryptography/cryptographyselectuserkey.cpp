/*
    cryptographyselectuserkey.cpp  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include <kiconloader.h>
#include <klocale.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qboxlayout.h>

#include "kopeteuiglobal.h"

#include <kleo/ui/keyrequester.h>

#include "kabcpersistence.h"
#include <kabc/addressbook.h>
#include <kabc/addressee.h>

#include "ui_kabckeyselectorbase.h"
#include "kopetemetacontact.h"
#include "cryptographyplugin.h"

#include "cryptographyselectuserkey.h"

CryptographySelectUserKey::CryptographySelectUserKey ( const QString& key ,Kopete::MetaContact *mc )
		: KDialog()
{
	setCaption ( i18n ( "Select Contact's Public Key" ) );
	setButtons ( KDialog::Ok | KDialog::Cancel );
	setDefaultButton ( KDialog::Ok );

	m_metaContact=mc;

	QWidget *w = new QWidget ( this );
	QLabel * label = new QLabel ( w );
	m_KeyEdit = new Kleo::EncryptionKeyRequester ( false, Kleo::EncryptionKeyRequester::OpenPGP, w, false, true );
	m_KeyEdit->setDialogMessage ( i18n ( "Select the key you want to use encrypt messages to the recipient" ) );
	m_KeyEdit->setDialogCaption ( i18n ( "Select the key you want to use encrypt messages to the recipient" ) );
	setMainWidget ( w );

	label->setText ( i18n ( "Select public key for %1", mc->displayName() ) );
	m_KeyEdit->setFingerprint ( key );

	QVBoxLayout * l = new QVBoxLayout ( w );
	l->addWidget ( label );
	l->addWidget ( m_KeyEdit );
	l->addStretch ();

	if ( key.isEmpty() )
	{
		// find keys in address book and possibly use them (this same code is in cryptographyguiclient.cpp)
		QStringList keys;
		keys = CryptographyPlugin::getKabcKeys ( m_metaContact->kabcId() );

		// if there is one key found, use it automatically
		if ( keys.count() == 1 )
		{
			mc->setPluginData ( CryptographyPlugin::plugin(), "gpgKey", keys.first() );
			m_KeyEdit->setFingerprint ( keys.first() );
		}

		// if more than one if found, let the user choose which one
		if ( keys.count() > 1 )
		{
			KABC::Addressee addressee = Kopete::KABCPersistence::self()->addressBook()->findByUid ( mc->kabcId() );
			KDialog dialog ( this );
			QWidget w ( &dialog );
			Ui::KabcKeySelectorUI ui;
			ui.setupUi ( &w );
			dialog.setCaption ( i18n ( "Public Keys Found" ) );
			dialog.setButtons ( KDialog::Ok | KDialog::Cancel );
			dialog.setMainWidget ( &w );
			ui.label->setText ( i18n ( QString ( "Cryptography plugin has found multiple encryption keys for " + mc->displayName() + " (" + addressee.assembledName() + ')' + " in your KDE address book. To use one of these keys, select it and choose OK." ).toLocal8Bit() ) );
			for ( int i = 0; i < keys.count(); i++ ) 
				ui.keyList->addItem ( new QListWidgetItem ( KIconLoader::global()->loadIconSet ("kgpg-key1-kopete", K3Icon::Small), keys[i].right(8).prepend("0x"), ui.keyList) );
			if ( dialog.exec() )
			{
				mc->setPluginData ( CryptographyPlugin::plugin(), "gpgKey", ui.keyList->currentItem()->text() );
				m_KeyEdit->setFingerprint ( ui.keyList->currentItem()->text() );
			}
		}
	}
}

CryptographySelectUserKey::~CryptographySelectUserKey()
{}

QString CryptographySelectUserKey::publicKey() const
{
	return m_KeyEdit->fingerprint();
}



#include "cryptographyselectuserkey.moc"

