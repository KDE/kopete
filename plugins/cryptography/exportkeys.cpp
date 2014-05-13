/*
    exportkeys.cpp

    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

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

#include <kiconloader.h>
#include <kpushbutton.h>

#include "kopetemetacontact.h"
#include "kabcpersistence.h"

#include "cryptographyplugin.h"

#include "ui_exportkeysbase.h"

ExportKeys::ExportKeys ( QList<Kopete::MetaContact*> mcs, QWidget *parent )
		: KDialog ( parent )
{
	QWidget * w = new QWidget(this);
	mUi = new Ui::ExportKeysUI;
	mUi->setupUi (w);
	setMainWidget (w);
	
	setCaption ( i18n ("Export Public Keys") );
	setButtons ( KDialog::User1 | KDialog::Cancel );
	setButtonGuiItem ( KDialog::User1, KGuiItem ( i18nc("@action:button", "Export"), "document-export-key", i18nc("@info:tooltip", "Export checked keys to address book")));
	connect ( this, SIGNAL(user1Clicked()), this, SLOT (accept()) );
	
	QString key;
	KABC::Addressee addressee;
	// this loop creates the list widget items
	foreach ( Kopete::MetaContact *mc, mcs )
	{
		// see if there is a key. if not, go to top of loop and start again with a new metacontact
		key = mc->pluginData( CryptographyPlugin::plugin(), "gpgKey" );
		if (key.isEmpty())
			continue;
		// get addressee for metacontact
		addressee = Kopete::KABCPersistence::addressBook()->findByUid (mc->metaContactId());
		// if no addressee exsists, create one by setting the name
		if (addressee.isEmpty())
			addressee.setFormattedName ( mc->displayName() );
		// add key to old or new addressee
		addressee.insertCustom ("KADDRESSBOOK", "OPENPGPFP", key);
		
		// now we create the ListWidgetItem
		key = key.right(8).prepend("0x");
		key = key + ' ' + mc->displayName() + " (" + addressee.formattedName() + ')';
		QListWidgetItem * tmpItem = new QListWidgetItem ( KIcon ("document-export-key"), key, mUi->keyList);
		tmpItem->setFlags (Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		tmpItem->setCheckState (Qt::Checked);
		mUi->keyList->addItem ( tmpItem );
		// add addressee to master lists
		// these are kept aligned so that mAddressees[i] refers to the same person as mMetaContacts[i]
		mAddressees.append (addressee);
		mMetaContacts.append (mc);
	}
	if ( mUi->keyList->count() == 0 ){
		mUi->keyList->addItem ( i18nc ("@item:inlistbox", "&lt;No metacontacts with keys to export&gt;") );
		button( KDialog::User1 )->setEnabled (false);
	}
}


ExportKeys::~ExportKeys()
{
	delete mUi;
}

void ExportKeys::accept()
{
	KABC::AddressBook * ab = Kopete::KABCPersistence::self()->addressBook();
	
	// add addressees to address book
	for (int i = 0; i < mUi->keyList->count(); i++)
	{
		if (mUi->keyList->item(i)->checkState() )
		{
			// if metacontact was not previously associated with this addressee, change the uid to associate it
			if ( mMetaContacts.at(i)->metaContactId() != mAddressees.at(i).uid() )
				mMetaContacts.at(i)->setMetaContactId (mAddressees.at(i).uid());
			kDebug (14303) << "new uid for kabc contact " << mAddressees.at(i).formattedName() << " is " << mMetaContacts.at(i)->metaContactId();
			ab->insertAddressee (mAddressees.at(i));
			Kopete::KABCPersistence::self()->write (mMetaContacts.at(i));
			Kopete::KABCPersistence::self()->writeAddressBook(mAddressees.at(i).resource());
		}
	}

	KDialog::accept();
}


