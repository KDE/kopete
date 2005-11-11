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

#include "customnotificationprops.h"

#include <knotification.h>
#include <kapplication.h>

#include <QMetaEnum>

CustomNotificationProps::CustomNotificationProps( QWidget *parent, const QPair<QString,QString> &context, const char * name )
	: QObject( parent, name ) , m_item(context)
{
	m_notifyWidget = new CustomNotificationWidget( parent, "notificationWidget" );

	QString path = "kopete/eventsrc";
	KConfig eventsfile( path, true, false, "data" );
	m_eventList = eventsfile.groupList();
	QStringList contactSpecificEvents; // we are only interested in events that relate to contacts
	QRegExp rx("^Event/([^/]*)$");
	m_eventList=m_eventList.filter( rx );
	foreach (QString group , m_eventList )
	{
		eventsfile.setGroup(group);
		if( !eventsfile.readListEntry("Contexts").contains(m_item.first))
			continue;
		rx.indexIn(group);
		contactSpecificEvents.append( rx.cap(1) );
		QMap<QString, QString> entries = eventsfile.entryMap( group );
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
	
	KConfig configfile( QString::fromAscii(kapp->instanceName())+QString::fromAscii( ".eventsrc" ), true, false);
	configfile.setGroup("Event/" + m_event + "/" + m_item.first + "/" + m_item.second);
	
	int presentationEnum = KNotification::staticMetaObject.indexOfEnumerator( "NotifyPresentation" );
	QString presentstring=configfile.readEntry("Action");
	int presentation = KNotification::staticMetaObject.enumerator(presentationEnum).keysToValue( presentstring.latin1() );
	if(presentation == -1 ) presentation = 0;

	// set the widgets accordingly
	resetEventWidgets();
		// sound presentation
			m_notifyWidget->chkCustomSound->setChecked( presentation & KNotification::Sound );
			m_notifyWidget->customSound->setURL( configfile.readEntry("Sound") );
//			m_notifyWidget->chkSoundSS->setChecked( pres->singleShot() );
		// message presentation
			m_notifyWidget->chkCustomMsg->setChecked( presentation & KNotification::PassivePopup );
//			m_notifyWidget->customMsg->setText( pres->content() );
//			m_notifyWidget->chkMsgSS->setChecked( pres->singleShot() );
		// chat presentation
/*		pres = evt->presentation( Kopete::EventPresentation::Chat );
		if ( pres )
		{
			m_notifyWidget->chkCustomChat->setChecked( pres->enabled() );
			m_notifyWidget->chkChatSS->setChecked( pres->singleShot() );
		}
		m_notifyWidget->chkSuppressCommon->setChecked( evt->suppressCommon() ); */
	//dumpData();
}


void CustomNotificationProps::dumpData()
{
/*	Kopete::NotifyEvent *evt = m_item->notifyEvent( m_event );
	if ( evt )
		kdDebug( 14000 ) << k_funcinfo << evt->toString() << endl;
	else 
		kdDebug( 14000 ) << k_funcinfo << " no event exists." << endl;*/
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
		KConfig configfile( QString::fromAscii(kapp->instanceName())+QString::fromAscii( ".eventsrc" ), false, false);
		configfile.setGroup("Event/" + m_event + "/" + m_item.first + "/" + m_item.second);
	
		int presentation=0;
				
	//	evt->setSuppressCommon( m_notifyWidget->chkSuppressCommon->isChecked() );
		// set different presentations
		
		configfile.writeEntry("Sound" , m_notifyWidget->customSound->url());
		//		m_notifyWidget->chkSoundSS->isChecked(),
		if( m_notifyWidget->chkCustomSound->isChecked() )
			presentation |= KNotification::Sound;
		// set message attributes
//				m_notifyWidget->customMsg->text(),
//				m_notifyWidget->chkMsgSS->isChecked(),
		if( m_notifyWidget->chkCustomMsg->isChecked() )
			presentation |= KNotification::PassivePopup;
/*		// set chat attributes
		eventNotify = new Kopete::EventPresentation( Kopete::EventPresentation::Chat,
				QString::null,
				m_notifyWidget->chkChatSS->isChecked(),
				m_notifyWidget->chkCustomChat->isChecked() );
		evt->setPresentation( Kopete::EventPresentation::Chat, eventNotify );
		evt->setSuppressCommon( m_notifyWidget->chkSuppressCommon->isChecked() );*/
		
		int presentationEnum = KNotification::staticMetaObject.indexOfEnumerator( "NotifyPresentation" );
		QString presentstring= KNotification::staticMetaObject.enumerator(presentationEnum).valueToKeys( presentation );
		configfile.writeEntry("Action" , presentstring);
	}
}

CustomNotificationWidget* CustomNotificationProps::widget()
{
	return m_notifyWidget;
}

#include "customnotificationprops.moc"
