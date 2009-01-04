/*
 * telepathyaddcontactpage.cpp - Telepathy Add Contact Page
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 *
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#include "telepathyaddcontactpage.h"
#include "ui_telepathyaddcontactpage.h"

// KDE includes
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>

// QtTapioca includes
#include <QtTapioca/Contact>

// Local includes
#include "telepathyaccount.h"
#include "telepathycontact.h"
#include "telepathycontactmanager.h"
#include "telepathyprotocol.h"

class TelepathyAddContactPage::Private
{
public:
	Ui::TelepathyAddContactPage mainUi;
};

TelepathyAddContactPage::TelepathyAddContactPage(QWidget *parent)
 : AddContactPage(parent), d(new Private)
{
	d->mainUi.setupUi(this);
	d->mainUi.textUserId->setFocus();
}

TelepathyAddContactPage::~TelepathyAddContactPage()
{
	delete d;
}

bool TelepathyAddContactPage::validateData()
{
	// Nothing to valid for now
	return true;
}

bool TelepathyAddContactPage::apply(Kopete::Account *account, Kopete::MetaContact *parentMetaContact)
{
	if( !account->isConnected() )
	{
		KMessageBox::error( this, i18n("You must be connected to add a contact."), i18n("Telepathy plugin") );
		return false;
	}
	TelepathyAccount *tAccount = static_cast<TelepathyAccount*>(account);
	
	// Get new id.
	QString newId = d->mainUi.textUserId->text();

	// Add contact into Telepathy
	QtTapioca::Contact *internalContact = tAccount->contactManager()->addContact( newId );
	if( internalContact )
	{
		// Now add contact in Kopete
		if( tAccount->addContact( newId, parentMetaContact ) )
		{
			// Add the internal contact to the Kopete contact
			TelepathyContact *newContact = static_cast<TelepathyContact*>( tAccount->contacts()[newId] );
			newContact->setInternalContact( internalContact );
			// Subsribe to contact status.
			internalContact->subscribe(true);

			return true;
		}
	}


	return false;
}

#include "telepathyaddcontactpage.moc"
