/*
    kopetelviprops.cpp

    Kopete Contactlist Properties GUI for Groups and MetaContacts

    Copyright (c) 2002-2003 by Stefan Gehn <metz AT gehn.net>
    Copyright (c) 2004 by Will Stephenson <lists@stevello.free-online.co.uk>
    Copyright (c) 2004 by Duncan Mac-Vicar P. <duncan@kde.org>
    
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

#include <klocale.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qcombobox.h>

#include <kdialogbase.h>
#include <kfiledialog.h>
#include <kicondialog.h>
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
#include "kopetemetacontact.h"
#include "kopetenotifyclient.h"
#include "kopetemetacontactlvi.h"
#include "kopeteaccount.h"
#include "kopeteprotocol.h"
#include "linkaddressbookui.h"

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

	mainWidget->edtDisplayName->setText( item->metaContact()->displayName() );
	mainWidget->chkTrackChildDisplayName->setChecked( item->metaContact()->nameSource() != 0 );
	mainWidget->chkTrackChildPhoto->setChecked( item->metaContact()->photoSource() != 0 );
	mainWidget->chkTrackChildDisplayName->setEnabled( item->metaContact()->contacts().count() > 0 );
	mainWidget->chkTrackChildPhoto->setEnabled( item->metaContact()->contacts().count() > 0 );
	mainWidget->chkSyncPhoto->setEnabled( mainWidget->chkTrackChildPhoto->isEnabled() );
	mainWidget->chkSyncPhoto->setChecked( item->metaContact()->isPhotoSyncedWithKABC() );
	mainWidget->cmbAccount->setEnabled( mainWidget->chkTrackChildDisplayName->isChecked() );
	
	slotSetNameComboEnabled(mainWidget->chkTrackChildDisplayName->isChecked());
	slotSetPhotoComboEnabled(mainWidget->chkTrackChildPhoto->isChecked());
	
	Kopete::Contact* trackingName = item->metaContact()->nameSource();
	QPtrList< Kopete::Contact > cList = item->metaContact()->contacts();
	QPtrListIterator<Kopete::Contact> it( cList );
	for( ; it.current(); ++it )
	{
		QString acct = it.current()->property( Kopete::Global::Properties::self()->nickName() ).value().toString() + " <" + it.current()->contactId() + ">";
		QPixmap acctIcon = it.current()->account()->accountIcon();
		mainWidget->cmbAccount->insertItem( acctIcon, acct );
		
		// Select this item if it's the one we're tracking.
		if( it.current() == trackingName )
		{
			mainWidget->cmbAccount->setCurrentItem( mainWidget->cmbAccount->count() - 1 );
		}
	}

	m_withPhotoContacts.clear();
	Kopete::Contact* trackingPhoto = item->metaContact()->photoSource();
	QPtrListIterator<Kopete::Contact> itp( cList );
	for( ; itp.current(); ++itp )
	{
		Kopete::Contact *citem = itp.current();
		if ( citem->hasProperty( Kopete::Global::Properties::self()->photo().key() ) )
		{
			m_countPhotoCapable++;
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

	if ( ! m_countPhotoCapable )
	{
		slotSetPhotoComboEnabled(false);
		mainWidget->chkTrackChildPhoto->setEnabled(false);
		mainWidget->chkSyncPhoto->setEnabled(false);
		mainWidget->cmbAccountPhoto->insertItem(i18n("No contacts with photo support"));
	}
	
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

	QString kabcUid = item->metaContact()->metaContactId();

	mExport = 0L;

	if ( !kabcUid.isEmpty() )
	{
		KABC::AddressBook *ab = Kopete::KABCPersistence::self()->addressBook();
		KABC::Addressee a = ab->findByUid( kabcUid );

		if(!a.isEmpty())
		{
			mainWidget->edtAddressee->setText( a.realName() );
			mainWidget->btnSelectAddressee->setEnabled( true );
			mainWidget->btnImportKABC->setEnabled( true );
			mainWidget->btnExportKABC->setEnabled( true );
			mainWidget->edtAddressee->setEnabled( true );
			mainWidget->lblAddressee->setEnabled( true );
			mainWidget->chkHasAddressbookEntry->setChecked( true );
			mExport = new KopeteAddressBookExport( this, item->metaContact() );
			
			mSound = a.sound();
			mFromKABC->setEnabled( !( mSound.isIntern() || mSound.url().isEmpty() ) );
		}
	}
	
	connect( this, SIGNAL(okClicked()), this, SLOT( slotOkClicked() ) );
	connect( mainWidget->chkUseCustomIcons, SIGNAL( toggled( bool ) ),
		this, SLOT( slotUseCustomIconsToggled( bool ) ) );
	connect( mainWidget->chkHasAddressbookEntry, SIGNAL( toggled( bool ) ),
		this, SLOT( slotHasAddressbookEntryToggled( bool ) ) );
	connect( mainWidget->btnSelectAddressee, SIGNAL( clicked() ),
		this, SLOT( slotSelectAddresseeClicked() ) );
	connect( mainWidget->btnImportKABC, SIGNAL( clicked() ),
		this, SLOT( slotImportClicked() ) );
	connect( mainWidget->btnExportKABC, SIGNAL( clicked() ),
					 this, SLOT( slotExportClicked() ) );
	connect( mFromKABC, SIGNAL( clicked() ),
		this, SLOT( slotFromKABCClicked() ) );
	connect( mNotificationProps->widget()->customSound, SIGNAL( openFileDialog( KURLRequester * )),
             SLOT( slotOpenSoundDialog( KURLRequester * )));

	connect( mainWidget->chkTrackChildPhoto, SIGNAL(toggled(bool)), SLOT(slotSetPhotoComboEnabled(bool)));
	connect( mainWidget->chkTrackChildDisplayName, SIGNAL(toggled(bool)), SLOT(slotSetNameComboEnabled(bool)));
	slotUseCustomIconsToggled( mainWidget->chkUseCustomIcons->isChecked() );
}

KopeteMetaLVIProps::~KopeteMetaLVIProps()
{
}

void KopeteMetaLVIProps::slotSetPhotoComboEnabled( bool on )
{
	mainWidget->cmbAccountPhoto->setEnabled(on);
	mainWidget->chkSyncPhoto->setEnabled(on);
	mainWidget->lblPhotoAccount->setEnabled(on);
}

void KopeteMetaLVIProps::slotSetNameComboEnabled( bool on )
{
	mainWidget->cmbAccount->setEnabled(on);
	mainWidget->lblAccountName->setEnabled(on);
}

void KopeteMetaLVIProps::slotOkClicked()
{
	if( mainWidget->edtDisplayName->text() != item->metaContact()->displayName() )
	{
		item->metaContact()->setDisplayName( mainWidget->edtDisplayName->text() );
	}
	
	// set name source
	if ( mainWidget->chkTrackChildDisplayName->isChecked() )
		item->metaContact()->setNameSource( item->metaContact()->contacts().at( mainWidget->cmbAccount->currentItem() ) );
	else
		item->metaContact()->setNameSource( 0L );
	
	// set photo source
	if ( mainWidget->chkTrackChildPhoto->isChecked() )
	{
		item->metaContact()->setPhotoSource( m_withPhotoContacts[ mainWidget->cmbAccountPhoto->currentItem() ] );
		item->metaContact()->setPhotoSyncedWithKABC( mainWidget->chkSyncPhoto->isChecked() );
	}
	else
	{
		item->metaContact()->setPhotoSource( 0L );
		item->metaContact()->setPhotoSyncedWithKABC(false);
	}
	
	
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
	// if no kabc link, remove any existing link
	if ( !mainWidget->chkHasAddressbookEntry->isChecked() )
		item->metaContact()->setMetaContactId( QString::null );
	
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

void KopeteMetaLVIProps::slotHasAddressbookEntryToggled( bool on )
{
	mainWidget->lblAddressee->setEnabled( on );
	mainWidget->edtAddressee->setEnabled( on );
	mainWidget->btnSelectAddressee->setEnabled( on );
	if ( !on )
		mainWidget->btnImportKABC->setEnabled( false );
		mainWidget->btnExportKABC->setEnabled( false );
}

void KopeteMetaLVIProps::slotSelectAddresseeClicked()
{
	LinkAddressbookUI dialog( item->metaContact(), this, "link_addr_book_dialog" );
	if ( dialog.exec() == QDialog::Rejected )
		return;
	
	KABC::Addressee a = dialog.addressee();
	if ( a.isEmpty() )
	{
		mainWidget->edtAddressee->setText( QString::null ) ;
		mainWidget->btnExportKABC->setEnabled( false );
		mainWidget->btnImportKABC->setEnabled( false );
		mFromKABC->setEnabled( false );
	}
	else
	{
		mSound = a.sound();
		mFromKABC->setEnabled( !( mSound.isIntern() || mSound.url().isEmpty() ) );
		mainWidget->btnExportKABC->setEnabled( true );
		mainWidget->btnImportKABC->setEnabled( true );
		// set the lineedit to the Addressee's name
		mainWidget->edtAddressee->setText( a.realName() );
		// set/update the MC's addressee uin field
		item->metaContact()->setMetaContactId( a.uid() );
		delete mExport;
		mExport = new KopeteAddressBookExport( this, item->metaContact() );
	}
}

void KopeteMetaLVIProps::slotExportClicked()
{
	if ( mExport->showDialog() == QDialog::Accepted )
		mExport->exportData();
}

void KopeteMetaLVIProps::slotImportClicked()
{
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

#include "kopetelviprops.moc"
