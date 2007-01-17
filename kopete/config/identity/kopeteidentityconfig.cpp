/*
    kopeteidentityconfig.cpp  -  Kopete Identity config page

    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

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
#include <QPixmap>
#include <QVBoxLayout>
#include <qbuffer.h>

// KDE includes
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kurlrequester.h>
#include <kinputdialog.h>
#include <kpixmapregionselectordialog.h>
#include <kcodecs.h>

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
#include "kopetegeneralsettings.h"

// Local includes
#include "globalidentitiesmanager.h"
#include "kopeteidentityconfigpreferences.h"

class KopeteIdentityConfig::Private
{
public:
	Private() : myself(0L), currentIdentity(0L), selectedIdentity("")
	{}

	Kopete::MetaContact *myself;
	Kopete::MetaContact *currentIdentity;
	
	QMap<int, Kopete::Contact*> contactPhotoSourceList;
	QString selectedIdentity;
};

typedef KGenericFactory<KopeteIdentityConfig, QWidget> KopeteIdentityConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_identityconfig, KopeteIdentityConfigFactory( "kcm_kopete_identityconfig" ) )

KopeteIdentityConfig::KopeteIdentityConfig(QWidget *parent, const QStringList &args)
: KCModule( KopeteIdentityConfigFactory::instance(), parent, args)
{
	d = new Private;
	setupUi( this );

	// Setup KConfigXT link with GUI.
	addConfig( Kopete::GeneralSettings::self(), this );

	// Load config
	KopeteIdentityConfigPreferences::self()->readConfig();

	// Load from XML the identities.
	GlobalIdentitiesManager::self()->loadXML();

	d->myself = Kopete::ContactList::self()->myself();
	
	// Set the latest selected Identity.
	d->selectedIdentity = KopeteIdentityConfigPreferences::self()->selectedIdentity();
	kDebug() << k_funcinfo << "Latest loaded identity: " << d->selectedIdentity << endl;

	// If the latest selected Identity is not present anymore, use a fallback identity.
	if( !GlobalIdentitiesManager::self()->isIdentityPresent(d->selectedIdentity) )
	{
		QMap<QString, Kopete::MetaContact*>::iterator it = GlobalIdentitiesManager::self()->getGlobalIdentitiesList().begin();
		d->selectedIdentity = it.key();
	}
	else
	{
		// Update the latest identity with myself Metacontact.
		GlobalIdentitiesManager::self()->updateIdentity(d->selectedIdentity, d->myself);
	}
	d->currentIdentity = GlobalIdentitiesManager::self()->getIdentity(d->selectedIdentity);
	
	// Set icon for KPushButton
	buttonNewIdentity->setIcon(KIcon("new"));
	buttonCopyIdentity->setIcon(KIcon("editcopy"));
	buttonRenameIdentity->setIcon(KIcon("edit"));
	buttonRemoveIdentity->setIcon(KIcon("delete_user"));
	buttonClearPhoto->setIcon( KIcon( (QApplication::layoutDirection() == Qt::RightToLeft) ? "locationbar_erase" : "clear_left" ) );

	load(); // Load Configuration

	// Action signal/slots
	connect(buttonChangeAddressee, SIGNAL(clicked()), this, SLOT(slotChangeAddressee()));
	connect(comboSelectIdentity, SIGNAL(activated(const QString &)), this, SLOT(slotUpdateCurrentIdentity(const QString& )));
	connect(buttonNewIdentity, SIGNAL(clicked()), this, SLOT(slotNewIdentity()));
	connect(buttonCopyIdentity, SIGNAL(clicked()), this, SLOT(slotCopyIdentity()));
	connect(buttonRenameIdentity, SIGNAL(clicked()), this, SLOT(slotRenameIdentity()));
	connect(buttonRemoveIdentity, SIGNAL(clicked()), this, SLOT(slotRemoveIdentity()));
	connect(comboPhotoURL, SIGNAL(urlSelected(const KUrl& )), this, SLOT(slotChangePhoto(const KUrl&)));
	connect(buttonClearPhoto, SIGNAL(clicked()), this, SLOT(slotClearPhoto()));

	// Settings signal/slots
	connect(radioNicknameContact, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(radioNicknameCustom, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(radioNicknameKABC, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));

	connect(radioPhotoContact, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(radioPhotoCustom, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(radioPhotoKABC, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));

	connect(checkSyncPhotoKABC, SIGNAL(toggled(bool )), this, SLOT(slotSettingsChanged()));
	connect(lineNickname, SIGNAL(textChanged(const QString& )), this, SLOT(slotSettingsChanged()));
	connect(comboNameContact, SIGNAL(activated(int )), this, SLOT(slotSettingsChanged()));
	connect(comboPhotoContact, SIGNAL(activated(int )), this, SLOT(slotEnableAndDisableWidgets()));
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
		lineAddressee->setText(a.realName());
	}

	slotEnableAndDisableWidgets();
}

void KopeteIdentityConfig::save()
{
	KCModule::save();

	saveCurrentIdentity();

	// Don't save the new global identity if it's not activated.
	if(kcfg_EnableGlobalIdentity->isChecked())
	{
		// Save the myself metacontact settings.
		// Nickname settings.
		if(lineNickname->text() != d->myself->customDisplayName())
			d->myself->setDisplayName(lineNickname->text());
		
		d->myself->setDisplayNameSource(selectedNameSource());
		d->myself->setDisplayNameSourceContact(selectedNameSourceContact());

		// Photo settings
		d->myself->setPhotoSource(selectedPhotoSource());
		d->myself->setPhotoSourceContact(selectedPhotoSourceContact());
		if(!comboPhotoURL->url().isEmpty())
			d->myself->setPhoto(comboPhotoURL->url());
		else
			d->myself->setPhoto( KUrl() );
		d->myself->setPhotoSyncedWithKABC(checkSyncPhotoKABC->isChecked());
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
	comboSelectIdentity->clear();

	QMap<QString, Kopete::MetaContact*> identitiesList = GlobalIdentitiesManager::self()->getGlobalIdentitiesList();
	QMap<QString, Kopete::MetaContact*>::iterator it;
	QMap<QString, Kopete::MetaContact*>::iterator end = identitiesList.end();

	int count=0, selectedIndex=0;
	for(it = identitiesList.begin(); it != end; ++it)
	{
		comboSelectIdentity->addItem(it.key());
		if(it.key() == d->selectedIdentity)
		{
			selectedIndex = count;
		}
		count++;
	}

	comboSelectIdentity->setCurrentIndex(selectedIndex);
	buttonRemoveIdentity->setEnabled(count == 1 ? false : true);
}

void KopeteIdentityConfig::saveCurrentIdentity()
{
	kDebug() << k_funcinfo << "Saving data of current identity." << endl;
	// Ignore saving when removing a identity
	if(!d->currentIdentity)
		return;

	if(lineNickname->text() != d->currentIdentity->customDisplayName())
		d->currentIdentity->setDisplayName(lineNickname->text());
		
	d->currentIdentity->setDisplayNameSource(selectedNameSource());
	d->currentIdentity->setDisplayNameSourceContact(selectedNameSourceContact());

	// Photo settings
	d->currentIdentity->setPhotoSource(selectedPhotoSource());
	d->currentIdentity->setPhotoSourceContact(selectedPhotoSourceContact());
	if(!comboPhotoURL->url().isEmpty())
		d->currentIdentity->setPhoto(comboPhotoURL->url());
	else
		d->currentIdentity->setPhoto( KUrl() );
	d->currentIdentity->setPhotoSyncedWithKABC(checkSyncPhotoKABC->isChecked());
}

void KopeteIdentityConfig::slotLoadNameSources()
{
	Kopete::Contact *nameSourceContact = d->currentIdentity->displayNameSourceContact();

	QList<Kopete::Contact*> contactList = d->myself->contacts(); // Use myself contact PtrList. Safer.
	QList<Kopete::Contact*>::iterator it;

	comboNameContact->clear();

	for( it = contactList.begin(); it != contactList.end(); ++it)
	{
		
		QString account = (*it)->property(Kopete::Global::Properties::self()->nickName()).value().toString() + " <" + (*it)->contactId() + '>';
		QPixmap accountIcon = (*it)->account()->accountIcon();
		comboNameContact->addItem( QIcon(accountIcon),  account);

		// Select this item if it's the one we're tracking.
		if((*it) == nameSourceContact)
		{
			comboNameContact->setCurrentIndex(comboNameContact->count() - 1);
		}
	}

	lineNickname->setText(d->currentIdentity->customDisplayName());

	Kopete::MetaContact::PropertySource nameSource = d->currentIdentity->displayNameSource();

	radioNicknameCustom->setChecked(nameSource == Kopete::MetaContact::SourceCustom);
	radioNicknameKABC->setChecked(nameSource == Kopete::MetaContact::SourceKABC);
	radioNicknameContact->setChecked(nameSource == Kopete::MetaContact::SourceContact);
}

void KopeteIdentityConfig::slotLoadPhotoSources()
{
	Kopete::Contact *photoSourceContact = d->currentIdentity->photoSourceContact();

	QList<Kopete::Contact*> contactList = d->myself->contacts(); // Use myself contact PtrList. Safer.
	QList<Kopete::Contact*>::iterator it;

	comboPhotoContact->clear();
	comboPhotoURL->clear();
	d->contactPhotoSourceList.clear();

	for( it = contactList.begin(); it != contactList.end(); ++it)
	{
		Kopete::Contact *currentContact = (*it);
		if(currentContact->hasProperty(Kopete::Global::Properties::self()->photo().key()))
		{
			QString account = currentContact->property(Kopete::Global::Properties::self()->nickName()).value().toString() + " <" + currentContact->contactId() + '>';
			QPixmap accountIcon = currentContact->account()->accountIcon();

			comboPhotoContact->addItem( QIcon(accountIcon),  account);
			d->contactPhotoSourceList.insert(comboPhotoContact->count() - 1, currentContact);

			// Select this item if it's the one we're tracking.
			if(currentContact == photoSourceContact)
			{
				comboPhotoContact->setCurrentIndex(comboPhotoContact->count() - 1);
			}
		}
	}

	comboPhotoURL->setUrl(d->currentIdentity->customPhoto().url());
	Kopete::MetaContact::PropertySource photoSource = d->currentIdentity->photoSource();

	radioPhotoCustom->setChecked(photoSource == Kopete::MetaContact::SourceCustom);
	radioPhotoContact->setChecked(photoSource == Kopete::MetaContact::SourceContact);
	radioPhotoKABC->setChecked(photoSource == Kopete::MetaContact::SourceKABC);

	checkSyncPhotoKABC->setChecked(d->currentIdentity->isPhotoSyncedWithKABC());
}

void KopeteIdentityConfig::slotEnableAndDisableWidgets()
{
	KABC::Addressee a = KABC::StdAddressBook::self()->whoAmI();
	bool hasKABCLink = !a.isEmpty();

	radioNicknameKABC->setEnabled(hasKABCLink);
	radioPhotoKABC->setEnabled(hasKABCLink);

	// Don't sync global photo with KABC if KABC is the source
	// or if they are no KABC link. (would create a break in timeline)
	if( selectedPhotoSource() == Kopete::MetaContact::SourceKABC || !hasKABCLink )
	{
		checkSyncPhotoKABC->setEnabled(false);
	}
	else
	{
		checkSyncPhotoKABC->setEnabled(true);
	}

	radioNicknameContact->setEnabled(d->currentIdentity->contacts().count());
	radioPhotoContact->setEnabled(!d->contactPhotoSourceList.isEmpty());

	comboNameContact->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceContact);
	lineNickname->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceCustom);

	comboPhotoContact->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceContact);
	comboPhotoURL->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceCustom);

	if(d->contactPhotoSourceList.isEmpty() )
	{
		comboPhotoContact->clear();
		comboPhotoContact->addItem(i18n("No Contacts with Photo Support"));
		comboPhotoContact->setEnabled(false);
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
			photo = QImage(comboPhotoURL->url().url());
			break;
	}

	if(!photo.isNull())
		labelPhoto->setPixmap(QPixmap::fromImage(photo.scaled(64, 92, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	else
		labelPhoto->setPixmap(QPixmap());

	emit changed(true);
}

void KopeteIdentityConfig::slotUpdateCurrentIdentity(const QString &selectedIdentity)
{
	kDebug() << k_funcinfo << "Updating current identity." << endl;

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
	kDebug() << k_funcinfo << "Removing current identity." << endl;
	GlobalIdentitiesManager::self()->removeIdentity(d->selectedIdentity);
	// Reset the currentIdentity pointer. The currentIdentity object was deleted in GlobalIdentitiesManager.
	d->currentIdentity = 0;
	
	// Select the entry before(or after) the removed identity.
	int currentItem = comboSelectIdentity->currentIndex();
	// Use the next item if the removed identity is the first in the comboBox.
	if(currentItem - 1 < 0)
	{
		currentItem++;
	}
	else
	{
		currentItem--;
	}
	comboSelectIdentity->setCurrentIndex(currentItem);

	slotUpdateCurrentIdentity(comboSelectIdentity->currentText());
	loadIdentities();
}



void KopeteIdentityConfig::slotChangeAddressee()
{
	KABC::Addressee a = Kopete::UI::AddressBookSelectorDialog::getAddressee(i18n("Addressbook Association"), i18n("Choose the person who is yourself."), d->myself->metaContactId(), this);

	if ( !a.isEmpty() )
	{
		lineAddressee->setText(a.realName());
		KABC::StdAddressBook::self()->setWhoAmI(a);
		d->myself->setMetaContactId(a.uid());
	}

	emit changed(true);
}

void KopeteIdentityConfig::slotChangePhoto(const KUrl &photoUrl)
{
	QString saveLocation;
	
	QImage photo(photoUrl.path());
	// use KABC photo size 100x140
	photo = KPixmapRegionSelectorDialog::getSelectedImage( QPixmap::fromImage(photo), 96, 96, this );

	if(!photo.isNull())
	{
		if(photo.width() > 96 || photo.height() > 96)
		{
			// Scale and crop the picture.
			photo = photo.scaled( 96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
			// crop image if not square
			if(photo.width() < photo.height()) 
				photo = photo.copy((photo.width()-photo.height())/2, 0, 96, 96);
			else if (photo.width() > photo.height())
				photo = photo.copy(0, (photo.height()-photo.width())/2, 96, 96);

		}
		else if (photo.width() < 32 || photo.height() < 32)
		{
			// Scale and crop the picture.
			photo = photo.scaled( 96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
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
		QBuffer tempBuffer(&tempArray);
		tempBuffer.open( IO_WriteOnly );
		photo.save(&tempBuffer, "PNG");
		KMD5 context(tempArray);
		// Save the image to a file.
		saveLocation = context.hexDigest() + ".png";
		saveLocation = KStandardDirs::locateLocal( "appdata", QString::fromUtf8("globalidentitiespictures/%1").arg( saveLocation ) );

		if(!photo.save(saveLocation, "PNG"))
		{
			KMessageBox::sorry(this, 
					i18n("An error occurred when trying to save the custom photo for %1 identity.", d->selectedIdentity),
					i18n("Identity Configuration"));
		}
		comboPhotoURL->setUrl(saveLocation);
		slotEnableAndDisableWidgets();
	}
	else
	{
		KMessageBox::sorry(this, 
					i18n("An error occurred when trying to save the custom photo for %1 identity.", d->selectedIdentity),
					i18n("Identity Configuration"));
	}
}

void KopeteIdentityConfig::slotClearPhoto()
{
	comboPhotoURL->setUrl( KUrl() );
	slotEnableAndDisableWidgets();
}

void KopeteIdentityConfig::slotSettingsChanged()
{
	emit changed(true);
}

Kopete::MetaContact::PropertySource KopeteIdentityConfig::selectedNameSource() const
{
	if (radioNicknameKABC->isChecked())
		return Kopete::MetaContact::SourceKABC;
	if (radioNicknameContact->isChecked())
		return Kopete::MetaContact::SourceContact;
	if (radioNicknameCustom->isChecked())
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::MetaContact::PropertySource KopeteIdentityConfig::selectedPhotoSource() const
{
	if (radioPhotoKABC->isChecked())
		return Kopete::MetaContact::SourceKABC;
	if (radioPhotoContact->isChecked())
		return Kopete::MetaContact::SourceContact;
	if (radioPhotoCustom->isChecked())
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::Contact* KopeteIdentityConfig::selectedNameSourceContact() const
{
	Kopete::Contact *c = d->myself->contacts().at(comboNameContact->currentIndex());
	return c ? c : 0L;
}

Kopete::Contact* KopeteIdentityConfig::selectedPhotoSourceContact() const
{
	if (d->contactPhotoSourceList.isEmpty())
		return 0L;

	Kopete::Contact *c = d->contactPhotoSourceList[comboPhotoContact->currentIndex()];
	return c ? c : 0L;
}

#include "kopeteidentityconfig.moc"
