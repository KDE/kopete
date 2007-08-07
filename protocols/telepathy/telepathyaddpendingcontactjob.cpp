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
#include <contactaddednotifydialog.h>
#include <kopetemetacontact.h>
#include <kopetecontactlist.h>

// QtTapioca includes
#include <QtTapioca/Contact>

// Local includes
#include "telepathyprotocol.h"
#include "telepathyaccount.h"
#include "telepathycontact.h"

using namespace Kopete::UI;

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
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo;

	Q_ASSERT( !d->pendingContact.isNull() );

	ContactAddedNotifyDialog::HideWidgetOptions hideOptions = ContactAddedNotifyDialog::InfoButton;
	if( d->authorizationOnly )
	{
		hideOptions |= ContactAddedNotifyDialog::AddGroupBox | ContactAddedNotifyDialog::AddCheckBox;
	}

	Kopete::UI::ContactAddedNotifyDialog *dialog = new Kopete::UI::ContactAddedNotifyDialog(d->pendingContact->uri(), d->pendingContact->alias(), d->account, hideOptions);
	connect(dialog, SIGNAL(applyClicked(QString)), this, SLOT(contactDialogDone()));

	dialog->show();
}

void TelepathyAddPendingContactJob::contactDialogDone()
{
	// NOTE: sender() is evil. It kill kittens.
	Kopete::UI::ContactAddedNotifyDialog *dialog = dynamic_cast<Kopete::UI::ContactAddedNotifyDialog *>( sender() );

	if( dialog )
	{
		if( dialog->added() )
		{
			// Get the new metacontact
			Kopete::MetaContact *newMetaContact = dialog->addContact();
	
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
				kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Could not find new Telepathy contact " << contactUri;
				setError( KJob::UserDefinedError );
			}
		}

		if( dialog->authorized() )
		{
			// Authorize new contact
			d->pendingContact->authorize(true);
		}
	}
	else
	{
		setError( KJob::UserDefinedError );
	}

	emitResult();
}

#include "telepathyaddpendingcontactjob.moc"
