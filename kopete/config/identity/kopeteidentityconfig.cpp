/*
    kopeteidentityconfig.cpp  -  Kopete Identity config page

    Copyright (c) 2005      by Michaël Larouche       <shock@shockdev.ca.tc>

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

#include <qlayout.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qapplication.h>

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

#include <kio/netaccess.h>

#include <kabc/addresseedialog.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>

#include "kabcpersistence.h"
#include "kopeteglobal.h"

#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"

#include "kopetecontactlist.h"
#include "addressbookselectordialog.h"

#include "kopeteconfig.h"

#include "kopeteidentityconfigbase.h"

typedef KGenericFactory<KopeteIdentityConfig, QWidget> KopeteIdentityConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_identityconfig, KopeteIdentityConfigFactory( "kcm_kopete_identityconfig" ) )

KopeteIdentityConfig::KopeteIdentityConfig(QWidget *parent, const char */*name*/, const QStringList &args) : KCModule( KopeteIdentityConfigFactory::instance(), parent, args)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_view = new KopeteIdentityConfigBase( this, "KopeteIdentityConfig::m_view" );

	addConfig( Kopete::Config::self(), m_view );

	myself = Kopete::ContactList::self()->myself();

	load();

	// Action signal/slots
	connect(m_view->buttonChangeAddressee, SIGNAL(clicked()), this, SLOT(slotChangeAddressee()));

	// Settings signal/slots
	connect(m_view->radioNicknameContact, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(m_view->radioNicknameCustom, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(m_view->radioNicknameKABC, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));

	connect(m_view->radioPhotoContact, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(m_view->radioPhotoCustom, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(m_view->radioPhotoKABC, SIGNAL(toggled(bool )), this, SLOT(slotEnableAndDisableWidgets()));
	connect(m_view->comboPhotoURL, SIGNAL(urlSelected(const QString& )), this, SLOT(slotEnableAndDisableWidgets()));

	connect(m_view->checkSyncPhotoKABC, SIGNAL(toggled(bool )), this, SLOT(slotSettingsChanged()));
	connect(m_view->lineNickname, SIGNAL(textChanged(const QString& )), this, SLOT(slotSettingsChanged()));	
	connect(m_view->comboNameContact, SIGNAL(activated(int )), this, SLOT(slotSettingsChanged()));
	connect(m_view->comboPhotoContact, SIGNAL(activated(int )), this, SLOT(slotEnableAndDisableWidgets()));

	connect(this, SIGNAL(changed()), this, SLOT(slotSettingsChanged()));
}

void KopeteIdentityConfig::load()
{
	KCModule::load();

	// Populate the name contact ComboBox
	slotLoadNameSources();
	// Populate the photo contact ComboBOx
	slotLoadPhotoSources();

	KABC::Addressee a = KABC::StdAddressBook::self()->whoAmI();
	// Load the address book link
	if (!a.isEmpty())
	{
		m_view->lineAddressee->setText(a.realName());
	}

	slotEnableAndDisableWidgets();
}

void KopeteIdentityConfig::save()
{
	KCModule::save();
	
	// Don't save the new global identity if it's not activated.
	if(m_view->kcfg_EnableGlobalIdentity->isChecked())
	{
		// Save the myself metacontact settings.
		// Nickname settings.
		if(m_view->lineNickname->text() != myself->customDisplayName())
			myself->setDisplayName(m_view->lineNickname->text());
	
		myself->setDisplayNameSource(selectedNameSource());
		myself->setDisplayNameSourceContact(selectedNameSourceContact());
		
		// Photo settings
		myself->setPhotoSource(selectedPhotoSource());
		myself->setPhotoSourceContact(selectedPhotoSourceContact());
		if(!m_view->comboPhotoURL->url().isEmpty())
			myself->setPhoto(m_view->comboPhotoURL->url());
		myself->setPhotoSyncedWithKABC(m_view->checkSyncPhotoKABC->isChecked());
	}
	
	load();
}

void KopeteIdentityConfig::slotLoadNameSources()
{
	Kopete::Contact *nameSourceContact = myself->displayNameSourceContact();
	
	QPtrList<Kopete::Contact> contactList = myself->contacts();
	QPtrListIterator<Kopete::Contact> it(contactList);

	m_view->comboNameContact->clear();

	for(; it.current(); ++it)
	{
		QString account = it.current()->property(Kopete::Global::Properties::self()->nickName()).value().toString() + " <" + it.current()->contactId() + ">";
		QPixmap accountIcon = it.current()->account()->accountIcon();
		m_view->comboNameContact->insertItem(accountIcon,  account);
		
		// Select this item if it's the one we're tracking.
		if(it.current() == nameSourceContact)
		{
			m_view->comboNameContact->setCurrentItem(m_view->comboNameContact->count() - 1);
		}
	}

	m_view->lineNickname->setText(myself->customDisplayName());

	Kopete::MetaContact::PropertySource nameSource = myself->displayNameSource();
	
	m_view->radioNicknameCustom->setChecked(nameSource == Kopete::MetaContact::SourceCustom);
	m_view->radioNicknameKABC->setChecked(nameSource == Kopete::MetaContact::SourceKABC);
	m_view->radioNicknameContact->setChecked(nameSource == Kopete::MetaContact::SourceContact);
}

void KopeteIdentityConfig::slotLoadPhotoSources()
{
	Kopete::Contact *photoSourceContact = myself->photoSourceContact();
	
	QPtrList<Kopete::Contact> contactList = myself->contacts();
	QPtrListIterator<Kopete::Contact> it(contactList);

	m_view->comboPhotoContact->clear();
	contactPhotoSourceList.clear();

	for(; it.current(); ++it)
	{
		Kopete::Contact *currentContact = it.current();
		if(currentContact->hasProperty(Kopete::Global::Properties::self()->photo().key()))
		{
			QString account = currentContact->property(Kopete::Global::Properties::self()->nickName()).value().toString() + " <" + currentContact->contactId() + ">";
			QPixmap accountIcon = currentContact->account()->accountIcon();

			m_view->comboPhotoContact->insertItem(accountIcon,  account);
			contactPhotoSourceList.insert(m_view->comboPhotoContact->count() - 1, currentContact);

			// Select this item if it's the one we're tracking.
			if(currentContact == photoSourceContact)
			{
				m_view->comboPhotoContact->setCurrentItem(m_view->comboPhotoContact->count() - 1);
			}
		}
	}

	m_view->comboPhotoURL->setURL(myself->customPhoto().url());
	Kopete::MetaContact::PropertySource photoSource = myself->photoSource();
	
	m_view->radioPhotoCustom->setChecked(photoSource == Kopete::MetaContact::SourceCustom);
	m_view->radioPhotoContact->setChecked(photoSource == Kopete::MetaContact::SourceContact);
	m_view->radioPhotoKABC->setChecked(photoSource == Kopete::MetaContact::SourceKABC);

	m_view->checkSyncPhotoKABC->setChecked(myself->isPhotoSyncedWithKABC());
}

void KopeteIdentityConfig::slotEnableAndDisableWidgets()
{
	KABC::Addressee a = KABC::StdAddressBook::self()->whoAmI();
	bool hasKABCLink = !a.isEmpty();

	m_view->radioNicknameKABC->setEnabled(hasKABCLink);
	m_view->radioPhotoKABC->setEnabled(hasKABCLink);

	// Don't sync global photo with KABC if KABC is the source
	// or if they are no KABC link. (would create a break in timeline)
	if( selectedPhotoSource() == Kopete::MetaContact::SourceKABC || !hasKABCLink )
	{
		m_view->checkSyncPhotoKABC->setEnabled(false);
	}
	else
	{
		m_view->checkSyncPhotoKABC->setEnabled(true);
	}

	m_view->radioNicknameContact->setEnabled(myself->contacts().count());
	m_view->radioPhotoContact->setEnabled(!contactPhotoSourceList.isEmpty());
	
	m_view->comboNameContact->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceContact);
	m_view->lineNickname->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceCustom);

	m_view->comboPhotoContact->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceContact);
	m_view->comboPhotoURL->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceCustom);
	
	if(contactPhotoSourceList.isEmpty() )
	{
		m_view->comboPhotoContact->clear();
		m_view->comboPhotoContact->insertItem(i18n("No contacts with photo support"));
		m_view->comboPhotoContact->setEnabled(false);
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
			photo = QImage(m_view->comboPhotoURL->url());
			break;
	}

	if(!photo.isNull())
		m_view->labelPhoto->setPixmap(QPixmap(photo.smoothScale(64, 92, QImage::ScaleMin)));

	emit changed(true);
}

