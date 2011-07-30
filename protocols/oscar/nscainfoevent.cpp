/*
    nscainfoevent.cpp  -  Non Server Contacts Add Info Event

    Copyright (c) 2008 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "nscainfoevent.h"

#include <klocale.h>

#include "contactmanager.h"
#include "oscarutils.h"

NonServerContactsAddInfoEvent::NonServerContactsAddInfoEvent( ContactManager* listManager, bool icq, QObject* parent )
: Kopete::InfoEvent(parent), mContactCount(0)
{
	setShowOnSend( true );
	setTitle( i18n("Adding contacts") );

	if ( icq )
		setText( i18n( "Adding contacts to ICQ server list.") );
	else
		setText( i18n( "Adding contacts to AIM server list.") );

	connect( listManager, SIGNAL(contactAdded(OContact)),
	         this, SLOT(ssiContactAdded(OContact)) );
}

void NonServerContactsAddInfoEvent::updateText()
{
	int addedContacts = mContactCount - mRemainingContacts.count();
	setAdditionalText( i18nc( "%1 out of %2 contacts have been added", "%1 out of %2 added.", addedContacts, mContactCount ) );
}

void NonServerContactsAddInfoEvent::addContact( const QString& contact )
{
	if ( mRemainingContacts.contains( contact ) )
		return;

	mContactCount++;
	mRemainingContacts.insert( contact );
	updateText();
}

void NonServerContactsAddInfoEvent::ssiContactAdded( const OContact& item )
{
	QString normalizedName = Oscar::normalize( item.name() );
	mRemainingContacts.remove( normalizedName );
	updateText();
}

#include "nscainfoevent.moc"
