/*
    customnotificationprops.cpp

    Kopete Contactlist Custom Notifications GUI for Groups and MetaContacts

    Contains UI controller logic for managing custom notifications

    Copyright (c) 2004 Will Stephenson <lists@stevello.free-online.co.uk>
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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kurlrequester.h>

#include "customnotifications.h"
#include "kopeteeventpresentation.h"
#include "kopetenotifyevent.h"
#include "kopetenotifydataobject.h"

#include "customnotificationprops.h"

CustomNotificationProps::CustomNotificationProps( QWidget *parent, Kopete::NotifyDataObject* item, const char * name )
: QObject( parent, name )
{
	m_notifyWidget = new CustomNotificationWidget( parent, "notificationWidget" );

	m_item = item;
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
		m_notifyWidget->cmbEvents->insertItem( comment );
	}
	m_eventList = contactSpecificEvents;
	slotEventsComboChanged( m_notifyWidget->cmbEvents->currentItem() );
	// we have to do this after adding items
	connect( m_notifyWidget->cmbEvents, SIGNAL( activated( int ) ), this, SLOT( slotEventsComboChanged( int ) ) );
}

void CustomNotificationProps::slotEventsComboChanged( int itemNo )
{
	// if the combo has changed, store the previous state of the widgets
	// record the selected item so we can save it when the widget changes next
	storeCurrentCustoms();
	m_event = m_eventList[ itemNo ];
	// update the widgets for the selected item
	// get the corresponding Kopete::NotifyEvent
	Kopete::NotifyEvent *evt = m_item->notifyEvent( m_event );
	// set the widgets accordingly
	resetEventWidgets();
	if ( evt )
	{
		// sound presentation
		Kopete::EventPresentation *pres = evt->presentation( Kopete::EventPresentation::Sound );
		if ( pres )
		{
			m_notifyWidget->chkCustomSound->setChecked( pres->enabled() );
			m_notifyWidget->customSound->setURL( pres->content() );
			m_notifyWidget->chkSoundSS->setChecked( pres->singleShot() );
		}
		// message presentation
		pres = evt->presentation( Kopete::EventPresentation::Message );
		if ( pres )
		{
			m_notifyWidget->chkCustomMsg->setChecked( pres->enabled() );
			m_notifyWidget->customMsg->setText( pres->content() );
			m_notifyWidget->chkMsgSS->setChecked( pres->singleShot() );
		}
		// chat presentation
		pres = evt->presentation( Kopete::EventPresentation::Chat );
		if ( pres )
		{
			m_notifyWidget->chkCustomChat->setChecked( pres->enabled() );
			m_notifyWidget->chkChatSS->setChecked( pres->singleShot() );
		}
		m_notifyWidget->chkSuppressCommon->setChecked( evt->suppressCommon() );
	}
	//dumpData();
}


void CustomNotificationProps::dumpData()
{
	Kopete::NotifyEvent *evt = m_item->notifyEvent( m_event );
	if ( evt )
		kdDebug( 14000 ) << k_funcinfo << evt->toString() << endl;
	else 
		kdDebug( 14000 ) << k_funcinfo << " no event exists." << endl;
}

void CustomNotificationProps::resetEventWidgets()
{
	m_notifyWidget->chkCustomSound->setChecked( false );
	m_notifyWidget->customSound->clear();
	m_notifyWidget->chkSoundSS->setChecked( true );
	m_notifyWidget->chkCustomMsg->setChecked( false );
	m_notifyWidget->customMsg->clear();
	m_notifyWidget->chkMsgSS->setChecked( true );
	m_notifyWidget->chkCustomChat->setChecked( false );
	m_notifyWidget->chkChatSS->setChecked( true );
	m_notifyWidget->chkSuppressCommon->setChecked( false );
}

void CustomNotificationProps::storeCurrentCustoms()
{
	if ( !m_event.isNull() )
	{
		Kopete::NotifyEvent *evt = m_item->notifyEvent( m_event );
		if ( !evt )
		{
			evt = new Kopete::NotifyEvent( );
			// store the changed event
			m_item->setNotifyEvent( m_event, evt );
		}
		evt->setSuppressCommon( m_notifyWidget->chkSuppressCommon->isChecked() );
		// set different presentations
		Kopete::EventPresentation *eventNotify = 0;
		eventNotify = new Kopete::EventPresentation( Kopete::EventPresentation::Sound, 
				m_notifyWidget->customSound->url(),
				m_notifyWidget->chkSoundSS->isChecked(),
				m_notifyWidget->chkCustomSound->isChecked() );
		evt->setPresentation( Kopete::EventPresentation::Sound, eventNotify );
		// set message attributes
		eventNotify = new Kopete::EventPresentation( Kopete::EventPresentation::Message,
				m_notifyWidget->customMsg->text(),
				m_notifyWidget->chkMsgSS->isChecked(),
				m_notifyWidget->chkCustomMsg->isChecked() );
		evt->setPresentation( Kopete::EventPresentation::Message, eventNotify );
		// set chat attributes
		eventNotify = new Kopete::EventPresentation( Kopete::EventPresentation::Chat,
				QString::null,
				m_notifyWidget->chkChatSS->isChecked(),
				m_notifyWidget->chkCustomChat->isChecked() );
		evt->setPresentation( Kopete::EventPresentation::Chat, eventNotify );
		evt->setSuppressCommon( m_notifyWidget->chkSuppressCommon->isChecked() );
	}
}

CustomNotificationWidget* CustomNotificationProps::widget()
{
	return m_notifyWidget;
}

#include "customnotificationprops.moc"
