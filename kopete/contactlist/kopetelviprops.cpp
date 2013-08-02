/*
    kopetelviprops.cpp

    Kopete Contactlist Properties GUI for Groups and MetaContacts

    Copyright (c) 2002-2003 by Stefan Gehn <metz@gehn.net>
    Copyright (c) 2004 by Will Stephenson <wstephenson@kde.org>
    Copyright (c) 2004-2005 by Duncan Mac-Vicar P. <duncan@kde.org>
    
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetelviprops.h"

#include <kdebug.h>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QSize>

#include <kfiledialog.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kabc/addresseedialog.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>
#include <kstandarddirs.h>

#include "kabcpersistence.h"
#include "kopeteaddrbookexport.h"
#include "kopetecontact.h"
#include "kopetegroup.h"
#include "kopeteaccount.h"
#include "kopeteprotocol.h"
#include "addressbooklinkwidget.h"
#include "avatardialog.h"

#include "customnotificationprops.h"

const QLatin1String MC_OFF( "user-offline" );
const QLatin1String MC_ON( "user-online" );
const QLatin1String MC_AW( "user-away" );
const QLatin1String MC_UNK( "metacontact_unknown" );

// KDE4 port notes:
// setIcon has changed, so it has been commented out. 
// also cmbPhotoUrl  is gone completely. Let's pray for it

KopeteGVIProps::KopeteGVIProps(Kopete::Group *group, QWidget *parent)
: KDialog(parent), mGroup( group )
{
	setCaption( i18n("Properties of Group %1", mGroup->displayName()) );
	setButtons( Ok | Cancel );

	mainWidget = new QWidget( this );
	mainWidget->setObjectName( "mainWidget" );
	ui_mainWidget = new Ui::KopeteGVIPropsWidget;
	ui_mainWidget->setupUi( mainWidget );

	ui_mainWidget->icnbOpen->setIconSize(QSize(KIconLoader::SizeSmall,KIconLoader::SizeSmall));
	ui_mainWidget->icnbClosed->setIconSize(QSize(KIconLoader::SizeSmall,KIconLoader::SizeSmall));
	QPair<QString,QString> context=qMakePair( QString::fromLatin1("group") , QString::number(mGroup->groupId() ) );
	mNotificationProps = new CustomNotificationProps( this, context );

	QWidget* npMainWidget = new QWidget();
	QVBoxLayout* vbLayout = new QVBoxLayout( npMainWidget );
	vbLayout->addWidget( mNotificationProps->widget() );
	ui_mainWidget->tabWidget->addTab( npMainWidget, i18n( "Custom &Notifications" ) );

	setMainWidget(mainWidget);
	m_dirty = false;

	ui_mainWidget->edtDisplayName->setText( mGroup->displayName() );

	ui_mainWidget->chkUseCustomIcons->setChecked( mGroup->useCustomIcon() );

// 	QString openName = mGroup->icon( Kopete::ContactListElement::Open );
// 	if(openName.isEmpty())
// 		openName = KOPETE_GROUP_DEFAULT_OPEN_ICON;
// 	QString closeName = mGroup->icon( Kopete::ContactListElement::Closed );
// 	if(closeName.isEmpty())
// 		closeName = KOPETE_GROUP_DEFAULT_CLOSED_ICON;
//	ui_mainWidget->icnbOpen->setIcon( openName );
//	ui_mainWidget->icnbClosed->setIcon( closeName );

	connect( this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()) );
	connect( ui_mainWidget->chkUseCustomIcons, SIGNAL(toggled(bool)),
		this, SLOT(slotUseCustomIconsToggled(bool)) );
	connect( ui_mainWidget->icnbOpen, SIGNAL(iconChanged(QString)),
		SLOT(slotIconChanged()) );
	connect( ui_mainWidget->icnbClosed, SIGNAL(iconChanged(QString)),
		SLOT(slotIconChanged()) );
	slotUseCustomIconsToggled( ui_mainWidget->chkUseCustomIcons->isChecked() );
}

KopeteGVIProps::~KopeteGVIProps()
{
	delete ui_mainWidget;
}

void KopeteGVIProps::slotOkClicked()
{
	if( ui_mainWidget->edtDisplayName->text() != mGroup->displayName() )
	{
		mGroup->setDisplayName( ui_mainWidget->edtDisplayName->text() );
	}

	mGroup->setUseCustomIcon( ui_mainWidget->chkUseCustomIcons->isChecked() );

	// only call setIcon if the icon was changed
	if( m_dirty )
	{
/*		mGroup->setIcon( ui_mainWidget->icnbOpen->icon(),
			 Kopete::ContactListElement::Open );

		mGroup->setIcon( ui_mainWidget->icnbClosed->icon(),
			Kopete::ContactListElement::Closed );
*/	}
	
	mNotificationProps->storeCurrentCustoms();
}

