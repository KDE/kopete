/***************************************************************************
                          jabberaccount.cpp  -  description
                             -------------------
    begin                : Sat Mär 8 2003
    copyright            : (C) 2003 by Kopete developers
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>
#include <qstringlist.h>

#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetemetacontact.h"

#include "jid.h"

#include "jabbercontact.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"

JabberAccount::JabberAccount(KopeteProtocol *parent, const QString& accountID, const char *name)
							: KopeteAccount(parent, accountID, name)
{

	/*
	 * create a new contact for myself(), using accountID for the actual user ID,
	 * the display name and the identity ID, metacontact is left empty
	 */
	myContact = new JabberContact(accountID, accountID, QStringList(), static_cast<JabberProtocol*>(parent), 0L, accountID);

	client = 0L;

}

JabberAccount::~JabberAccount()
{

	delete client;
	delete myContact;

}

KopeteContact *JabberAccount::myself() const
{

	return myContact;

}

void JabberAccount::setAway(bool)
{

}

KActionMenu *JabberAccount::actionMenu()
{

}

bool JabberAccount::addContactToMetaContact( const QString &contactId, const QString &displayName,
		 KopeteMetaContact *parentContact )
{

}