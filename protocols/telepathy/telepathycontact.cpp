/*
 * telepathycontact.cpp - Telepathy Kopete Contact.
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
#include "telepathycontact.h"

// Qt includes
#include <QtCore/QPointer>

// KDE includes
#include <kaction.h>
#include <kdebug.h>

// Kopete includes
#include "kopetechatsessionmanager.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"

// QtTapioca includes
#include <QtTapioca/Contact>

// Telepathy includes
#include "telepathyaccount.h"
#include "telepathyprotocol.h"

using namespace QtTapioca;

class TelepathyContact::Private
{
public:
	Private()
	{}

	QPointer<QtTapioca::Contact> internalContact;
};

TelepathyContact::TelepathyContact(TelepathyAccount *account, const QString &contactId, Kopete::MetaContact *parent)
 : Kopete::Contact(account, contactId, parent), d(new Private)
{
}

TelepathyContact::~TelepathyContact()
{
	delete d;
}

QtTapioca::Contact *TelepathyContact::internalContact()
{
	Q_ASSERT( !d->internalContact.isNull() );
	return d->internalContact;
}

void TelepathyContact::setInternalContact(QtTapioca::Contact *internalContact)
{
	if( !d->internalContact.isNull() )
	{
		// Disconnect signals from previous internal contact
		d->internalContact->disconnect();
	}
	d->internalContact = internalContact;

	// Set initial presence
	TelepathyProtocol::protocol()->telepathyStatusToKopete( d->internalContact->getContactInfo()->presence() );

	// Set nickname/alias
	setNickName( d->internalContact->getContactInfo()->alias() );

	// Connect signal/slots
	connect(d->internalContact->getContactInfo(), SIGNAL(presenceUpdated(ContactInfo*, ContactInfo::Presence, QString)), this, SLOT(slotPresenceUpdated(ContactInfo*, ContactInfo::Presence, QString)));
}

bool TelepathyContact::isReachable()
{
	return account()->isConnected();
}

void TelepathyContact::serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData)
{
	// Nothing specific to serialize yet.
}

QList<KAction *> *TelepathyContact::customContextMenuActions()
{
	return 0;
}

Kopete::ChatSession *TelepathyContact::manager(CanCreateFlags canCreate)
{
	return 0;
}

void TelepathyContact::slotPresenceUpdated(ContactInfo *contactInfo, ContactInfo::Presence presence, const QString &presenceMessage)
{
	Kopete::OnlineStatus newStatus = TelepathyProtocol::protocol()->telepathyStatusToKopete(presence);

	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Updating " << contactId() << " presence to " << newStatus.description() << endl;
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "New Status Message for " << contactId() << ": " << presenceMessage << endl;

	setOnlineStatus( newStatus );
	setStatusMessage( Kopete::StatusMessage(presenceMessage) );
}

#include "telepathycontact.moc"
