/***************************************************************************
                          cryptographyselectuserkey.cpp  -  description
                             -------------------
    begin                : dim nov 17 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <klineedit.h>
 
#include "cryptographyuserkey_ui.h"
#include "kopetemetacontact.h"
#include "popuppublic.h"
 
#include "cryptographyselectuserkey.h"

CryptographySelectUserKey::CryptographySelectUserKey(const QString& key ,KopeteMetaContact *mc) : KDialogBase( 0l, "CryptographySelectUserKey", /*modal = */true, i18n("Select contact's public key") , KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok )  
{
	m_metaContact=mc;
	view = new CryptographyUserKey_ui(this,"CryptographyUserKey_ui");
	setMainWidget(view);

	connect (view->m_selectKey , SIGNAL(pressed()) , this , SLOT(slotSelectPressed()));
	connect (view->m_removeButton , SIGNAL(pressed()) , this , SLOT(slotRemovePressed()));

	view->m_titleLabel->setText(i18n("Select public key for %1").arg(mc->displayName()));
	view->m_editKey->setText(key);
}
CryptographySelectUserKey::~CryptographySelectUserKey()
{
}

void CryptographySelectUserKey::slotSelectPressed()
{
	popupPublic *dialogue=new popupPublic(this, "public_keys", 0,false);
	connect(dialogue,SIGNAL(selectedKey(QString &,bool,bool,bool,bool)),this,SLOT(keySelected(QString &,bool,bool,bool,bool)));
	dialogue->exec();
	delete dialogue;
}


void CryptographySelectUserKey::keySelected(QString &key,bool,bool,bool,bool)
{
	view->m_editKey->setText(key);
}

void CryptographySelectUserKey::slotRemovePressed()
{
	view->m_editKey->setText("");
}

QString CryptographySelectUserKey::publicKey()
{
	return view->m_editKey->text();
}



#include "cryptographyselectuserkey.moc"

