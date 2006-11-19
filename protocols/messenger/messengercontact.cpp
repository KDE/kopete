/*
 * messengercontact.cpp - Windows Live Messenger Kopete Contact.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
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
#include "messengercontact.h"

// KDE includes
#include <kaction.h>

// Kopete includes
#include "kopetechatsessionmanager.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"

// Messenger includes
#include "messengeraccount.h"

MessengerContact::MessengerContact(MessengerAccount *account, const QString &contactId, Kopete::MetaContact *parent)
 : Kopete::Contact(account, contactId, parent)
{
}

bool MessengerContact::isReachable()
{
	return false;
}

void MessengerContact::serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData)
{
	
}

QList<KAction *> *MessengerContact::customContextMenuActions()
{
	return 0;
}

Kopete::ChatSession *MessengerContact::manager(CanCreateFlags canCreate)
{
	return 0;
}

#include "messengercontact.moc"
