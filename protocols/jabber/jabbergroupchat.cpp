/***************************************************************************
                          jabbergroupchat.cpp  -  description
                             -------------------
    begin                : Fre Feb 28 2003
    copyright            : (C) 2003 by Till Gerken (till@tantalo.net)
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

#include <psi/types.h>
#include "jabbergroupchat.h"

JabberGroupChat::JabberGroupChat(Jabber::Jid jid, QStringList groups,
								 JabberProtocol *p, KopeteMetaContact *mc, QString identity)
								 : JabberContact( jid.userHost(), jid.userHost(), groups, p, mc, identity )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberGroupChat] Joined room " << jid.user() << " at " << jid.host() << endl;
	
	room = jid;

}

JabberGroupChat::~JabberGroupChat()
{
}                                

void JabberGroupChat::slotMessageManagerDeleted()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberGroupChat] slotMessageManagerDeleted(), leaving room " << room.user() << " at server " << room.host() << endl;

	// the message manager has been deleted, leave the chat room
	JabberProtocol::protocol()->jabberClient->groupChatLeave(room.host(), room.user());

	// pass the slot on to the base class
	JabberContact::slotMessageManagerDeleted();

}

void JabberGroupChat::updatePresence(const Jabber::Jid &jid, const Jabber::Status &status)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberGroupChat] updatePresence() called for JID " << jid.full() << endl;

}
