/*
    kopetelviprops.cpp

    Kopete Contactlist Properties GUI for Groups and MetaContacts

    Copyright (c) 2002-2003 by Stefan Gehn            <metz AT gehn.net>
	Copyright (c) 2004      by Will Stephenson <lists@stevello.free-online.co.uk>
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

#include <kdebug.h>

#include <klocale.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>

#include <kconfig.h>
#include <kdialogbase.h>
#include <kfiledialog.h>
#include <kicondialog.h>
#include <kabc/addresseedialog.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "kopeteaddrbookexport.h"

#include "kopeteeventpresentation.h"
#include "kopetenotifyevent.h"
#include "kopetegroup.h"
#include "kopetegroupviewitem.h"
#include "kopetemetacontact.h"
#include "kopetenotifyclient.h"
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
	m_dirty = false;
	
	mainWidget->edtDisplayName->setText( item->group()->displayName() );
	mainWidget->chkUseCustomIcons->setChecked( item->group()->useCustomIcon() );

	QString openName = item->group()->icon( KopetePluginDataObject::Open );
	if(openName.isEmpty())
		openName = KOPETE_GROUP_DEFAULT_OPEN_ICON;
	QString closeName = item->group()->icon( KopetePluginDataObject::Closed );
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

void KopeteGVIProps::slotIconChanged()
{
	m_dirty = true;
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
		
		mSound = a.sound();
		mainWidget->btnFromKABC->setEnabled( !( mSound.isIntern() || mSound.url().isEmpty() ) );
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
	connect( mainWidget->btnFromKABC, SIGNAL( clicked() ),
		this, SLOT( slotFromKABCClicked() ) );
    connect( mainWidget->customSound, SIGNAL( openFileDialog( KURLRequester * )),
             SLOT( openSoundDialog( KURLRequester * )));
	slotUseCustomIconsToggled( mainWidget->chkUseCustomIcons->isChecked() );

	populateEventsCombo();
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
	
	// save the widgets' state
	storeCurrentCustoms();
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
		mainWidget->btnFromKABC->setEnabled( false );
	 }
	 else
	 {
		mSound = a.sound();
		mainWidget->btnFromKABC->setEnabled( !( mSound.isIntern() || mSound.url().isEmpty() ) );
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

void KopeteMetaLVIProps::populateEventsCombo()
{
	QString path = "kopete/eventsrc";
	KConfig eventsfile( path, true, false, "data" );
	m_eventList = eventsfile.groupList();
	QStringList contactSpecificEvents; // we are only interested in events that relate to contacts
	QStringList::Iterator it = m_eventList.begin();
	QStringList::Iterator end = m_eventList.end();
	for ( ; it != end; ++it )
	{
		if ( !(*it).startsWith( QString::fromLatin1( "kopete_contact_" ) ) )
			continue;
		contactSpecificEvents.append( *it );
		QMap<QString, QString> entries = eventsfile.entryMap( *it );
		eventsfile.setGroup( *it );
		QString comment = eventsfile.readEntry( "Comment", QString::fromLatin1( "Found nothing!" ) );
		mainWidget->cmbEvents->insertItem( comment );
	}
	m_eventList = contactSpecificEvents;
	slotEventsComboChanged( mainWidget->cmbEvents->currentItem() );
	connect( mainWidget->cmbEvents, SIGNAL( activated( int ) ), this, SLOT( slotEventsComboChanged( int ) ) );
}

void KopeteMetaLVIProps::slotEventsComboChanged( int itemNo )
{
	// if the combo has changed, store the previous state of the widgets
	// record the selected item so we can save it when the widget changes next
	storeCurrentCustoms();
	m_event = m_eventList[ itemNo ];
	// update the widgets for the selected item
	// get the corresponding KopeteNotifyEvent
	KopeteNotifyEvent *evt = item->metaContact()->notifyEvent( m_event );
	// set the widgets accordingly
	resetEventWidgets();
	if ( evt )
	{
		// sound presentation
		KopeteEventPresentation *pres = evt->presentation( KopeteEventPresentation::Sound );
		if ( pres )
		{
			mainWidget->chkCustomSound->setChecked( pres->enabled() );
			mainWidget->customSound->setURL( pres->content() );
			mainWidget->chkSoundSS->setChecked( pres->singleShot() );
		}
		// message presentation
		pres = evt->presentation( KopeteEventPresentation::Message );
		if ( pres )
		{
			mainWidget->chkCustomMsg->setChecked( pres->enabled() );
			mainWidget->customMsg->setText( pres->content() );
			mainWidget->chkMsgSS->setChecked( pres->singleShot() );
		}
		// chat presentation
		pres = evt->presentation( KopeteEventPresentation::Chat );
		if ( pres )
		{
			mainWidget->chkCustomChat->setChecked( pres->enabled() );
			mainWidget->chkChatSS->setChecked( pres->singleShot() );
		}
		mainWidget->chkSuppressCommon->setChecked( evt->suppressCommon() );
	}
	//dumpData();
}


void KopeteMetaLVIProps::dumpData()
{
	KopeteNotifyEvent *evt = item->metaContact()->notifyEvent( m_event );
	if ( evt )
		kdDebug( 14000 ) << k_funcinfo << evt->toString() << endl;
	else 
		kdDebug( 14000 ) << k_funcinfo << " no event exists." << endl;
}

void KopeteMetaLVIProps::resetEventWidgets()
{
	mainWidget->chkCustomSound->setChecked( false );
	mainWidget->customSound->clear();
	mainWidget->chkSoundSS->setChecked( true );
	mainWidget->chkCustomMsg->setChecked( false );
	mainWidget->customMsg->clear();
	mainWidget->chkMsgSS->setChecked( true );
	mainWidget->chkCustomChat->setChecked( false );
	mainWidget->chkChatSS->setChecked( true );
	mainWidget->chkSuppressCommon->setChecked( false );
}

void KopeteMetaLVIProps::storeCurrentCustoms()
{
	if ( !m_event.isNull() )
	{
		KopeteNotifyEvent *evt = item->metaContact()->notifyEvent( m_event );
		if ( !evt )
		{
			evt = new KopeteNotifyEvent( );
			// store the changed event
			item->metaContact()->setNotifyEvent( m_event, evt );
		}
		evt->setSuppressCommon( mainWidget->chkSuppressCommon->isChecked() );
		// set different presentations
		KopeteEventPresentation *eventNotify = 0;
		eventNotify = new KopeteEventPresentation( KopeteEventPresentation::Sound, 
				mainWidget->customSound->url(),
				mainWidget->chkSoundSS->isChecked(),
				mainWidget->chkCustomSound->isChecked() );
		evt->setPresentation( KopeteEventPresentation::Sound, eventNotify );
		// set message attributes
		eventNotify = new KopeteEventPresentation( KopeteEventPresentation::Message,
				mainWidget->customMsg->text(),
				mainWidget->chkMsgSS->isChecked(),
				mainWidget->chkCustomMsg->isChecked() );
		evt->setPresentation( KopeteEventPresentation::Message, eventNotify );
		// set chat attributes
		eventNotify = new KopeteEventPresentation( KopeteEventPresentation::Chat,
				QString::null,
				mainWidget->chkChatSS->isChecked(),
				mainWidget->chkCustomChat->isChecked() );
		evt->setPresentation( KopeteEventPresentation::Chat, eventNotify );
		evt->setSuppressCommon( mainWidget->chkSuppressCommon->isChecked() );
	}
}

void KopeteMetaLVIProps::slotFromKABCClicked()
{
	mainWidget->customSound->setURL( mSound.url() );
}

void KopeteMetaLVIProps::slotOpenSoundDialog( KURLRequester *requester )
{
	// taken from kdelibs/kio/kfile/knotifydialog.cpp
    // only need to init this once
    requester->disconnect( SIGNAL( openFileDialog( KURLRequester * )),
                           this, SLOT( openSoundDialog( KURLRequester * )));

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
