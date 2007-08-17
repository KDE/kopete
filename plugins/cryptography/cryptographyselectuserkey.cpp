/*
    cryptographyselectuserkey.cpp  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include <klocale.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qboxlayout.h>

#include "kopeteuiglobal.h"

#include <kleo/ui/keyrequester.h>

#include "kopetemetacontact.h"

#include "cryptographyselectuserkey.h"

CryptographySelectUserKey::CryptographySelectUserKey(const QString& key ,Kopete::MetaContact *mc) 
 : KDialog()
{
	setCaption( i18n("Select Contact's Public Key") );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );

	m_metaContact=mc;
	
	QWidget *w = new QWidget(this);
	QLabel * label = new QLabel (w);
	m_KeyEdit = new Kleo::EncryptionKeyRequester ( false, Kleo::EncryptionKeyRequester::OpenPGP, w, false, true );
	m_KeyEdit->setDialogMessage ( i18n ( "Select the key you want to use encrypt messages to the recipient" ) );
	m_KeyEdit->setDialogCaption ( i18n ( "Select the key you want to use encrypt messages to the recipient" ) );
	setMainWidget(w);

	label->setText ( i18n("Select public key for %1", mc->displayName()) );
	m_KeyEdit->setFingerprint (key);
	
	QVBoxLayout * l = new QVBoxLayout (w);
	l->addWidget (label);
	l->addWidget (m_KeyEdit);
	l->addStretch ();
}

CryptographySelectUserKey::~CryptographySelectUserKey()
{

}

QString CryptographySelectUserKey::publicKey() const
{
	return m_KeyEdit->fingerprint();
}



#include "cryptographyselectuserkey.moc"

