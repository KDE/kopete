/*
    jabbergroupchatmanager.h - Jabber Message Manager for group chats

    Copyright (c) 2004 by Till Gerken            <till@tantalo.net>

    Kopete    (c) 2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef JABBERGROUPCHATMANAGER_H
#define JABBERGROUPCHATMANAGER_H

#include "kopetemessagemanager.h"
#include "xmpp.h"

class JabberProtocol;
class JabberAccount;
class JabberBaseContact;
class KopeteMessage;
class QString;

/**
 * @author Till Gerken
 */
class JabberGroupChatManager : public KopeteMessageManager
{
	Q_OBJECT

public:
	JabberGroupChatManager ( JabberProtocol *protocol, const JabberBaseContact *user,
							 KopeteContactPtrList others, XMPP::Jid roomJid, const char *name = 0 );

	/**
	 * @brief Get the local user in the session
	 * @return the local user in the session, same as account()->myself()
	 */
	const JabberBaseContact *user () const;

	/**
	 * @brief get the account
	 * @return the account
	 */
	JabberAccount *account() const ;

	/**
	 * Re-generate the display name
	 */
	void updateDisplayName ();

private slots:
	void slotMessageSent ( KopeteMessage &message, KopeteMessageManager *kmm );

private:
	XMPP::Jid mRoomJid;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

