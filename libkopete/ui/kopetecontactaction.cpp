/*
    kopetecontactaction.cpp - KAction for selecting a Kopete::Contact

    Copyright (c) 2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactaction.h"

#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopeteonlinestatus.h"

KopeteContactAction::KopeteContactAction( Kopete::Contact *contact, const QObject *receiver,
	const char *slot, KAction *parent )
: KAction( contact->metaContact()->displayName(), QIconSet( contact->onlineStatus().iconFor( contact ) ), KShortcut(),
	parent, contact->contactId().latin1() )
{
	m_contact = contact;

	connect( this, SIGNAL( activated() ), SLOT( slotContactActionActivated() ) );
	connect( this, SIGNAL( activated( Kopete::Contact * ) ), receiver, slot );
}

KopeteContactAction::~KopeteContactAction()
{
}

void KopeteContactAction::slotContactActionActivated()
{
	emit activated( m_contact );
}

Kopete::Contact * KopeteContactAction::contact() const
{
	return m_contact;
}


#include "kopetecontactaction.moc"

// vim: set noet ts=4 sts=4 sw=4:


