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
#include "cryptographyselectuserkey.h"

#include <kiconloader.h>
#include <klocalizedstring.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qboxlayout.h>

#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
#include "kabcpersistence.h"

#include <kleo/ui/keyrequester.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>

#include "cryptographyplugin.h"

CryptographySelectUserKey::CryptographySelectUserKey ( const QString& key ,Kopete::MetaContact *mc )
		: KDialog()
{
	setCaption ( i18n ( "Select Contact's Public Key" ) );
	setButtons ( KDialog::Ok | KDialog::Cancel );
	setDefaultButton ( KDialog::Ok );

	m_metaContact=mc;

	QWidget *w = new QWidget ( this );
	QLabel * label = new QLabel ( w );
	m_KeyEdit = new Kleo::EncryptionKeyRequester ( false/*multipleKeys*/, Kleo::EncryptionKeyRequester::OpenPGP, w, false/*onlyTrusted*/, true/*onlyValid*/ );
	m_KeyEdit->setDialogMessage ( i18nc ( "@label:chooser", "Select the key you want to use encrypt messages to the recipient" ) );
	m_KeyEdit->setDialogCaption ( i18n ( "Select public key" ) );
	setMainWidget ( w );

	label->setText ( i18nc ( "@label:chooser", "Select public key for %1", mc->displayName() ) );
	m_KeyEdit->setFingerprint ( key );

	QVBoxLayout * l = new QVBoxLayout ( w );
	l->addWidget ( label );
	l->addWidget ( m_KeyEdit );
	l->addStretch ();

	if ( key.isEmpty() )
	{
		// find keys in address book and possibly use them (this same code is in cryptographyguiclient.cpp)
		QStringList keys;
		keys = CryptographyPlugin::getKabcKeys ( mc->metaContactId() );
		m_KeyEdit->setFingerprint ( CryptographyPlugin::kabcKeySelector (
				mc->displayName(),Kopete::KABCPersistence::self()->addressBook()->findByUid (
				mc->metaContactId()).assembledName(), keys, this ) );
	}
}

CryptographySelectUserKey::~CryptographySelectUserKey()
{
}

QString CryptographySelectUserKey::publicKey() const
{
	return m_KeyEdit->fingerprint();
}



#include "cryptographyselectuserkey.moc"

