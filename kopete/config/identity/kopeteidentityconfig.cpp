/*
    kopeteidentityconfig.cpp  -  Kopete Identity config page

    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

    Kopete    (c) 2003-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteidentityconfig.h"

// Qt includes
#include <qlayout.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qbuffer.h>

// KDE includes
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kurlrequester.h>
#include <kinputdialog.h>
#include <kpixmapregionselectordialog.h>
#include <kmdcodec.h>

// KDE KIO includes
#include <kio/netaccess.h>

// KDE KABC(AddressBook) includes
#include <kabc/addresseedialog.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>

// Kopete include
#include "kabcpersistence.h"
#include "kopeteglobal.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "addressbookselectordialog.h"
#include "kopeteconfig.h"

// Local includes
#include "kopeteidentityconfigbase.h"
#include "globalidentitiesmanager.h"
#include "kopeteidentityconfigpreferences.h"

class KopeteIdentityConfig::Private
{
public:
	Private() : m_view(0L), myself(0L), currentIdentity(0L), selectedIdentity("")
	{}

	KopeteIdentityConfigBase *m_view;
	Kopete::MetaContact *myself;
	Kopete::MetaContact *currentIdentity;
	
	QMap<int, Kopete::Contact*> contactPhotoSourceList;
	QString selectedIdentity;
};

typedef KGenericFactory<KopeteIdentityConfig, QWidget> KopeteIdentityConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_identityconfig, KopeteIdentityConfigFactory( "kcm_kopete_identityconfig" ) )

KopeteIdentityConfig::KopeteIdentityConfig(QWidget *parent, const char */*name*/, const QStringList &args) : KCModule( KopeteIdentityConfigFactory::instance(), parent, args)
{
	d = new Private;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	d->m_view = new KopeteIdentityConfigBase( this, "KopeteIdentityConfig::m_view" );
	
	// Setup KConfigXT link with GUI.
	addConfig( Kopete::Config::self(), d->m_view );

	// Load config
	KopeteIdentityConfigPreferences::self()->readConfig();

	// Load from XML the identities.
	GlobalIdentitiesManager::self()->loadXML();

	d->myself = Kopete::ContactList::self()->myself();
	
	// Set the latest selected Identity.
	d->selectedIdentity = KopeteIdentityConfigPreferences::self()->selectedIdentity();
	kdDebug() << k_funcinfo << "Latest loaded identity: " << d->selectedIdentity << endl;

	// If the latest selected Identity is not present anymore, use a fallback identity.
	if( !GlobalIdentitiesManager::self()->isIdentityPresent(d->selectedIdentity) )
	{
		QMapIterator<QString, Kopete::MetaContact*> it = GlobalIdentitiesManager::self()->getGlobalIdentitiesList().begin();
		d->selectedIdentity = it.key();
	}
	else
	{
		// Update the latest identity with myself Metacontact.
		GlobalIdentitiesManager::self()->updateIdentity(d->selectedIdentity, d->myself);
	}
	d->currentIdentity = GlobalIdentitiesManager::self()->getIdentity(d->selectedIdentity);
	
	// Set icon for KPushButton
	d->m_view->buttonNewIdentity->setIconSet(SmallIconSet("new"));
	d->m_view->buttonCopyIdentity->setIconSet(SmallIconSet("editcopy"));
	d->m_view->buttonRenameIdentity->setIconSet(SmallIconSet("edit"));
	d->m_view->buttonRemoveIdentity->setIconSet(SmallIconSet("delete_user"));
	d->m_view->buttonClearPhoto->setIconSet(  SmallIconSet( QApplication::reverseLayout() ? "locationbar_erase" : "clear_left" ) );

	load(); // Load Configuration

	// Action signal/slots
	connect(d->m_view->buttonChangeAddressee, SIGNAL(clicked()), this, SLOT(slotChangeAddressee()));
	connect(d->m_view->comboSelectIdentity, SIGNAL(activated(const QString &)), this, SLOT(slotUpdateCurrentIdentity(const QString& )));
	connect(d->m_view->buttonNewIdentity, SIGNAL(clicked()), this, SLOT(slotNewIdentity()));
	connect(d->m_view->buttonCopyIdentity, SIGNAL(clicked()), this, SLOT(slotCopyIdentity()));
	connect(d->m_view->buttonRenameIdentity, SIGNAL(clicked()), this, SLOT(slotRenameIdentity()));
	connect(d->m_view->buttonRemoveIdentity, SIGNAL(clicked()), this, SLOT(slotRemoveIdentity()));
	connect(d->m_view->comboPhotoURL, SIGNAL(urlSelected(const QString& )), this, SLOT(slotChangePhoto(const QString& )));
	connect(d->m_view->buttonClearPhoto, SIGNAL(clicked()), this, SLOT(slotClearPhoto()));

	// Settings signal/slots
	connect(d->m_view->radioNicknameContact, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(d->m_view->radioNicknameCustom, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(d->m_view->radioNicknameKABC, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));

	connect(d->m_view->radioPhotoContact, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(d->m_view->radioPhotoCustom, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(d->m_view->radioPhotoKABC, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));

	connect(d->m_view->checkSyncPhotoKABC, SIGNAL(toggled(bool )), this, SLOT(slotSettingsChanged()));
	connect(d->m_view->lineNickname, SIGNAL(textChanged(const QString& )), this, SLOT(slotSettingsChanged()));
	connect(d->m_view->comboNameContact, SIGNAL(activated(int )), this, SLOT(slotSettingsChanged()));
	connect(d->m_view->comboPhotoContact, SIGNAL(activated(int )), this, SLOT(slotEnableAndDisableWidgets()));
}

KopeteIdentityConfig::~KopeteIdentityConfig()
{
	delete d;
}

void KopeteIdentityConfig::load()
{
	KCModule::load();

	// Populate the select Identity combo box.
	loadIdentities();
	// Populate the name contact ComboBox
	slotLoadNameSources();
	// Populate the photo contact ComboBOx
	slotLoadPhotoSources();

	KABC::Addressee a = KABC::StdAddressBook::self()->whoAmI();
	// Load the address book link
	if (!a.isEmpty())
	{
		d->m_view->lineAddressee->setText(a.realName());
	}

	slotEnableAndDisableWidgets();
}

void KopeteIdentityConfig::save()
{
	KCModule::save();

	saveCurrentIdentity();

	// Don't save the new global identity if it's not activated.
	if(d->m_view->kcfg_EnableGlobalIdentity->isChecked())
	{
		// Save the myself metacontact settings.
		// Nickname settings.
		if(d->m_view->lineNickname->text() != d->myself->customDisplayName())
			d->myself->setDisplayName(d->m_view->lineNickname->text());
		
		d->myself->setDisplayNameSource(selectedNameSource());
		d->myself->setDisplayNameSourceContact(selectedNameSourceContact());

		// Photo settings
		d->myself->setPhotoSource(selectedPhotoSource());
		d->myself->setPhotoSourceContact(selectedPhotoSourceContact());
		if(!d->m_view->comboPhotoURL->url().isEmpty())
			d->myself->setPhoto(d->m_view->comboPhotoURL->url());
		else
			d->myself->setPhoto( KURL() );
		d->myself->setPhotoSyncedWithKABC(d->m_view->checkSyncPhotoKABC->isChecked());
	}
	
	// Save global identities list.
	KopeteIdentityConfigPreferences::self()->setSelectedIdentity(d->selectedIdentity);
	GlobalIdentitiesManager::self()->saveXML();

	// (Re)made slot connections to apply Global Identity in protocols
	Kopete::ContactList::self()->loadGlobalIdentity();

	load();
}

void KopeteIdentityConfig::loadIdentities()
{
	d->m_view->comboSelectIdentity->clear();

	QMap<QString, Kopete::MetaContact*> identitiesList = GlobalIdentitiesManager::self()->getGlobalIdentitiesList();
	QMapIterator<QString, Kopete::MetaContact*> it;
	QMapIterator<QString, Kopete::MetaContact*> end = identitiesList.end();

	int count=0, selectedIndex=0;
	for(it = identitiesList.begin(); it != end; ++it)
	{
		d->m_view->comboSelectIdentity->insertItem(it.key());
		if(it.key() == d->selectedIdentity)
		{
			selectedIndex = count;
		}
		count++;
	}

	d->m_view->comboSelectIdentity->setCurrentItem(selectedIndex);
	d->m_view->buttonRemoveIdentity->setEnabled(count == 1 ? false : true);
}

void KopeteIdentityConfig::saveCurrentIdentity()
{
	kdDebug() << k_funcinfo << "Saving data of current identity." << endl;
	// Ignore saving when removing a identity
	if(!d->currentIdentity)
		return;

	if(d->m_view->lineNickname->text() != d->currentIdentity->customDisplayName())
		d->currentIdentity->setDisplayName(d->m_view->lineNickname->text());
		
	d->currentIdentity->setDisplayNameSource(selectedNameSource());
	d->currentIdentity->setDisplayNameSourceContact(selectedNameSourceContact());

	// Photo settings
	d->currentIdentity->setPhotoSource(selectedPhotoSource());
	d->currentIdentity->setPhotoSourceContact(selectedPhotoSourceContact());
	if(!d->m_view->comboPhotoURL->url().isEmpty())
		d->currentIdentity->setPhoto(d->m_view->comboPhotoURL->url());
	else
		d->currentIdentity->setPhoto( KURL() );
	d->currentIdentity->setPhotoSyncedWithKABC(d->m_view->checkSyncPhotoKABC->isChecked());
}

void KopeteIdentityConfig::slotLoadNameSources()
{
	Kopete::Contact *nameSourceContact = d->currentIdentity->displayNameSourceContact();

	QPtrList<Kopete::Contact> contactList = d->myself->contacts(); // Use myself contact PtrList. Safer.
	QPtrListIterator<Kopete::Contact> it(contactList);

	d->m_view->comboNameContact->clear();

	for(; it.current(); ++it)
	{
		QString account = it.current()->property(Kopete::Global::Properties::self()->nickName()).value().toString() + " <" + it.current()->contactId() + ">";
		QPixmap accountIcon = it.current()->account()->accountIcon();
		d->m_view->comboNameContact->insertItem(accountIcon,  account);

		// Select this item if it's the one we're tracking.
		if(it.current() == nameSourceContact)
		{
			d->m_view->comboNameContact->setCurrentItem(d->m_view->comboNameContact->count() - 1);
		}
	}

	d->m_view->lineNickname->setText(d->currentIdentity->customDisplayName());

	Kopete::MetaContact::PropertySource nameSource = d->currentIdentity->displayNameSource();

	d->m_view->radioNicknameCustom->setChecked(nameSource == Kopete::MetaContact::SourceCustom);
	d->m_view->radioNicknameKABC->setChecked(nameSource == Kopete::MetaContact::SourceKABC);
	d->m_view->radioNicknameContact->setChecked(nameSource == Kopete::MetaContact::SourceContact);
}

void KopeteIdentityConfig::slotLoadPhotoSources()
{
	Kopete::Contact *photoSourceContact = d->currentIdentity->photoSourceContact();

	QPtrList<Kopete::Contact> contactList = d->myself->contacts(); // Use myself contact PtrList. Safer.
	QPtrListIterator<Kopete::Contact> it(contactList);

	d->m_view->comboPhotoContact->clear();
	d->m_view->comboPhotoURL->clear();
	d->contactPhotoSourceList.clear();

	for(; it.current(); ++it)
	{
		Kopete::Contact *currentContact = it.current();
		if(currentContact->hasProperty(Kopete::Global::Properties::self()->photo().key()))
		{
			QString account = currentContact->property(Kopete::Global::Properties::self()->nickName()).value().toString() + " <" + currentContact->contactId() + ">";
			QPixmap accountIcon = currentContact->account()->accountIcon();

			d->m_view->comboPhotoContact->insertItem(accountIcon,  account);
			d->contactPhotoSourceList.insert(d->m_view->comboPhotoContact->count() - 1, currentContact);

			// Select this item if it's the one we're tracking.
			if(currentContact == photoSourceContact)
			{
				d->m_view->comboPhotoContact->setCurrentItem(d->m_view->comboPhotoContact->count() - 1);
			}
		}
	}

	d->m_view->comboPhotoURL->setURL(d->currentIdentity->customPhoto().pathOrURL());
	Kopete::MetaContact::PropertySource photoSource = d->currentIdentity->photoSource();

	d->m_view->radioPhotoCustom->setChecked(photoSource == Kopete::MetaContact::SourceCustom);
	d->m_view->radioPhotoContact->setChecked(photoSource == Kopete::MetaContact::SourceContact);
	d->m_view->radioPhotoKABC->setChecked(photoSource == Kopete::MetaContact::SourceKABC);

	d->m_view->checkSyncPhotoKABC->setChecked(d->currentIdentity->isPhotoSyncedWithKABC());
}

void KopeteIdentityConfig::slotEnableAndDisableWidgets()
{
	KABC::Addressee a = KABC::StdAddressBook::self()->whoAmI();
	bool hasKABCLink = !a.isEmpty();

	d->m_view->radioNicknameKABC->setEnabled(hasKABCLink);
	d->m_view->radioPhotoKABC->setEnabled(hasKABCLink);

	// Don't sync global photo with KABC if KABC is the source
	// or if they are no KABC link. (would create a break in timeline)
	if( selectedPhotoSource() == Kopete::MetaContact::SourceKABC || !hasKABCLink )
	{
		d->m_view->checkSyncPhotoKABC->setEnabled(false);
	}
	else
	{
		d->m_view->checkSyncPhotoKABC->setEnabled(true);
	}

	d->m_view->radioNicknameContact->setEnabled(d->currentIdentity->contacts().count());
	d->m_view->radioPhotoContact->setEnabled(!d->contactPhotoSourceList.isEmpty());

	d->m_view->comboNameContact->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceContact);
	d->m_view->lineNickname->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceCustom);

	d->m_view->comboPhotoContact->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceContact);
	d->m_view->comboPhotoURL->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceCustom);

	if(d->contactPhotoSourceList.isEmpty() )
	{
		d->m_view->comboPhotoContact->clear();
		d->m_view->comboPhotoContact->insertItem(i18n("No Contacts with Photo Support"));
		d->m_view->comboPhotoContact->setEnabled(false);
	}

	QImage photo;
	switch ( selectedPhotoSource() )
	{
		case Kopete::MetaContact::SourceKABC:
			photo = Kopete::photoFromKABC(a.uid());
			break;
		case Kopete::MetaContact::SourceContact:
			photo = Kopete::photoFromContact(selectedNameSourceContact());
			break;
		case Kopete::MetaContact::SourceCustom:
			photo = QImage(d->m_view->comboPhotoURL->url());
			break;
	}

	if(!photo.isNull())
		d->m_view->labelPhoto->setPixmap(QPixmap(photo.smoothScale(64, 92, QImage::ScaleMin)));
	else
		d->m_view->labelPhoto->setPixmap(QPixmap());

	emit changed(true);
}