void KopeteIdentityConfig::slotChangeAddressee()
{
	KABC::Addressee a = Kopete::UI::AddressBookSelectorDialog::getAddressee(i18n("Addressbook association"), i18n("Choose the person who is yourself."), myself->metaContactId(), this);

	if ( !a.isEmpty() )
	{
		m_view->lineAddressee->setText(a.realName());
		KABC::StdAddressBook::self()->setWhoAmI(a);
		myself->setMetaContactId(a.uid());
	}
	
	emit changed(true);
}

void KopeteIdentityConfig::slotSettingsChanged()
{
	emit changed(true);
}

Kopete::MetaContact::PropertySource KopeteIdentityConfig::selectedNameSource() const
{
	if (m_view->radioNicknameKABC->isChecked())
		return Kopete::MetaContact::SourceKABC;
	if (m_view->radioNicknameContact->isChecked())
		return Kopete::MetaContact::SourceContact;
	if (m_view->radioNicknameCustom->isChecked())
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::MetaContact::PropertySource KopeteIdentityConfig::selectedPhotoSource() const
{
	if (m_view->radioPhotoKABC->isChecked())
		return Kopete::MetaContact::SourceKABC;
	if (m_view->radioPhotoContact->isChecked())
		return Kopete::MetaContact::SourceContact;
	if (m_view->radioPhotoCustom->isChecked())
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::Contact* KopeteIdentityConfig::selectedNameSourceContact() const
{
	Kopete::Contact *c = myself->contacts().at(m_view->comboNameContact->currentItem());
	return c ? c : 0L;
}

Kopete::Contact* KopeteIdentityConfig::selectedPhotoSourceContact() const
{
	if (contactPhotoSourceList.isEmpty())
		return 0L;

	Kopete::Contact *c = contactPhotoSourceList[m_view->comboPhotoContact->currentItem()];
	return c ? c : 0L;
}

#include "kopeteidentityconfig.moc"