void KopeteGVIProps::slotUseCustomIconsToggled(bool on)
{
	ui_mainWidget->lblOpen->setEnabled( on );
	ui_mainWidget->icnbOpen->setEnabled( on );
	ui_mainWidget->lblClosed->setEnabled( on );
	ui_mainWidget->icnbClosed->setEnabled( on );
}

void KopeteGVIProps::slotIconChanged()
{
	m_dirty = true;
}

// =============================================================================


KopeteMetaLVIProps::KopeteMetaLVIProps(Kopete::MetaContact *metaContact, QWidget *parent)
: KDialog(parent), mMetaContact( metaContact )
{
	setCaption( i18n("Properties of Meta Contact %1", mMetaContact->displayName()) );
	setButtons( Ok | Cancel );
	m_countPhotoCapable = 0;

	mainWidget = new QWidget( this );
	mainWidget->setObjectName( "mainWidget" );
	ui_mainWidget = new Ui::KopeteMetaLVIPropsWidget;
	ui_mainWidget->setupUi( mainWidget );

	ui_mainWidget->icnbOffline->setIconSize( QSize(KIconLoader::SizeSmall,KIconLoader::SizeSmall) );
	ui_mainWidget->icnbOnline->setIconSize( QSize(KIconLoader::SizeSmall,KIconLoader::SizeSmall) );
	ui_mainWidget->icnbAway->setIconSize( QSize(KIconLoader::SizeSmall,KIconLoader::SizeSmall) );
	ui_mainWidget->icnbUnknown->setIconSize( QSize(KIconLoader::SizeSmall,KIconLoader::SizeSmall) );

	QPair<QString,QString> context=qMakePair( QString::fromLatin1("contact"), mMetaContact->metaContactId().toString() );
	mNotificationProps = new CustomNotificationProps( this, context  );
	// add a button to the notification props to get the sound from KABC
	// the widget's vert box layout, horiz box layout containing button, spacer, followed by a spacer
	QBoxLayout * vb = static_cast<QVBoxLayout*>( mNotificationProps->widget()->layout() );

	QHBoxLayout* hb = new QHBoxLayout();
	vb->addItem( hb );
	hb->setMargin( -1 );
	hb->setObjectName( "soundFromKABClayout" );

	mFromKABC = new QPushButton( i18n( "Sync KABC..." ), mNotificationProps->widget() );
	mFromKABC->setObjectName( QLatin1String("getSoundFromKABC") );
	hb->addWidget( mFromKABC ); // [ [Button] <-xxxxx-> ]
	hb->addStretch();
	vb->addStretch(); // vert spacer keeps the rest snug

	QWidget* npMainWidget = new QWidget();
	QVBoxLayout* vbLayout = new QVBoxLayout( npMainWidget );
	vbLayout->addWidget( mNotificationProps->widget() );

	ui_mainWidget->tabWidget->addTab( npMainWidget, i18n( "Custom &Notifications" ) );
	setMainWidget( mainWidget );

	connect( ui_mainWidget->radioNameKABC, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( ui_mainWidget->radioNameContact, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( ui_mainWidget->radioNameCustom, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( ui_mainWidget->radioPhotoKABC, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( ui_mainWidget->radioPhotoContact, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( ui_mainWidget->radioPhotoCustom, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( ui_mainWidget->cmbAccountPhoto, SIGNAL(activated(int)), SLOT(slotEnableAndDisableWidgets()));
	

	ui_mainWidget->btnClearPhoto->setIcon( KIcon( (QApplication::layoutDirection() == Qt::RightToLeft) ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl" ) );
	connect( ui_mainWidget->btnClearPhoto, SIGNAL(clicked()), this, SLOT(slotClearPhotoClicked()) );
	connect( ui_mainWidget->widAddresseeLink, SIGNAL(addresseeChanged(KABC::Addressee)), SLOT(slotAddresseeChanged(KABC::Addressee)) );
	connect( ui_mainWidget->btnChoosePhoto, SIGNAL(clicked()), this, SLOT(slotSelectPhoto()));
	ui_mainWidget->chkUseCustomIcons->setChecked( mMetaContact->useCustomIcon() );

	QString offlineName = mMetaContact->icon( Kopete::ContactListElement::Offline );
	if(offlineName.isEmpty())
		offlineName = MC_OFF; // Default

	QString onlineName = mMetaContact->icon( Kopete::ContactListElement::Online );
	if(onlineName.isEmpty())
		onlineName = MC_ON; // Default

	QString awayName = mMetaContact->icon( Kopete::ContactListElement::Away );
	if(awayName.isEmpty())
		awayName = MC_AW; // Default

	QString unknownName = mMetaContact->icon( Kopete::ContactListElement::Unknown );
	if(unknownName.isEmpty())
		unknownName = MC_UNK; // Default

//	ui_mainWidget->icnbOffline->setIcon( offlineName );
//	ui_mainWidget->icnbOnline->setIcon( onlineName );
//	ui_mainWidget->icnbAway->setIcon( awayName );
//	ui_mainWidget->icnbUnknown->setIcon( unknownName );

	ui_mainWidget->widAddresseeLink->setMetaContact( mMetaContact );

	mAddressBookUid = mMetaContact->kabcId();

	mExport = 0L;

	if ( !mAddressBookUid.isEmpty() )
	{
		KABC::AddressBook *ab = Kopete::KABCPersistence::self()->addressBook();
		KABC::Addressee a = ab->findByUid( mAddressBookUid );
		ui_mainWidget->widAddresseeLink->setAddressee( a );

		if ( !a.isEmpty() )
		{
			ui_mainWidget->btnImportKABC->setEnabled( true );
			ui_mainWidget->btnExportKABC->setEnabled( true );
			mExport = new KopeteAddressBookExport( this, mMetaContact );
			
			mSound = a.sound();
			mFromKABC->setEnabled( !( mSound.isIntern() || mSound.url().isEmpty() ) );
		}
	}
	
	slotLoadNameSources();
	slotLoadPhotoSources();

	connect( this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()) );
	connect( ui_mainWidget->chkUseCustomIcons, SIGNAL(toggled(bool)),
		this, SLOT(slotUseCustomIconsToggled(bool)) );
	connect( ui_mainWidget->btnImportKABC, SIGNAL(clicked()),
		this, SLOT(slotImportClicked()) );
	connect( ui_mainWidget->btnExportKABC, SIGNAL(clicked()),
					 this, SLOT(slotExportClicked()) );
	connect( mFromKABC, SIGNAL(clicked()),
		this, SLOT(slotFromKABCClicked()) );

	slotUseCustomIconsToggled( ui_mainWidget->chkUseCustomIcons->isChecked() );
	slotEnableAndDisableWidgets();
}

KopeteMetaLVIProps::~KopeteMetaLVIProps()
{
	delete ui_mainWidget;
}


void KopeteMetaLVIProps::slotLoadNameSources()
{
	Kopete::Contact* trackingName = mMetaContact->displayNameSourceContact();
	QList<Kopete::Contact*> cList = mMetaContact->contacts();
	QList<Kopete::Contact*>::iterator it = cList.begin();
	ui_mainWidget->cmbAccountName->clear();
	for( ; it != cList.end(); ++it )
	{
		QString acct;
		QVariant acctData;
		QVariant acctContact = QVariant::fromValue(static_cast<QObject *>(*it));
		QIcon acctIcon = QPixmap((*it)->account()->accountIcon());
		bool isTrackingName = ((*it) == trackingName);

		acct = (*it)->customName() + " <" + (*it)->contactId() + "> " + i18n("(custom name)");
		acctData = QVariantList() << acctContact << QVariant((uint)Kopete::Contact::CustomName);
		ui_mainWidget->cmbAccountName->addItem( acctIcon, acct, acctData );
		if (isTrackingName && (*it)->preferredNameType() == Kopete::Contact::CustomName)
			ui_mainWidget->cmbAccountName->setCurrentIndex( ui_mainWidget->cmbAccountName->count() - 1 );

		acct = (*it)->nickName() + " <" + (*it)->contactId() + "> " + i18n("(nick name)");
		acctData = QVariantList() << acctContact << QVariant((uint)Kopete::Contact::NickName);
		ui_mainWidget->cmbAccountName->addItem( acctIcon, acct, acctData );
		if (isTrackingName && (*it)->preferredNameType() == Kopete::Contact::NickName)
			ui_mainWidget->cmbAccountName->setCurrentIndex( ui_mainWidget->cmbAccountName->count() - 1 );

		acct = (*it)->formattedName() + " <" + (*it)->contactId() + "> " + i18n("(formatted name)");
		acctData = QVariantList() << acctContact << QVariant((uint)Kopete::Contact::FormattedName);
		ui_mainWidget->cmbAccountName->addItem( acctIcon, acct, acctData );
		if (isTrackingName && (*it)->preferredNameType() == Kopete::Contact::FormattedName)
			ui_mainWidget->cmbAccountName->setCurrentIndex( ui_mainWidget->cmbAccountName->count() - 1 );

		acct = (*it)->contactId() + " " + i18n("(contact id)");
		acctData = QVariantList() << acctContact << QVariant((uint)Kopete::Contact::ContactId);
		ui_mainWidget->cmbAccountName->addItem( acctIcon, acct, acctData );
		if (isTrackingName && (*it)->preferredNameType() == Kopete::Contact::ContactId)
			ui_mainWidget->cmbAccountName->setCurrentIndex( ui_mainWidget->cmbAccountName->count() - 1 );
	}

	ui_mainWidget->edtDisplayName->setText( mMetaContact->customDisplayName() );

	Kopete::MetaContact::PropertySource nameSource = mMetaContact->displayNameSource();
	
	ui_mainWidget->radioNameContact->setChecked(nameSource == Kopete::MetaContact::SourceContact);
	ui_mainWidget->radioNameKABC->setChecked(nameSource == Kopete::MetaContact::SourceKABC);
	ui_mainWidget->radioNameCustom->setChecked(nameSource == Kopete::MetaContact::SourceCustom);

}

void KopeteMetaLVIProps::slotLoadPhotoSources()
{
	// fill photo contact sources
	QList<Kopete::Contact*> cList = mMetaContact->contacts();
	m_withPhotoContacts.clear();
	Kopete::Contact* trackingPhoto = mMetaContact->photoSourceContact();
	ui_mainWidget->cmbAccountPhoto->clear();
	QList<Kopete::Contact*>::iterator itp = cList.begin();
	for( ; itp != cList.end(); ++itp )
	{
		Kopete::Contact *citem = (*itp);
		if ( citem->hasProperty( Kopete::Global::Properties::self()->photo().key() ) )
		{
			QString acct = citem->displayName() + " <" + citem->contactId() + '>';
			QPixmap acctIcon = citem->account()->accountIcon();
			ui_mainWidget->cmbAccountPhoto->addItem( QIcon(acctIcon), acct );
			
			// Select this item if it's the one we're tracking.
			if( citem == trackingPhoto )
			{
				ui_mainWidget->cmbAccountPhoto->setCurrentIndex( ui_mainWidget->cmbAccountPhoto->count() - 1 );
			}
			m_withPhotoContacts.insert(ui_mainWidget->cmbAccountPhoto->count() - 1  , citem );
		}
	}

	m_photoPath = mMetaContact->customPhoto().path();

	Kopete::MetaContact::PropertySource photoSource = mMetaContact->photoSource();

	ui_mainWidget->radioPhotoContact->setChecked(photoSource == Kopete::MetaContact::SourceContact);
	ui_mainWidget->radioPhotoKABC->setChecked(photoSource == Kopete::MetaContact::SourceKABC);
	ui_mainWidget->radioPhotoCustom->setChecked(photoSource == Kopete::MetaContact::SourceCustom);

	ui_mainWidget->chkSyncPhoto->setChecked(mMetaContact->isPhotoSyncedWithKABC());
}

void KopeteMetaLVIProps::slotSelectPhoto()
{
	const QString path = Kopete::UI::AvatarDialog::getAvatar(this, m_photoPath);
	if (path.isNull())
		return;

	m_photoPath = path;
	slotEnableAndDisableWidgets();
}

void KopeteMetaLVIProps::slotEnableAndDisableWidgets()
{
	KABC::AddressBook *ab = Kopete::KABCPersistence::self()->addressBook();
	KABC::Addressee a = ab->findByUid( mAddressBookUid );
	bool validLink = ! a.isEmpty();
	// kabc source requires a kabc link
	ui_mainWidget->radioNameKABC->setEnabled(validLink);
	// kabc source requires a kabc link
	ui_mainWidget->radioPhotoKABC->setEnabled(validLink);
	// sync with kabc has no sense if we use kabc as source (sync kabc with kabc? uh?)
	// it has also no sense if they are no kabc link
	if( selectedPhotoSource() == Kopete::MetaContact::SourceKABC || !validLink )
	{
		ui_mainWidget->chkSyncPhoto->setEnabled(false);
	}
	else
	{
		ui_mainWidget->chkSyncPhoto->setEnabled(true);
	}

	ui_mainWidget->radioNameContact->setEnabled(mMetaContact->contacts().count());
	ui_mainWidget->radioPhotoContact->setEnabled(!m_withPhotoContacts.isEmpty());

	ui_mainWidget->cmbAccountName->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceContact);
	ui_mainWidget->edtDisplayName->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceCustom);

	ui_mainWidget->cmbAccountPhoto->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceContact);
	ui_mainWidget->btnChoosePhoto->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceCustom);
	
	if ( m_withPhotoContacts.isEmpty() )
	{
		ui_mainWidget->cmbAccountPhoto->clear();
		ui_mainWidget->cmbAccountPhoto->addItem(i18n("No Contacts with Photo Support"));
		ui_mainWidget->cmbAccountPhoto->setEnabled(false);
	}

	QImage photo;
	switch ( selectedPhotoSource() )
	{
		case Kopete::MetaContact::SourceKABC:
		photo = Kopete::photoFromKABC(mAddressBookUid);
		break;
		case Kopete::MetaContact::SourceContact:
		photo = Kopete::photoFromContact(selectedPhotoSourceContact());
		break;
		case Kopete::MetaContact::SourceCustom:
		photo = QImage(m_photoPath);
		break;
	}
	if( !photo.isNull() )
		ui_mainWidget->photoLabel->setPixmap(QPixmap::fromImage(photo.scaled(96, 96)));
	else
		ui_mainWidget->photoLabel->setPixmap( QPixmap() );
}

Kopete::MetaContact::PropertySource KopeteMetaLVIProps::selectedNameSource() const
{
	if ( ui_mainWidget->radioNameKABC->isChecked() )
		return Kopete::MetaContact::SourceKABC;
	if ( ui_mainWidget->radioNameContact->isChecked() )
		return Kopete::MetaContact::SourceContact;
	if ( ui_mainWidget->radioNameCustom->isChecked() )
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::MetaContact::PropertySource KopeteMetaLVIProps::selectedPhotoSource() const
{
	if ( ui_mainWidget->radioPhotoKABC->isChecked() )
		return Kopete::MetaContact::SourceKABC;
	if ( ui_mainWidget->radioPhotoContact->isChecked() )
		return Kopete::MetaContact::SourceContact;
	if ( ui_mainWidget->radioPhotoCustom->isChecked() )
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::Contact* KopeteMetaLVIProps::selectedNameSourceContact() const
{
	QVariantList data = ui_mainWidget->cmbAccountName->itemData(ui_mainWidget->cmbAccountName->currentIndex()).toList();
	QObject *object = data.at(0).value<QObject *>();
	return static_cast<Kopete::Contact *>(object);
}

void KopeteMetaLVIProps::setContactsNameTypes()
{
	QVariantList data = ui_mainWidget->cmbAccountName->itemData(ui_mainWidget->cmbAccountName->currentIndex()).toList();
	QObject *object = data.at(0).value<QObject *>();
	Kopete::Contact *contact = static_cast<Kopete::Contact *>(object);
	Kopete::Contact::NameType nameType = (Kopete::Contact::NameType)data.at(1).toUInt();
	Kopete::MetaContact::PropertySource nameSource = selectedNameSource();

	QList<Kopete::Contact*> cList = mMetaContact->contacts();
	QList<Kopete::Contact*>::iterator it = cList.begin();
	for( ; it != cList.end(); ++it )
	{
		if (nameSource == Kopete::MetaContact::SourceContact && (*it) == contact)
			contact->setPreferredNameType(nameType);
		else
			contact->setPreferredNameType(Kopete::Contact::CustomName);
	}

}

Kopete::Contact* KopeteMetaLVIProps::selectedPhotoSourceContact() const
{
	return m_withPhotoContacts.value( ui_mainWidget->cmbAccountPhoto->currentIndex(), 0 );
}

void KopeteMetaLVIProps::slotOkClicked()
{
	// update meta contact's UID
	mMetaContact->setKabcId( mAddressBookUid );
	//this has to be done first, in the case something is synced with KABC   (see bug 109494)
	
	// set custom display name
	if( ui_mainWidget->edtDisplayName->text() != mMetaContact->customDisplayName() )
		mMetaContact->setDisplayName( ui_mainWidget->edtDisplayName->text() );
	
	mMetaContact->setDisplayNameSource(selectedNameSource());
	mMetaContact->setDisplayNameSourceContact( selectedNameSourceContact() );
	
	setContactsNameTypes();
	
	// set photo source
	mMetaContact->setPhotoSource(selectedPhotoSource());
	mMetaContact->setPhotoSourceContact( selectedPhotoSourceContact() );
	if ( !m_photoPath.isEmpty())
		mMetaContact->setPhoto(KUrl(m_photoPath));
	mMetaContact->setPhotoSyncedWithKABC( ui_mainWidget->chkSyncPhoto->isChecked() );
	
	mMetaContact->setUseCustomIcon(
		ui_mainWidget->chkUseCustomIcons->isChecked() );

	// only call setIcon if any of the icons is not set to default icon
/*
	if(
		ui_mainWidget->icnbOffline->icon() != MC_OFF ||
		ui_mainWidget->icnbOnline->icon() != MC_ON ||
		ui_mainWidget->icnbAway->icon() != MC_AW ||
		ui_mainWidget->icnbUnknown->icon() != MC_UNK )
	{
		mMetaContact->setIcon( ui_mainWidget->icnbOffline->icon(),
			 Kopete::ContactListElement::Offline );

		mMetaContact->setIcon( ui_mainWidget->icnbOnline->icon(),
			Kopete::ContactListElement::Online );

		mMetaContact->setIcon( ui_mainWidget->icnbAway->icon(),
			Kopete::ContactListElement::Away );

		mMetaContact->setIcon( ui_mainWidget->icnbUnknown->icon(),
			Kopete::ContactListElement::Unknown );
	}
*/
	mNotificationProps->storeCurrentCustoms();
}

void KopeteMetaLVIProps::slotUseCustomIconsToggled(bool on)
{
	ui_mainWidget->lblOffline->setEnabled( on );
	ui_mainWidget->lblOnline->setEnabled( on );
	ui_mainWidget->lblAway->setEnabled( on );
	ui_mainWidget->lblUnknown->setEnabled( on );

	ui_mainWidget->icnbOffline->setEnabled( on );
	ui_mainWidget->icnbOnline->setEnabled( on );
	ui_mainWidget->icnbAway->setEnabled( on );
	ui_mainWidget->icnbUnknown->setEnabled( on );
}

void KopeteMetaLVIProps::slotAddresseeChanged( const KABC::Addressee & a )
{
	if ( !a.isEmpty() )
	{
		mSound = a.sound();
		mFromKABC->setEnabled( !( mSound.isIntern() || mSound.url().isEmpty() ) );
		ui_mainWidget->btnExportKABC->setEnabled( true );
		ui_mainWidget->btnImportKABC->setEnabled( true );
		// set/update the MC's addressee uin field
		mAddressBookUid = a.uid();
	}
	else
	{
		ui_mainWidget->btnExportKABC->setEnabled( false );
		ui_mainWidget->btnImportKABC->setEnabled( false );
		mAddressBookUid.clear();
		ui_mainWidget->radioNameContact->setChecked( true );
		ui_mainWidget->radioPhotoContact->setChecked( true );
	}
	slotEnableAndDisableWidgets();
}

void KopeteMetaLVIProps::slotExportClicked()
{
	mMetaContact->setKabcId( mAddressBookUid );
	delete mExport;
	mExport = new KopeteAddressBookExport( this, mMetaContact );
	if ( mExport->showDialog() == QDialog::Accepted )
		mExport->exportData();
}

void KopeteMetaLVIProps::slotImportClicked()
{
	mMetaContact->setKabcId( mAddressBookUid );
	if ( Kopete::KABCPersistence::self()->syncWithKABC( mMetaContact ) )
		KMessageBox::queuedMessageBox( this, KMessageBox::Information,
																	 i18n( "No contacts were imported from the address book." ),
																	 i18n( "No Change" ) );
}

void KopeteMetaLVIProps::slotFromKABCClicked()
{
#if 0 
	mNotificationProps->widget()->customSound->setUrl( mSound.url() );
#endif
}

void KopeteMetaLVIProps::slotOpenSoundDialog( KUrlRequester *requester )
{
	// taken from kdelibs/kio/kfile/knotifydialog.cpp
	// only need to init this once
	requester->disconnect( SIGNAL(openFileDialog(KUrlRequester*)),
						this, SLOT(slotOpenSoundDialog(KUrlRequester*)));

	KFileDialog *fileDialog = requester->fileDialog();
	//fileDialog->setCaption( i18n("Select Sound File") );
	QStringList filters;
	filters << "audio/x-wav" << "audio/mpeg" << "application/ogg"
			<< "audio/x-adpcm";
	fileDialog->setMimeFilter( filters );

	// find the first "sound"-resource that contains files
	QStringList soundDirs =
		KGlobal::dirs()->findDirs("data", "kopete/sounds");
	soundDirs += KGlobal::dirs()->resourceDirs( "sound" );

	if ( !soundDirs.isEmpty() ) {
		KUrl soundURL;
		QDir dir;
		dir.setFilter( QDir::Files | QDir::Readable );
		QStringList::ConstIterator it = soundDirs.constBegin();
		while ( it != soundDirs.constEnd() ) {
			dir = *it;
			if ( dir.isReadable() && dir.count() > 2 ) {
				soundURL.setPath( *it );
				fileDialog->setUrl( soundURL );
				break;
			}
			++it;
		}
	}
}

void KopeteMetaLVIProps::slotClearPhotoClicked()
{
	m_photoPath.clear();
	mMetaContact->setPhoto( KUrl() );

	slotEnableAndDisableWidgets();
}

#include "kopetelviprops.moc"
