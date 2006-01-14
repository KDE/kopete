/***************************************************************************
                          cryptographyselectuserkey.cpp  -  description
                             -------------------
    begin                : dim nov 17 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart @ kde.org
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
#include <qpushbutton.h>
#include <qlabel.h>

#include "cryptographyuserkey_ui.h"
#include "kopetemetacontact.h"
#include "popuppublic.h"

#include "cryptographyselectuserkey.h"

CryptographySelectUserKey::CryptographySelectUserKey(const QString& key ,Kopete::MetaContact *mc) : KDialogBase( 0l, "CryptographySelectUserKey", /*modal = */true, i18n("Select Contact's Public Key") , KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok )
{
	m_metaContact=mc;
	view = new CryptographyUserKey_ui(this,"CryptographyUserKey_ui");
	setMainWidget(view);

	connect (view->m_selectKey , SIGNAL(clicked()) , this , SLOT(slotSelectPressed()));
	connect (view->m_removeButton , SIGNAL(clicked()) , this , SLOT(slotRemovePressed()));

	view->m_titleLabel->setText(i18n("Select public key for %1").arg(mc->displayName()));
	view->m_editKey->setText(key);
}
CryptographySelectUserKey::~CryptographySelectUserKey()
{
}

void CryptographySelectUserKey::slotSelectPressed()
{
	popupPublic *dialog=new popupPublic(this, "public_keys", 0,false);
	connect(dialog,SIGNAL(selectedKey(QString &,QString,bool,bool)),this,SLOT(keySelected(QString &)));
	dialog->show();
}


void CryptographySelectUserKey::keySelected(QString &key)
{
	view->m_editKey->setText(key);
}

void CryptographySelectUserKey::slotRemovePressed()
{
	view->m_editKey->setText("");
}

QString CryptographySelectUserKey::publicKey() const
{
	return view->m_editKey->text();
}



#include "cryptographyselectuserkey.moc"

