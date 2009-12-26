 /*
  * jabbercontact.cpp  -  Kopete Jabber protocol groupchat contact
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

#ifndef JABBERGROUPCONTACT_H
#define JABBERGROUPCONTACT_H

#include "jabberbasecontact.h"

#include <QList>

namespace Kopete { class MetaContact; }
class JabberGroupChatManager;

class JabberGroupContact : public JabberBaseContact
{

Q_OBJECT

public:

	JabberGroupContact (const XMPP::RosterItem &rosterItem,
						JabberAccount *account, Kopete::MetaContact * mc);

	~JabberGroupContact ();

	/**
	 * Create custom context menu items for the contact
	 * FIXME: implement manager version here?
	 */
	QList<KAction*> *customContextMenuActions ();
	using JabberBaseContact::customContextMenuActions;

	/**
	 * Deal with an incoming message for this contact.
	 */
	void handleIncomingMessage ( const XMPP::Message &message );

	/**
	 * Add a contact to this room.
	 */
	JabberBaseContact *addSubContact ( const XMPP::RosterItem &rosterItem, bool addToManager = true );

	/**
	 * Remove a contact from this room.
	 */
	void removeSubContact ( const XMPP::RosterItem &rosterItem );

	Kopete::ChatSession *manager ( Kopete::Contact::CanCreateFlags canCreate = Kopete::Contact::CannotCreate );

public slots:

	/**
	 * This is the JabberContact level slot for sending files.
	 *
	 * @param sourceURL The actual KUrl of the file you are sending
	 * @param fileName (Optional) An alternate name for the file - what the
	 *                 receiver will see
	 * @param fileSize (Optional) Size of the file being sent. Used when sending
	 *                 a nondeterminate file size (such as over a socket)
	 */
	virtual void sendFile( const KUrl &sourceURL = KUrl(),
		const QString &fileName = QString(), uint fileSize = 0L );

private slots:

	/**
	 * Catch a dying message manager and leave the room.
	 */
	void slotChatSessionDeleted ();
	
	/**
	 * When our own status change, we need to manually send the presence.
	 */
	void slotStatusChanged();
	
	/**
	 * ask the user to change the nick, and change it
	 */
	void slotChangeNick();
	
	/**
	 * a subcontact has been destroyed   (may happen when closing kopete)
	 */
	void slotSubContactDestroyed(Kopete::Contact*);

private:

	QList<Kopete::Contact*> mContactList;
	QList<Kopete::MetaContact*> mMetaContactList;

	JabberGroupChatManager *mManager;
	JabberBaseContact *mSelfContact;
	QString mNick;
	bool mLeaveGroupChat;
};

#endif