void KopeteIdentityConfig::slotUpdateCurrentIdentity(const QString &selectedIdentity)
{
	kdDebug() << k_funcinfo << "Updating current identity." << endl;

	// Save the current identity detail, so we don't loose information.
	saveCurrentIdentity();

	// Change the current identity reflecting the combo box.
	d->selectedIdentity = selectedIdentity;
	d->currentIdentity = GlobalIdentitiesManager::self()->getIdentity(d->selectedIdentity);
	KopeteIdentityConfigPreferences::self()->setSelectedIdentity(d->selectedIdentity);
	KopeteIdentityConfigPreferences::self()->writeConfig();
	// Save global identities list.
	GlobalIdentitiesManager::self()->saveXML();

	// Reload the details.
	slotLoadNameSources();
	slotLoadPhotoSources();
}
	
void KopeteIdentityConfig::slotNewIdentity()
{
	bool ok;
	QString newIdentityName = KInputDialog::getText(i18n("New Identity"), i18n("Identity name:") , QString::null , &ok);
	
	if(newIdentityName.isEmpty() || !ok)
		return;

	
	GlobalIdentitiesManager::self()->createNewIdentity(newIdentityName);

	slotUpdateCurrentIdentity(newIdentityName);
	loadIdentities();
}

void KopeteIdentityConfig::slotCopyIdentity()
{
	bool ok;
	QString copyName = KInputDialog::getText(i18n("Copy Identity"), i18n("Identity name:") , QString::null, &ok);
	
	if(copyName.isEmpty() || !ok)
		return;

	
	if(!GlobalIdentitiesManager::self()->isIdentityPresent(copyName))
	{
		GlobalIdentitiesManager::self()->copyIdentity(copyName, d->selectedIdentity);
		
		slotUpdateCurrentIdentity(copyName);
		loadIdentities();
	}
	else
	{
		KMessageBox::error(this, i18n("An identity with the same name was found."), i18n("Identity Configuration"));
	}
}

