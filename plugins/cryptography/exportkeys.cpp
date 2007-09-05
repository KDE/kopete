/*
    exportkeys.cpp

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
#include "exportkeys.h"

#include <kabc/addressee.h>
#include <kabc/addressbook.h>
#include <kabc/resource.h>

#include <kiconloader.h>

#include "kopetemetacontact.h"
#include "kopetechatsession.h"
#include "kopetecontact.h"
#include "kabcpersistence.h"
#include "kopeteprotocol.h"

#include "cryptographyplugin.h"

#include "ui_exportkeysbase.h"

ExportKeys::ExportKeys ( Kopete::ChatSession * cs, QWidget *parent )
		: KDialog ( parent )
{
	QWidget * w = new QWidget(this);
	mUi = new Ui::ExportKeysUI;
	mUi->setupUi (w);
	setMainWidget (w);
	
	setCaption ( i18n ("Export Public Keys") );
	setButtons ( KDialog::User1 | KDialog::Cancel );
	setButtonGuiItem ( KDialog::User1, KGuiItem ( i18n("Export"), QString(), i18n("Export checked keys to address book")));
	connect ( this, SIGNAL( user1Clicked() ), this, SLOT ( accept() ) );
	
	QString key;
	KABC::Addressee addressee;
	Kopete::MetaContact * mc;
	foreach ( Kopete::Contact *c, cs->members() )
	{
		mc = c->metaContact();
		key = mc->pluginData( CryptographyPlugin::plugin(), "gpgKey" );
		if (key.isEmpty())
			continue;
		addressee = Kopete::KABCPersistence::addressBook()->findByUid (mc->kabcId());
		if (addressee.isEmpty())
			addressee.setName ( mc->displayName() );
		addressee.insertCustom ("KADDRESSBOOK", "OPENPGPFP", key);
		
		key = key.right(8).prepend("0x");
		key = key + ' ' + mc->displayName() + " (" + addressee.assembledName() + ')';
		QListWidgetItem * tmpItem = new QListWidgetItem ( KIconLoader::global()->loadIconSet ("kgpg-export-kgpg", K3Icon::Small), key, mUi->keyList);
		tmpItem->setFlags (Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		tmpItem->setCheckState (Qt::Checked);
		mUi->keyList->addItem ( tmpItem );
		mAddressees.append (addressee);
	}
	if ( mUi->keyList->count() == 0 )
		mUi->keyList->addItem ( i18n ("<No meta-contacts with keys to export>") );
}


ExportKeys::~ExportKeys()
{
	delete mUi;
}

void ExportKeys::accept()
{
	kDebug (14303) << "running" << endl;
	
	KABC::AddressBook * ab = Kopete::KABCPersistence::self()->addressBook();
	
	for (int i = 0; i < mUi->keyList->count(); i++)
	{
		if (mUi->keyList->item(i)->checkState()){
			ab->insertAddressee (mAddressees.at(i));
			Kopete::KABCPersistence::self()->writeAddressBook(mAddressees.at(i).resource());
		}
	}
		
	QDialog::accept();
}

