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
	JabberContact (QString userid, QString name, QStringList groups,
				   JabberAccount * protocol, KopeteMetaContact * mc);

	/********************************************************************
	 *
	 * KopeteContact reimplementation start
	 *
	 ********************************************************************/

	/**
	 * Return the user ID
	 */
	QString userId () const;

	/*
	 * Return the currently used resource for this contact
	 */
	QString resource () const;

	/**
	 * Return the reason why we are away
	 */
	QString reason () const;

	/**
	 * Return if the contact is reachable (this is true if the account
	 * is online)
	 */
	virtual bool isReachable ();

	/**
	 * Create custom context menu items for the contact
	 */
	virtual QPtrList<KAction> *customContextMenuActions ();

	/**
	 * Determine the currently best resource for the contact
	 */
	JabberResource *bestResource ();

	/**
	 * Serialize contact
	 */
	virtual void serialize (QMap < QString, QString > &serializedData, QMap < QString, QString > &addressBookData);

	virtual KopeteMessageManager *manager (bool canCreate = false);

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
	 * Slots called when a certain resource
	 * appears or disappears for the contact
	 */
	void slotResourceAvailable (const XMPP::Jid & jid, const XMPP::Resource & resource);

	/**
	 * Remove a resource from the contact
	 */
	void slotResourceUnavailable (const XMPP::Jid & jid, const XMPP::Resource & resource);

	/**
	* Select a new resource for the contact
	*/
	void slotSelectResource ();

	/**
	 * Update contact to new roster data
	 */
	void slotUpdateContact (const XMPP::RosterItem & item);

	/**
	 * Update contact to a new status
	 */
	void slotUpdatePresence (const KopeteOnlineStatus & newStatus, const QString & reason);

	/**
	 * Received a message for this contact
	 */
	void slotReceivedMessage (const XMPP::Message & message);

	/**
	 * Sync Groups with server
	 */
	virtual void syncGroups ();

	/**
	 * Catch the rename dialog's results
	 */
	void slotDisplayNameChanged (const QString &oldName, const QString &newName);

protected slots:

	/**
	 * Catch a dying message manager
	 */
	virtual void slotMessageManagerDeleted ();

private slots:
	/**
	 * Edit a vCard for the contact.
	 */
	void slotEditVCard ();

	/**
	 * Save this contact's vCard.
	 */
	void slotSaveVCard (QDomElement &);

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

	/**
	 * Send a message to somebody
	 */
	void slotSendMessage (KopeteMessage & message);

private:

	/**
	 * Convert KopeteMessage to XMPP::Message
	 */
	void km2jm (const KopeteMessage & km, XMPP::Message & jm);

	/**
	 * Sends subscription messages.
	 */
	void sendSubscription (const QString& subType);

	/**
	 * The metacontact that this contact belongs
	 * to.
	 */
	KopeteMetaContact *parentMetaContact;

	/**
	 * Currently active resource for this contact.
	 */
	JabberResource *activeResource;

	/**
	 * Default resource for this contact (usually empty)
	 */
	JabberResource *defaultResource;

	/**
	 * This will simply cache all
	 * relevant data for this contact.
	 */
	XMPP::RosterItem rosterItem;

	/**
	 * List of available resources for the
	 * contact.
	 */
	QPtrList < JabberResource > resources;

	/**
	 * Current away reason and -type
	 */
	QString awayReason;

	/**
	 * Do we specifically send to a
	 * certain resource or do we use
	 * autoselection?
	 */
	bool resourceOverride, mEditingVCard;

	bool mIsNetworkPush;

	/**
	 * Message manager in use to display a message
	 */
	KopeteMessageManager *messageManager;

	KopeteMessageManager *m_manager;

	dlgJabberVCard *dlgVCard;

	//Actions
	KAction *actionSendAuth;
	KAction *actionRequestAuth;
	KAction *actionRemoveAuth;
	KActionMenu *actionSetAvailability;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