void KopeteIdentityConfig::slotRenameIdentity()
{
	if(d->selectedIdentity.isNull())
		return;
	
	bool ok;
	QString renamedName = KInputDialog::getText(i18n("Rename Identity"), i18n("Identity name:") , d->selectedIdentity, &ok);

	if(renamedName.isEmpty() || !ok)
		return;


	if(renamedName.isEmpty())
		return;

	if(!GlobalIdentitiesManager::self()->isIdentityPresent(renamedName))
	{
		GlobalIdentitiesManager::self()->renameIdentity(d->selectedIdentity, renamedName);
		
		slotUpdateCurrentIdentity(renamedName);
		loadIdentities();
	}
	else
	{
		KMessageBox::error(this, i18n("An identity with the same name was found."), i18n("Identity Configuration"));
	}
}

void KopeteIdentityConfig::slotRemoveIdentity()
{
	kdDebug() << k_funcinfo << "Removing current identity." << endl;
	GlobalIdentitiesManager::self()->removeIdentity(d->selectedIdentity);
	// Reset the currentIdentity pointer. The currentIdentity object was deleted in GlobalIdentitiesManager.
	d->currentIdentity = 0;
	
	// Select the entry before(or after) the removed identity.
	int currentItem = d->m_view->comboSelectIdentity->currentItem();
	// Use the next item if the removed identity is the first in the comboBox.
	if(currentItem - 1 < 0)
	{
		currentItem++;
	}
	else
	{
		currentItem--;
	}
	d->m_view->comboSelectIdentity->setCurrentItem(currentItem);

	slotUpdateCurrentIdentity(d->m_view->comboSelectIdentity->currentText());
	loadIdentities();
}



