 /*
  * jabbercontact.h  -  Base class for the Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2003 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
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

#ifndef JABBERCONTACT_H
#define JABBERCONTACT_H

#include "jabberaccount.h"
#include "jabberresource.h"
#include "kopetecontact.h"


class KAction;
class KPopupMenu;
class KSelectAction;

class dlgJabberRename;
class dlgJabberVCard;
class JabberProtocol;
class JabberResource;
class JabberMessage;
class JabberMessageManager;
class KopeteMessage;
class KopeteMessageManager;
class KopeteMetaContact;
class KopeteGroup;

class XMPP::Jid;
class XMPP::Message;

class JabberContact:public KopeteContact
{
	Q_OBJECT

	 friend class JabberAccount;	/* Friends can touch each other's private parts. */

public:
	JabberContact (const XMPP::RosterItem &rosterItem,
				   JabberAccount *account, KopeteMetaContact * mc);

	/********************************************************************
	 *
	 * KopeteContact reimplementation start
	 *
	 ********************************************************************/

	/**
	 * Return the protocol instance associated with this contact
	 */
	JabberProtocol *protocol ();

	/**
	 * Return the account instance associated with this contact
	 */
	JabberAccount *account ();

	/**
	 * Return if the contact is reachable (this is true if the account
	 * is online)
	 */
	virtual bool isReachable ();

	/**
	 * Create custom context menu items for the contact
	 * FIXME: implement manager version here?
	 */
	virtual QPtrList<KAction> *customContextMenuActions ();

	/**
	 * Serialize contact
	 */
	virtual void serialize (QMap < QString, QString > &serializedData, QMap < QString, QString > &addressBookData);

	/**
	 * Create a message manager for this contact
	 */
	virtual KopeteMessageManager *manager ( bool canCreate = false );

	JabberMessageManager *manager ( const QString &resource, bool canCreate = false );

	/**
	 * Start a rename request.
	 */
	virtual void rename ( const QString &newName );

	/**
	 * Update contact if a roster item has been
	 * received for it. (used during login)
	 */
	void updateContact ( const XMPP::RosterItem &item );

	/**
	 * Deal with an incoming message for this contact.
	 */
	void handleIncomingMessage ( const XMPP::Message &message );

	/**
	 * Re-evaluate online status. Gets called
	 * whenever a resource is added, removed, or
	 * changed in the resource pool.
	 */
	void reevaluateStatus ();

public slots:

	/**
	 * Remove this contact from the roster
	 */
	virtual void slotDeleteContact ();

	/**
	 * Retrieve a vCard for the contact
	 */
	virtual void slotUserInfo ();

	/**
	 * Sync Groups with server
	 */
	virtual void syncGroups ();

	/**
	* Select a new resource for the contact
	*/
	void slotSelectResource ();

private slots:
	/**
	 * Send type="subscribed" to contact
	 */
	void slotSendAuth ();

	/**
	 * Send type="subscribe" to contact
	 */
	void slotRequestAuth ();

	/**
	 * Send type="unsubscribed" to contact
	 */
	void slotRemoveAuth ();

	/**
	 * Change this contact's status
	 */
	void slotStatusOnline ();
	void slotStatusChatty ();
	void slotStatusAway ();
	void slotStatusXA ();
	void slotStatusDND ();
	void slotStatusInvisible ();

	void slotMessageManagerDeleted ( QObject *sender );

private:

	/**
	 * Sends subscription messages.
	 */
	void sendSubscription (const QString& subType);

	/**
	 * Sends a presence packet to this contact
	 */
	void sendPresence ( const XMPP::Status status );

	/**
	 * Construct best address out of
	 * eventually preselected resource
	 * (due to subscription) and best
	 * available resource.
	 */
	XMPP::Jid bestAddress ();

	/**
	 * This will simply cache all
	 * relevant data for this contact.
	 */
	XMPP::RosterItem mRosterItem;

	QPtrList<JabberMessageManager> mManagers;

	dlgJabberVCard *dlgVCard;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:
