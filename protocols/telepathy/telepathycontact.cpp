/*
 * telepathycontact.cpp - Telepathy Kopete Contact.
 *
 * Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
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

// KDE includes
#include <kaction.h>

// Kopete includes
#include "kopetechatsessionmanager.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"

// Telepathy includes
#include "telepathyaccount.h"

TelepathyContact::TelepathyContact(TelepathyAccount *account, const QString &contactId, Kopete::MetaContact *parent)
 : Kopete::Contact(account, contactId, parent)
{
}

bool TelepathyContact::isReachable()
{
	return false;
}

void TelepathyContact::serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData)
{
	
}

QList<KAction *> *TelepathyContact::customContextMenuActions()
{
	return 0;
}

Kopete::ChatSession *TelepathyContact::manager(CanCreateFlags canCreate)
{
	return 0;
}

#include "telepathycontact.moc"
