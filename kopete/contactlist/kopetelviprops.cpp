/*
    kopetelviprops.cpp

    Kopete Contactlist Properties GUI for Groups and MetaContacts

    Copyright (c) 2002-2003 by Stefan Gehn            <metz AT gehn.net>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include <klocale.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <kdialogbase.h>
#include <kicondialog.h>
#include <kabc/addresseedialog.h>
#include <kabc/stdaddressbook.h>

#include "kopeteaddrbookexport.h"

#include "kopetegroup.h"
#include "kopetegroupviewitem.h"

#include "kopetemetacontact.h"
#include "kopetemetacontactlvi.h"

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
	setMainWidget(mainWidget);
	item = gvi;

	mainWidget->edtDisplayName->setText( item->group()->displayName() );
	mainWidget->chkUseCustomIcons->setChecked( item->group()->useCustomIcon() );

	QString openName = item->group()->icon( KopetePluginDataObject::Open );
	if(openName.isEmpty())
		openName = "folder_green_open"; // Default
	QString closeName = item->group()->icon( KopetePluginDataObject::Closed );
	if(closeName.isEmpty())
		closeName = "folder_green"; // Default
	mainWidget->icnbOpen->setIcon( openName );
    mainWidget->icnbClosed->setIcon( closeName );

	connect( this, SIGNAL(okClicked()), this, SLOT( slotOkClicked() ) );
	connect( mainWidget->chkUseCustomIcons, SIGNAL( toggled( bool ) ),
		this, SLOT( slotUseCustomIconsToggled( bool ) ) );

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

	// only call setIcon if either closed or open is not set to default icon
	if(
		mainWidget->icnbOpen->icon() != "folder_green_open" ||
		mainWidget->icnbClosed->icon() != "folder_green" )
	{
		item->group()->setIcon( mainWidget->icnbOpen->icon(),
			 KopetePluginDataObject::Open );

		item->group()->setIcon( mainWidget->icnbClosed->icon(),
			KopetePluginDataObject::Closed );
	}
}

void KopeteGVIProps::slotUseCustomIconsToggled(bool on)
{
	mainWidget->lblOpen->setEnabled( on );
	mainWidget->icnbOpen->setEnabled( on );
	mainWidget->lblClosed->setEnabled( on );
	mainWidget->icnbClosed->setEnabled( on );
}


// =============================================================================


KopeteMetaLVIProps::KopeteMetaLVIProps(KopeteMetaContactLVI *lvi, QWidget *parent, const char *name)
: KDialogBase(parent, name, true, i18n("Properties of Meta Contact %1").arg(lvi->metaContact()->displayName()), Ok|Cancel, Ok, false)
{
	mainWidget = new KopeteMetaLVIPropsWidget( this, "mainWidget" );
	mainWidget->icnbOffline->setIconSize( KIcon::SizeSmall );
	mainWidget->icnbOnline->setIconSize( KIcon::SizeSmall );
	mainWidget->icnbAway->setIconSize( KIcon::SizeSmall );
	mainWidget->icnbUnknown->setIconSize( KIcon::SizeSmall );
	setMainWidget( mainWidget );
	item = lvi;

	mainWidget->edtDisplayName->setText( item->metaContact()->displayName() );
	mainWidget->chkTrackChildDisplayName->setChecked( item->metaContact()->trackChildNameChanges() );
	mainWidget->chkTrackChildDisplayName->setEnabled( item->metaContact()->contacts().count() == 1 );

	mainWidget->chkUseCustomIcons->setChecked( item->metaContact()->useCustomIcon() );

	QString offlineName = item->metaContact()->icon( KopetePluginDataObject::Offline );
	if(offlineName.isEmpty())
		offlineName = QString::fromLatin1(MC_OFF); // Default

	QString onlineName = item->metaContact()->icon( KopetePluginDataObject::Online );
	if(onlineName.isEmpty())
		onlineName = QString::fromLatin1(MC_ON); // Default

	QString awayName = item->metaContact()->icon( KopetePluginDataObject::Away );
	if(awayName.isEmpty())
		awayName = QString::fromLatin1(MC_AW); // Default

	QString unknownName = item->metaContact()->icon( KopetePluginDataObject::Unknown );
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
		KABC::AddressBook *ab = KABC::StdAddressBook::self();
		KABC::StdAddressBook::setAutomaticSave( false );
		KABC::Addressee a = ab->findByUid( kabcUid );
		mainWidget->edtAddressee->setText( a.realName() );
		mainWidget->btnSelectAddressee->setEnabled( true );
		mainWidget->btnMerge->setEnabled( true );
		mainWidget->edtAddressee->setEnabled( true );
		mainWidget->lblAddressee->setEnabled( true );
		mainWidget->chkHasAddressbookEntry->setChecked( true );
		mExport = new KopeteAddressBookExport( this, item->metaContact() );
	}
	
	connect( this, SIGNAL(okClicked()), this, SLOT( slotOkClicked() ) );
	connect( mainWidget->chkUseCustomIcons, SIGNAL( toggled( bool ) ),
		this, SLOT( slotUseCustomIconsToggled( bool ) ) );
	connect( mainWidget->chkHasAddressbookEntry, SIGNAL( toggled( bool ) ),
		this, SLOT( slotHasAddressbookEntryToggled( bool ) ) );
	connect( mainWidget->btnSelectAddressee, SIGNAL( clicked() ),
		this, SLOT( slotSelectAddresseeClicked() ) );
	connect( mainWidget->btnMerge, SIGNAL( clicked() ),
		this, SLOT( slotMergeClicked() ) );
		
	slotUseCustomIconsToggled( mainWidget->chkUseCustomIcons->isChecked() );
}

KopeteMetaLVIProps::~KopeteMetaLVIProps()
{
}

void KopeteMetaLVIProps::slotOkClicked()
{
	if( mainWidget->edtDisplayName->text() != item->metaContact()->displayName() )
	{
		item->metaContact()->setDisplayName( mainWidget->edtDisplayName->text() );
	}
	item->metaContact()->setTrackChildNameChanges( mainWidget->chkTrackChildDisplayName->isChecked() );

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
			 KopetePluginDataObject::Offline );

		item->metaContact()->setIcon( mainWidget->icnbOnline->icon(),
			KopetePluginDataObject::Online );

		item->metaContact()->setIcon( mainWidget->icnbAway->icon(),
			KopetePluginDataObject::Away );

		item->metaContact()->setIcon( mainWidget->icnbUnknown->icon(),
			KopetePluginDataObject::Unknown );
	}
	// if no kabc link, remove any existing link
	if ( !mainWidget->chkHasAddressbookEntry->isChecked() )
		item->metaContact()->setMetaContactId( QString::null );
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
		mainWidget->btnMerge->setEnabled( false );
}

void KopeteMetaLVIProps::slotSelectAddresseeClicked()
{
	 KABC::Addressee a = KABC::AddresseeDialog::getAddressee(this);
	 if ( a.isEmpty() )
	 {
	 	mainWidget->edtAddressee->setText( QString::null ) ;
		mainWidget->btnMerge->setEnabled( false );
	 }
	 else
	 {
	 	mainWidget->btnMerge->setEnabled( true );
		// set the lineedit to the Addressee's name
		mainWidget->edtAddressee->setText( a.realName() );
		// set/update the MC's addressee uin field
		item->metaContact()->setMetaContactId( a.uid() );
		delete mExport;
		mExport = new KopeteAddressBookExport( this, item->metaContact() );
	 }
}

void KopeteMetaLVIProps::slotMergeClicked()
{
	if ( mExport->showDialog() == QDialog::Accepted )
		mExport->exportData();
}
#include "kopetelviprops.moc"