void KopeteIdentityConfig::slotChangeAddressee()
{
	KABC::Addressee a = Kopete::UI::AddressBookSelectorDialog::getAddressee(i18n("Addressbook Association"), i18n("Choose the person who is yourself."), d->myself->metaContactId(), this);

	if ( !a.isEmpty() )
	{
		d->m_view->lineAddressee->setText(a.realName());
		KABC::StdAddressBook::self()->setWhoAmI(a);
		d->myself->setMetaContactId(a.uid());
	}

	emit changed(true);
}

void KopeteIdentityConfig::slotChangePhoto(const QString &photoUrl)
{
	QString saveLocation;
	
	QImage photo(photoUrl);
	// use KABC photo size 100x140
	photo = KPixmapRegionSelectorDialog::getSelectedImage( QPixmap(photo), 96, 96, this );

	if(!photo.isNull())
	{
		if(photo.width() > 96 || photo.height() > 96)
		{
			// Scale and crop the picture.
			photo = photo.smoothScale( 96, 96, QImage::ScaleMin );
			// crop image if not square
			if(photo.width() < photo.height()) 
				photo = photo.copy((photo.width()-photo.height())/2, 0, 96, 96);
			else if (photo.width() > photo.height())
				photo = photo.copy(0, (photo.height()-photo.width())/2, 96, 96);

		}
		else if (photo.width() < 32 || photo.height() < 32)
		{
			// Scale and crop the picture.
			photo = photo.smoothScale( 32, 32, QImage::ScaleMin );
			// crop image if not square
			if(photo.width() < photo.height())
				photo = photo.copy((photo.width()-photo.height())/2, 0, 32, 32);
			else if (photo.width() > photo.height())
				photo = photo.copy(0, (photo.height()-photo.width())/2, 32, 32);
	
		}
		else if (photo.width() != photo.height())
		{
			if(photo.width() < photo.height())
				photo = photo.copy((photo.width()-photo.height())/2, 0, photo.height(), photo.height());
			else if (photo.width() > photo.height())
				photo = photo.copy(0, (photo.height()-photo.width())/2, photo.height(), photo.height());
		}

		// Use MD5 hash to save the filename, so no problems will occur with the filename because of non-ASCII characters.
		// Bug 124175: My personnal picture doesn't appear cause of l10n
		QByteArray tempArray;
		QBuffer tempBuffer(tempArray);
		tempBuffer.open( IO_WriteOnly );
		photo.save(&tempBuffer, "PNG");
		KMD5 context(tempArray);
		// Save the image to a file.
		saveLocation = context.hexDigest() + ".png";
		saveLocation = locateLocal( "appdata", QString::fromUtf8("globalidentitiespictures/%1").arg( saveLocation ) );

		if(!photo.save(saveLocation, "PNG"))
		{
			KMessageBox::sorry(this, 
					i18n("An error occurred when trying to save the custom photo for %1 identity.").arg(d->selectedIdentity),
					i18n("Identity Configuration"));
		}
		d->m_view->comboPhotoURL->setURL(saveLocation);
		slotEnableAndDisableWidgets();
	}
	else
	{
		KMessageBox::sorry(this, 
					i18n("An error occurred when trying to save the custom photo for %1 identity.").arg(d->selectedIdentity),
					i18n("Identity Configuration"));
	}
}

