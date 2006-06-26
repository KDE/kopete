 /*
  * jabbergroupmembercontact.cpp  -  Kopete Jabber protocol groupchat contact (member)
  *
  * Copyright (c) 2002-2004 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#ifndef JABBERGROUPMEMBERCONTACT_H
#define JABBERGROUPMEMBERCONTACT_H

#include "jabberbasecontact.h"

namespace Kopete { class MetaContact; }
class JabberGroupChatManager;
class JabberChatSession;

class JabberGroupMemberContact : public JabberBaseContact
{

Q_OBJECT

public:

	JabberGroupMemberContact (const XMPP::RosterItem &rosterItem,
							  JabberAccount *account, Kopete::MetaContact * mc);

	~JabberGroupMemberContact ();

	/**
	 * Create custom context menu items for the contact
	 * FIXME: implement manager version here?
	 */
	QPtrList<KAction> *customContextMenuActions ();

	/**
	 * Return message manager for this instance.
	 */
	Kopete::ChatSession *manager ( Kopete::Contact::CanCreateFlags canCreate = Kopete::Contact::CannotCreate );

	/**
	 * Deal with incoming messages.
	 */
	void handleIncomingMessage ( const XMPP::Message &message );

public slots:

	/**
	 * This is the JabberContact level slot for sending files.
	 *
	 * @param sourceURL The actual KURL of the file you are sending
	 * @param fileName (Optional) An alternate name for the file - what the
	 *                 receiver will see
	 * @param fileSize (Optional) Size of the file being sent. Used when sending
	 *                 a nondeterminate file size (such as over a socket)
	 */
	virtual void sendFile( const KURL &sourceURL = KURL(),
		const QString &fileName = QString::null, uint fileSize = 0L );

private slots:
	/**
	 * Catch a dying message manager
	 */
	void slotChatSessionDeleted ();

private:
	JabberChatSession *mManager;

};

#endif
