/*
    kopetecontactaction.cpp - KAction for selecting a KopeteContact

    Copyright (c) 2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactaction.h"

#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"

KopeteContactAction::KopeteContactAction( KopeteContact *contact, const QObject *receiver,
	const char *slot, KAction *parent )
: KAction( contact->metaContact()->displayName(), contact->onlineStatus().genericIcon(), KShortcut(),
	parent, contact->contactId().latin1() )
{
	m_contact = contact;

	connect( this, SIGNAL( activated() ), SLOT( slotContactActionActivated() ) );
	connect( this, SIGNAL( activated( KopeteContact * ) ), receiver, slot );
}

KopeteContactAction::~KopeteContactAction()
{
}

void KopeteContactAction::slotContactActionActivated()
{
	emit activated( m_contact );
}

KopeteContact * KopeteContactAction::contact() const
{
	return m_contact;
}


#include "kopetecontactaction.moc"

// vim: set noet ts=4 sts=4 sw=4:


