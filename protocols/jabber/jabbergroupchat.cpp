
/***************************************************************************
                          jabbergroupchat.cpp  -  description
                             -------------------
    begin                : Fre Feb 28 2003
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
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

#include <kdebug.h>

#include "jabbergroupchat.h"

JabberGroupChat::JabberGroupChat (XMPP::Jid jid, QStringList groups, JabberAccount * p, KopeteMetaContact * mc):JabberContact (jid.userHost (), jid.userHost (), groups, p, mc)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Joined room " << jid.user () << " at " << jid.host () << endl;

	room = jid;

}

JabberGroupChat::~JabberGroupChat ()
{
}

void JabberGroupChat::slotMessageManagerDeleted ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Leaving room " << room.user () << " at server " << room.host () << endl;

	// the message manager has been deleted, leave the chat room
	static_cast<JabberAccount *>(account())->client()->groupChatLeave (room.host (), room.user ());

	// pass the slot on to the base class
	JabberContact::slotMessageManagerDeleted ();

}

void JabberGroupChat::updatePresence (const XMPP::Jid & jid, const XMPP::Status & /* status */ )
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "JID " << jid.full () << endl;

}