void KopeteIdentityConfig::slotClearPhoto()
{
	d->m_view->comboPhotoURL->setURL( QString::null );
	slotEnableAndDisableWidgets();
}

void KopeteIdentityConfig::slotSettingsChanged()
{
	emit changed(true);
}

Kopete::MetaContact::PropertySource KopeteIdentityConfig::selectedNameSource() const
{
	if (d->m_view->radioNicknameKABC->isChecked())
		return Kopete::MetaContact::SourceKABC;
	if (d->m_view->radioNicknameContact->isChecked())
		return Kopete::MetaContact::SourceContact;
	if (d->m_view->radioNicknameCustom->isChecked())
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::MetaContact::PropertySource KopeteIdentityConfig::selectedPhotoSource() const
{
	if (d->m_view->radioPhotoKABC->isChecked())
		return Kopete::MetaContact::SourceKABC;
	if (d->m_view->radioPhotoContact->isChecked())
		return Kopete::MetaContact::SourceContact;
	if (d->m_view->radioPhotoCustom->isChecked())
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::Contact* KopeteIdentityConfig::selectedNameSourceContact() const
{
	Kopete::Contact *c = d->myself->contacts().at(d->m_view->comboNameContact->currentItem());
	return c ? c : 0L;
}

Kopete::Contact* KopeteIdentityConfig::selectedPhotoSourceContact() const
{
	if (d->contactPhotoSourceList.isEmpty())
		return 0L;

	Kopete::Contact *c = d->contactPhotoSourceList[d->m_view->comboPhotoContact->currentItem()];
	return c ? c : 0L;
}

#include "kopeteidentityconfig.moc"
