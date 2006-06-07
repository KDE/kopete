/*
    kopetelviprops.cpp

    Kopete Contactlist Properties GUI for Groups and MetaContacts

    Copyright (c) 2002-2003 by Stefan Gehn <metz AT gehn.net>
    Copyright (c) 2004 by Will Stephenson <lists@stevello.free-online.co.uk>
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

#include <kdialogbase.h>
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
#include <kurlrequester.h>

#include "kabcpersistence.h"
#include "kopeteaddrbookexport.h"
#include "kopetecontact.h"
#include "kopetegroup.h"
#include "kopetegroupviewitem.h"
#include "kopetemetacontactlvi.h"
#include "kopeteaccount.h"
#include "kopeteprotocol.h"
#include "addressbooklinkwidget.h"
#include "addressbookselectordialog.h"

#include "customnotificationprops.h"
#include "customnotifications.h"

const char MC_OFF[] = "metacontact_offline";
const char MC_ON[] = "metacontact_online";
const char MC_AW[] = "metacontact_away";
const char MC_UNK[] = "metacontact_unknown";


KopeteGVIProps::KopeteGVIProps(KopeteGroupViewItem *gvi, QWidget *parent, const char *name)
: KDialogBase(parent, name, true, i18n("Properties of Group %1").arg(gvi->group()->displayName()), Ok|Cancel, Ok, false)
{
	mainWidget = new KopeteGVIPropsWidget(this, "mainWidget");
	mainWidget->icnbOpen->setIconSize(KIcon::SizeSmall);
	mainWidget->icnbClosed->setIconSize(KIcon::SizeSmall);
	
	mNotificationProps = new CustomNotificationProps( this, gvi->group() );
	mainWidget->tabWidget->addTab( mNotificationProps->widget(), i18n( "Custom &Notifications" ) );

	setMainWidget(mainWidget);
	item = gvi;
	m_dirty = false;

	mainWidget->edtDisplayName->setText( item->group()->displayName() );

	mainWidget->chkUseCustomIcons->setChecked( item->group()->useCustomIcon() );

	QString openName = item->group()->icon( Kopete::ContactListElement::Open );
	if(openName.isEmpty())
		openName = KOPETE_GROUP_DEFAULT_OPEN_ICON;
	QString closeName = item->group()->icon( Kopete::ContactListElement::Closed );
	if(closeName.isEmpty())
		closeName = KOPETE_GROUP_DEFAULT_CLOSED_ICON;
	mainWidget->icnbOpen->setIcon( openName );
	mainWidget->icnbClosed->setIcon( closeName );

	connect( this, SIGNAL(okClicked()), this, SLOT( slotOkClicked() ) );
	connect( mainWidget->chkUseCustomIcons, SIGNAL( toggled( bool ) ),
		this, SLOT( slotUseCustomIconsToggled( bool ) ) );
	connect( mainWidget->icnbOpen, SIGNAL( iconChanged( QString ) ),
		SLOT( slotIconChanged() ) );
	connect( mainWidget->icnbClosed, SIGNAL( iconChanged( QString ) ),
		SLOT( slotIconChanged() ) );
	slotUseCustomIconsToggled( mainWidget->chkUseCustomIcons->isChecked() );
}

KopeteGVIProps::~KopeteGVIProps()
{
}

void KopeteGVIProps::slotOkClicked()
{
	if( mainWidget->edtDisplayName->text() != item->group()->displayName() )
	{
		item->group()->setDisplayName( mainWidget->edtDisplayName->text() );
		item->refreshDisplayName();
	}

	item->group()->setUseCustomIcon( mainWidget->chkUseCustomIcons->isChecked() );

	// only call setIcon if the icon was changed
	if( m_dirty )
	{
		item->group()->setIcon( mainWidget->icnbOpen->icon(),
			 Kopete::ContactListElement::Open );

		item->group()->setIcon( mainWidget->icnbClosed->icon(),
			Kopete::ContactListElement::Closed );
	}
	
	mNotificationProps->storeCurrentCustoms();
}

void KopeteGVIProps::slotUseCustomIconsToggled(bool on)
{
	mainWidget->lblOpen->setEnabled( on );
	mainWidget->icnbOpen->setEnabled( on );
	mainWidget->lblClosed->setEnabled( on );
	mainWidget->icnbClosed->setEnabled( on );
}

void KopeteGVIProps::slotIconChanged()
{
	m_dirty = true;
}

// =============================================================================


KopeteMetaLVIProps::KopeteMetaLVIProps(KopeteMetaContactLVI *lvi, QWidget *parent, const char *name)
: KDialogBase(parent, name, true, i18n("Properties of Meta Contact %1").arg(lvi->metaContact()->displayName()), Ok|Cancel, Ok, false)
{
	m_countPhotoCapable = 0;
	mainWidget = new KopeteMetaLVIPropsWidget( this, "mainWidget" );
	mainWidget->icnbOffline->setIconSize( KIcon::SizeSmall );
	mainWidget->icnbOnline->setIconSize( KIcon::SizeSmall );
	mainWidget->icnbAway->setIconSize( KIcon::SizeSmall );
	mainWidget->icnbUnknown->setIconSize( KIcon::SizeSmall );

	mNotificationProps = new CustomNotificationProps( this, lvi->metaContact() );
	// add a button to the notification props to get the sound from KABC
	// the widget's vert box layout, horiz box layout containing button, spacer, followed by a spacer
	QBoxLayout * vb = static_cast<QVBoxLayout*>( mNotificationProps->widget()->layout() );

	QHBoxLayout* hb = new QHBoxLayout( vb, -1, "soundFromKABClayout" );
	mFromKABC = new QPushButton( i18n( "Sync KABC..." ), mNotificationProps->widget(), "getSoundFromKABC" );
	hb->addWidget( mFromKABC ); // [ [Button] <-xxxxx-> ]
	hb->addStretch();
	vb->addStretch(); // vert spacer keeps the rest snug
	
	mainWidget->tabWidget->addTab( mNotificationProps->widget(), i18n( "Custom &Notifications" ) );
	setMainWidget( mainWidget );
	item = lvi;

	connect( mainWidget->radioNameKABC, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( mainWidget->radioNameContact, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( mainWidget->radioNameCustom, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( mainWidget->radioPhotoKABC, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( mainWidget->radioPhotoContact, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( mainWidget->radioPhotoCustom, SIGNAL(toggled(bool)), SLOT(slotEnableAndDisableWidgets()));
	connect( mainWidget->cmbPhotoUrl, SIGNAL(urlSelected(const QString &)), SLOT(slotEnableAndDisableWidgets()));
	connect( mainWidget->cmbAccountPhoto, SIGNAL(activated ( int )), SLOT(slotEnableAndDisableWidgets()));
	

	mainWidget->btnClearPhoto->setIconSet( SmallIconSet( QApplication::reverseLayout() ? "locationbar_erase" : "clear_left" ) );
	connect( mainWidget->btnClearPhoto, SIGNAL( clicked() ), this, SLOT( slotClearPhotoClicked() ) );
	connect( mainWidget->widAddresseeLink, SIGNAL( addresseeChanged( const KABC::Addressee & ) ), SLOT( slotAddresseeChanged( const KABC::Addressee & ) ) );
	mainWidget->chkUseCustomIcons->setChecked( item->metaContact()->useCustomIcon() );

	QString offlineName = item->metaContact()->icon( Kopete::ContactListElement::Offline );
	if(offlineName.isEmpty())
		offlineName = QString::fromLatin1(MC_OFF); // Default

	QString onlineName = item->metaContact()->icon( Kopete::ContactListElement::Online );
	if(onlineName.isEmpty())
		onlineName = QString::fromLatin1(MC_ON); // Default

	QString awayName = item->metaContact()->icon( Kopete::ContactListElement::Away );
	if(awayName.isEmpty())
		awayName = QString::fromLatin1(MC_AW); // Default

	QString unknownName = item->metaContact()->icon( Kopete::ContactListElement::Unknown );
	if(unknownName.isEmpty())
		unknownName = QString::fromLatin1(MC_UNK); // Default

	mainWidget->icnbOffline->setIcon( offlineName );
	mainWidget->icnbOnline->setIcon( onlineName );
	mainWidget->icnbAway->setIcon( awayName );
	mainWidget->icnbUnknown->setIcon( unknownName );

	mainWidget->widAddresseeLink->setMetaContact( lvi->metaContact() );

	mAddressBookUid = item->metaContact()->metaContactId();

	mExport = 0L;

	if ( !mAddressBookUid.isEmpty() )
	{
		KABC::AddressBook *ab = Kopete::KABCPersistence::self()->addressBook();
		KABC::Addressee a = ab->findByUid( mAddressBookUid );
		mainWidget->widAddresseeLink->setAddressee( a );

		if ( !a.isEmpty() )
		{
			mainWidget->btnImportKABC->setEnabled( true );
			mainWidget->btnExportKABC->setEnabled( true );
			mExport = new KopeteAddressBookExport( this, item->metaContact() );
			
			mSound = a.sound();
			mFromKABC->setEnabled( !( mSound.isIntern() || mSound.url().isEmpty() ) );
		}
	}
	
	slotLoadNameSources();
	slotLoadPhotoSources();

	connect( this, SIGNAL(okClicked()), this, SLOT( slotOkClicked() ) );
	connect( mainWidget->chkUseCustomIcons, SIGNAL( toggled( bool ) ),
		this, SLOT( slotUseCustomIconsToggled( bool ) ) );
	connect( mainWidget->btnImportKABC, SIGNAL( clicked() ),
		this, SLOT( slotImportClicked() ) );
	connect( mainWidget->btnExportKABC, SIGNAL( clicked() ),
					 this, SLOT( slotExportClicked() ) );
	connect( mFromKABC, SIGNAL( clicked() ),
		this, SLOT( slotFromKABCClicked() ) );
	connect( mNotificationProps->widget()->customSound, SIGNAL( openFileDialog( KURLRequester * )),
             SLOT( slotOpenSoundDialog( KURLRequester * )));

	slotUseCustomIconsToggled( mainWidget->chkUseCustomIcons->isChecked() );
	slotEnableAndDisableWidgets();
}

KopeteMetaLVIProps::~KopeteMetaLVIProps()
{
}


void KopeteMetaLVIProps::slotLoadNameSources()
{
	Kopete::Contact* trackingName = item->metaContact()->displayNameSourceContact();
	QPtrList< Kopete::Contact > cList = item->metaContact()->contacts();
	QPtrListIterator<Kopete::Contact> it( cList );
	mainWidget->cmbAccountName->clear();
	for( ; it.current(); ++it )
	{
		QString acct = it.current()->property( Kopete::Global::Properties::self()->nickName() ).value().toString() + " <" + it.current()->contactId() + ">";
		QPixmap acctIcon = it.current()->account()->accountIcon();
		mainWidget->cmbAccountName->insertItem( acctIcon, acct );
		
		// Select this item if it's the one we're tracking.
		if( it.current() == trackingName )
		{
			mainWidget->cmbAccountName->setCurrentItem( mainWidget->cmbAccountName->count() - 1 );
		}
	}

	mainWidget->edtDisplayName->setText( item->metaContact()->customDisplayName() );

	Kopete::MetaContact::PropertySource nameSource = item->metaContact()->displayNameSource();
	
	mainWidget->radioNameContact->setChecked(nameSource == Kopete::MetaContact::SourceContact);
	mainWidget->radioNameKABC->setChecked(nameSource == Kopete::MetaContact::SourceKABC);
	mainWidget->radioNameCustom->setChecked(nameSource == Kopete::MetaContact::SourceCustom);

}

void KopeteMetaLVIProps::slotLoadPhotoSources()
{
	// fill photo contact sources
	QPtrList< Kopete::Contact > cList = item->metaContact()->contacts();
	m_withPhotoContacts.clear();
	Kopete::Contact* trackingPhoto = item->metaContact()->photoSourceContact();
	mainWidget->cmbAccountPhoto->clear();
	QPtrListIterator<Kopete::Contact> itp( cList );
	for( ; itp.current(); ++itp )
	{
		Kopete::Contact *citem = itp.current();
		if ( citem->hasProperty( Kopete::Global::Properties::self()->photo().key() ) )
		{
			QString acct = citem->property( Kopete::Global::Properties::self()->nickName() ).value().toString() + " <" + citem->contactId() + ">";
			QPixmap acctIcon = citem->account()->accountIcon();
			mainWidget->cmbAccountPhoto->insertItem( acctIcon, acct );
			
			// Select this item if it's the one we're tracking.
			if( citem == trackingPhoto )
			{
				mainWidget->cmbAccountPhoto->setCurrentItem( mainWidget->cmbAccountPhoto->count() - 1 );
			}
			m_withPhotoContacts.insert(mainWidget->cmbAccountPhoto->count() - 1  , citem );
		}
	}
#if KDE_IS_VERSION(3,4,0)
	mainWidget->cmbPhotoUrl->setKURL(item->metaContact()->customPhoto().url());
#else
	mainWidget->cmbPhotoUrl->setURL(item->metaContact()->customPhoto().url());
#endif

	Kopete::MetaContact::PropertySource photoSource = item->metaContact()->photoSource();

	mainWidget->radioPhotoContact->setChecked(photoSource == Kopete::MetaContact::SourceContact);
	mainWidget->radioPhotoKABC->setChecked(photoSource == Kopete::MetaContact::SourceKABC);
	mainWidget->radioPhotoCustom->setChecked(photoSource == Kopete::MetaContact::SourceCustom);

	mainWidget->chkSyncPhoto->setChecked(item->metaContact()->isPhotoSyncedWithKABC());
}

void KopeteMetaLVIProps::slotEnableAndDisableWidgets()
{
	KABC::AddressBook *ab = Kopete::KABCPersistence::self()->addressBook();
	KABC::Addressee a = ab->findByUid( mAddressBookUid );
	bool validLink = ! a.isEmpty();
	// kabc source requires a kabc link
	mainWidget->radioNameKABC->setEnabled(validLink);
	// kabc source requires a kabc link
	mainWidget->radioPhotoKABC->setEnabled(validLink);
	// sync with kabc has no sense if we use kabc as source (sync kabc with kabc? uh?)
	// it has also no sense if they are no kabc link
	if( selectedPhotoSource() == Kopete::MetaContact::SourceKABC || !validLink )
	{
		mainWidget->chkSyncPhoto->setEnabled(false);
	}
	else
	{
		mainWidget->chkSyncPhoto->setEnabled(true);
	}

	mainWidget->radioNameContact->setEnabled(item->metaContact()->contacts().count());
	mainWidget->radioPhotoContact->setEnabled(!m_withPhotoContacts.isEmpty());

	mainWidget->cmbAccountName->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceContact);
	mainWidget->edtDisplayName->setEnabled(selectedNameSource() == Kopete::MetaContact::SourceCustom);

	mainWidget->cmbAccountPhoto->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceContact);
	mainWidget->cmbPhotoUrl->setEnabled(selectedPhotoSource() == Kopete::MetaContact::SourceCustom);
	
	if ( m_withPhotoContacts.isEmpty() )
	{
		mainWidget->cmbAccountPhoto->clear();
		mainWidget->cmbAccountPhoto->insertItem(i18n("No Contacts with Photo Support"));
		mainWidget->cmbAccountPhoto->setEnabled(false);
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
		photo = QImage(KURL::decode_string(mainWidget->cmbPhotoUrl->url()));
		break;
	}
	if( !photo.isNull() )
		mainWidget->photoLabel->setPixmap(QPixmap(photo.smoothScale( 64, 92, QImage::ScaleMin )));
	else
		mainWidget->photoLabel->setPixmap( QPixmap() );
}

Kopete::MetaContact::PropertySource KopeteMetaLVIProps::selectedNameSource() const
{
	if ( mainWidget->radioNameKABC->isChecked() )
		return Kopete::MetaContact::SourceKABC;
	if ( mainWidget->radioNameContact->isChecked() )
		return Kopete::MetaContact::SourceContact;
	if ( mainWidget->radioNameCustom->isChecked() )
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::MetaContact::PropertySource KopeteMetaLVIProps::selectedPhotoSource() const
{
	if ( mainWidget->radioPhotoKABC->isChecked() )
		return Kopete::MetaContact::SourceKABC;
	if ( mainWidget->radioPhotoContact->isChecked() )
		return Kopete::MetaContact::SourceContact;
	if ( mainWidget->radioPhotoCustom->isChecked() )
		return Kopete::MetaContact::SourceCustom;
	else
		return Kopete::MetaContact::SourceCustom;
}

Kopete::Contact* KopeteMetaLVIProps::selectedNameSourceContact() const
{
	Kopete::Contact *c= item->metaContact()->contacts().at( mainWidget->cmbAccountName->currentItem() );
	return c ? c : 0L;
}

Kopete::Contact* KopeteMetaLVIProps::selectedPhotoSourceContact() const
{
	if (m_withPhotoContacts.isEmpty())
		return 0L;
	Kopete::Contact *c = m_withPhotoContacts[mainWidget->cmbAccountPhoto->currentItem() ];
	return c ? c : 0L;
}

void KopeteMetaLVIProps::slotOkClicked()
{
	// update meta contact's UID
	item->metaContact()->setMetaContactId( mAddressBookUid );	
	//this has to be done first, in the case something is synced with KABC   (see bug 109494)
	
	// set custom display name
	if( mainWidget->edtDisplayName->text() != item->metaContact()->customDisplayName() )
		item->metaContact()->setDisplayName( mainWidget->edtDisplayName->text() );
	
	item->metaContact()->setDisplayNameSource(selectedNameSource());
	item->metaContact()->setDisplayNameSourceContact( selectedNameSourceContact() );
	
	// set photo source
	item->metaContact()->setPhotoSource(selectedPhotoSource());
	item->metaContact()->setPhotoSourceContact( selectedPhotoSourceContact() );
	if ( !mainWidget->cmbPhotoUrl->url().isEmpty())
		item->metaContact()->setPhoto(KURL::fromPathOrURL((mainWidget->cmbPhotoUrl->url())));
	item->metaContact()->setPhotoSyncedWithKABC( mainWidget->chkSyncPhoto->isChecked() );
	
	item->metaContact()->setUseCustomIcon(
		mainWidget->chkUseCustomIcons->isChecked() );

	// only call setIcon if any of the icons is not set to default icon
	if(
		mainWidget->icnbOffline->icon() != MC_OFF ||
		mainWidget->icnbOnline->icon() != MC_ON ||
		mainWidget->icnbAway->icon() != MC_AW ||
		mainWidget->icnbUnknown->icon() != MC_UNK )
	{
		item->metaContact()->setIcon( mainWidget->icnbOffline->icon(),
			 Kopete::ContactListElement::Offline );

		item->metaContact()->setIcon( mainWidget->icnbOnline->icon(),
			Kopete::ContactListElement::Online );

		item->metaContact()->setIcon( mainWidget->icnbAway->icon(),
			Kopete::ContactListElement::Away );

		item->metaContact()->setIcon( mainWidget->icnbUnknown->icon(),
			Kopete::ContactListElement::Unknown );
	}

	mNotificationProps->storeCurrentCustoms();
}

void KopeteMetaLVIProps::slotUseCustomIconsToggled(bool on)
{
	mainWidget->lblOffline->setEnabled( on );
	mainWidget->lblOnline->setEnabled( on );
	mainWidget->lblAway->setEnabled( on );
	mainWidget->lblUnknown->setEnabled( on );

	mainWidget->icnbOffline->setEnabled( on );
	mainWidget->icnbOnline->setEnabled( on );
	mainWidget->icnbAway->setEnabled( on );
	mainWidget->icnbUnknown->setEnabled( on );
}

void KopeteMetaLVIProps::slotAddresseeChanged( const KABC::Addressee & a )
{
	if ( !a.isEmpty() )
	{
		mSound = a.sound();
		mFromKABC->setEnabled( !( mSound.isIntern() || mSound.url().isEmpty() ) );
		mainWidget->btnExportKABC->setEnabled( true );
		mainWidget->btnImportKABC->setEnabled( true );
		// set/update the MC's addressee uin field
		mAddressBookUid = a.uid();
	}
	else
	{
		mainWidget->btnExportKABC->setEnabled( false );
		mainWidget->btnImportKABC->setEnabled( false );
		mAddressBookUid = QString::null;
		mainWidget->radioNameContact->setChecked( true );
		mainWidget->radioPhotoContact->setChecked( true );
	}
	slotEnableAndDisableWidgets();
}

void KopeteMetaLVIProps::slotExportClicked()
{
	item->metaContact()->setMetaContactId( mAddressBookUid );
	delete mExport;
	mExport = new KopeteAddressBookExport( this, item->metaContact() );
	if ( mExport->showDialog() == QDialog::Accepted )
		mExport->exportData();
}

void KopeteMetaLVIProps::slotImportClicked()
{
	item->metaContact()->setMetaContactId( mAddressBookUid );
	if ( Kopete::KABCPersistence::self()->syncWithKABC( item->metaContact() ) )
		KMessageBox::queuedMessageBox( this, KMessageBox::Information,
																	 i18n( "No contacts were imported from the address book." ),
																	 i18n( "No Change" ) );
}

void KopeteMetaLVIProps::slotFromKABCClicked()
{
	 mNotificationProps->widget()->customSound->setURL( mSound.url() );
}

void KopeteMetaLVIProps::slotOpenSoundDialog( KURLRequester *requester )
{
	// taken from kdelibs/kio/kfile/knotifydialog.cpp
	// only need to init this once
	requester->disconnect( SIGNAL( openFileDialog( KURLRequester * )),
						this, SLOT( slotOpenSoundDialog( KURLRequester * )));

	KFileDialog *fileDialog = requester->fileDialog();
	//fileDialog->setCaption( i18n("Select Sound File") );
	QStringList filters;
	filters << "audio/x-wav" << "audio/x-mp3" << "application/ogg"
			<< "audio/x-adpcm";
	fileDialog->setMimeFilter( filters );

	// find the first "sound"-resource that contains files
	QStringList soundDirs =
		KGlobal::dirs()->findDirs("data", "kopete/sounds");
	soundDirs += KGlobal::dirs()->resourceDirs( "sound" );

	if ( !soundDirs.isEmpty() ) {
		KURL soundURL;
		QDir dir;
		dir.setFilter( QDir::Files | QDir::Readable );
		QStringList::ConstIterator it = soundDirs.begin();
		while ( it != soundDirs.end() ) {
			dir = *it;
			if ( dir.isReadable() && dir.count() > 2 ) {
				soundURL.setPath( *it );
				fileDialog->setURL( soundURL );
				break;
			}
			++it;
		}
	}
}

void KopeteMetaLVIProps::slotClearPhotoClicked()
{
#if KDE_IS_VERSION(3,4,0)
	mainWidget->cmbPhotoUrl->setKURL( KURL() );
#else
	mainWidget->cmbPhotoUrl->setURL( QString::null );
#endif
	item->metaContact()->setPhoto( KURL() );

	slotEnableAndDisableWidgets();
}

#include "kopetelviprops.moc"
