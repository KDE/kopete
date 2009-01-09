/*
   telepathyaddpendingcontactjob.cpp - Telepathy Add Pending Contact Job

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This program is free software; you can redistribute it and/or modify  *
   * it under the terms of the GNU General Public License as published by  *
   * the Free Software Foundation; either version 2 of the License, or     *
   * (at your option) any later version.                                   *
   *                                                                       *
   *************************************************************************
*/
#include "telepathyaddpendingcontactjob.h"

// Qt includes
#include <QtCore/QPointer>

// KDE includes
#include <kdebug.h>

// Kopete includes
#include <kopeteaddedinfoevent.h>
#include <kopetemetacontact.h>
#include <kopetecontactlist.h>

// QtTapioca includes
#include <QtTapioca/Contact>

// Local includes
#include "telepathyprotocol.h"
#include "telepathyaccount.h"
#include "telepathycontact.h"

class TelepathyAddPendingContactJob::Private
{
public:
	Private()
	 : authorizationOnly(false)
	{}

	QPointer<QtTapioca::Contact> pendingContact;
	QPointer<TelepathyAccount> account;
	bool authorizationOnly;
};

TelepathyAddPendingContactJob::TelepathyAddPendingContactJob(TelepathyAccount *parent)
 : KJob(parent), d(new Private)
{
	d->account = parent;
}

TelepathyAddPendingContactJob::~TelepathyAddPendingContactJob()
{
	delete d;
}

void TelepathyAddPendingContactJob::setPendingContact(QtTapioca::Contact *contact)
{
	d->pendingContact = contact;
}

void TelepathyAddPendingContactJob::setAuthorizeOnly(bool value)
{
	d->authorizationOnly = value;
}

void TelepathyAddPendingContactJob::start()
{
	kDebug(TELEPATHY_DEBUG_AREA) ;

	Q_ASSERT( !d->pendingContact.isNull() );

	Kopete::AddedInfoEvent::ShowActionOptions actions = Kopete::AddedInfoEvent::AuthorizeAction;
	if( !d->authorizationOnly )
	{
		actions |= Kopete::AddedInfoEvent::AddAction;
	}

	Kopete::AddedInfoEvent* event = new Kopete::AddedInfoEvent( d->pendingContact->uri(), d->account );
	QObject::connect( event, SIGNAL(actionActivated(uint)),
	                  this, SLOT(slotAddedInfoEventActionActivated(uint)) );
	QObject::connect( event, SIGNAL(eventClosed(Kopete::InfoEvent*)),
	                  this, SLOT(slotAddedInfoEventClosed()) );

	event->showActions( actions );
	event->setContactNickname( d->pendingContact->alias() );
	event->sendEvent();
}

void TelepathyAddPendingContactJob::slotAddedInfoEventActionActivated( uint actionId )
{
	const Kopete::AddedInfoEvent *event = dynamic_cast<const Kopete::AddedInfoEvent *>(sender());
	if( !event )
	{
		setError( KJob::UserDefinedError );
		return;
	}

	if ( actionId == Kopete::AddedInfoEvent::AddContactAction )
	{
		// Get the new metacontact
		Kopete::MetaContact *newMetaContact = event->addContact();

		// Set internal contact pointer to new contact
		QString contactUri = d->pendingContact->uri();
		TelepathyContact *newContact = static_cast<TelepathyContact*>( d->account->contacts()[contactUri] );
		if( newContact )
		{
			newContact->setInternalContact( d->pendingContact );
			// Subscribe to contact presence.
			d->pendingContact->subscribe(true);

			// Add to contact list
			Kopete::ContactList::self()->addMetaContact( newMetaContact );
		}
		else
		{
			kDebug(TELEPATHY_DEBUG_AREA) << "Could not find new Telepathy contact " << contactUri;
			setError( KJob::UserDefinedError );
		}
	}
	else if ( actionId == Kopete::AddedInfoEvent::AuthorizeAction )
	{
		// Authorize new contact
		d->pendingContact->authorize(true);
	}
}

void TelepathyAddPendingContactJob::slotAddedInfoEventClosed()
{
	emitResult();
}

#include "telepathyaddpendingcontactjob.moc"
