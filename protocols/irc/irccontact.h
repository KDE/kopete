/***************************************************************************
                          irccontact.h  -  description
                             -------------------
    begin                : Thu Feb 20 2003
    copyright            : (C) 2003 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IRCCONTACT_H
#define IRCCONTACT_H

#include "kirc.h"
#include "kopetecontact.h"

class KopeteMessageManager;
class KopeteMetaContact;
class IRCIdentity;
class KopeteMessage;

class IRCContact : public KopeteContact
{
	public:
		IRCContact(IRCIdentity *identity, KopeteMetaContact *metac);

		// Checks a message for server commands
		bool processMessage( const KopeteMessage & );

	protected:
		KopeteMetaContact *mMetaContact;
		KIRC *mEngine;
		KopeteMessageManager *mMsgManager;
		IRCIdentity *mIdentity;
};

#endif
