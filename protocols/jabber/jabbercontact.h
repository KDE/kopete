 /*
    jabbercontact.h  -  Base class for the Kopete Jabber protocol contact

    Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
    Copyright (c) 2002 by Till Gerken <till@tantalo.net>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef JABBERCONTACT_H
#define JABBERCONTACT_H

#include "jabberprotocol.h"
#include "jabberresource.h"
#include "kopetecontact.h"


class KAction;
class KListAction;
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
class KopeteGroupList;

class Jabber::Jid;
class Jabber::Message;

enum JabberProtocol::Presence;

class JabberContact : public KopeteContact
{
	Q_OBJECT

	friend class JabberProtocol; /* Friends can touch each other's private parts. */

public:
	JabberContact( QString userid, QString name, QStringList groups,
			JabberProtocol *protocol, KopeteMetaContact *mc, QString identity);

	~JabberContact();

	/********************************************************************
	 *
	 * KopeteContact reimplementation start
	 *
	 ********************************************************************/

	/**
	 * Add/Remove user to/from a group
	 */
	virtual void addToGroup( KopeteGroup * );
	virtual void removeFromGroup( KopeteGroup * );
	virtual void moveToGroup( KopeteGroup * , KopeteGroup * );

	/**
	 * Return contact's status
	 */
	virtual ContactStatus status() const;

	/**
	 * Return contact's status in textual form
	 */
	virtual QString statusText() const;

	/**
	 * Return contact's status as icon name
	 */
	virtual QString statusIcon() const;

	/**
	 * Return importance of contact.
	 * The importance is used for sorting and determined based
	 * on the contact's current status. See ICQ for reference values.
	 */
	virtual int importance() const;

	/**
	 * Return the identity ID
	 */
	virtual QString identityId() const;

	/**
	 * Return the user ID
	 */
	QString userId() const;

	/*
	 * Return the currently used resource for this contact
	 */
	QString resource() const;

	/*
	 * Return the group this contact resides in
	 */
	virtual KopeteGroupList groups() const;

	/**
	 * Return the reason why we are away
	 */
	QString reason() const;

	/**
	 * Return if we are reachable (defaults to
	 * true because we can send on- and offline
	 */
	virtual bool isReachable();

	/**
	 * Create custom context menu items for the contact
	 */
	virtual KActionCollection *customContextMenuActions();

	/**
	 * Determine the currently best resource for the contact
	 */
	JabberResource *bestResource();

	/*
	 * Contact ID and associated data
	 */
	virtual QString data() const;

	/**
	 * Serialize contact
	 */
	virtual void serialize( QMap<QString, QString> &serializedData,
		QMap<QString, QString> &addressBookData );

	virtual KopeteMessageManager *manager( bool canCreate = false );

public slots:
	/**
	 * Remove this contact from the roster
	 */
	virtual void slotDeleteContact();

	/**
	 * Retrieve a vCard for the contact
	 */
	virtual void slotUserInfo();

	/**
	 * Slots called when a certain resource
	 * appears or disappears for the contact
	 */
	void slotResourceAvailable(const Jabber::Jid &jid, const Jabber::Resource &resource);

	/**
	 * Remove a resource from the contact
	 */
	void slotResourceUnavailable(const Jabber::Jid &jid, const Jabber::Resource &resource);

	/**
	* Select a new resource for the contact
	*/
	void slotSelectResource();

	/**
	 * vCard received from server for this contact
	 */
	void slotGotVCard();

	/**
	 * Update contact to new roster data
	 */
	void slotUpdateContact(const Jabber::RosterItem &item);

	/**
	 * Update contact to a new status
	 */
	void slotUpdatePresence(const JabberProtocol::Presence newStatus, const QString &reason);

private slots:
	/**
	 * Delete this contact instance
	 */
	void slotDeleteMySelf(bool);

	/**
	 * Display a rename dialog
	 */
	void slotRenameContact();

	/**
	 * Catch the rename dialog's results
	 */
	void slotDoRenameContact(const QString &);

	/**
	 * Edit a vCard for the contact.
	 */
	void slotEditVCard();

	/**
	 * Save this contact's vCard.
	 */
	void slotSaveVCard(QDomElement &);

	/**
	 * Send type="subscribed" to contact
	 */
	void slotSendAuth();

	/**
	 * Send type="subscribe" to contact
	 */
	void slotRequestAuth();

	void slotStatusOnline();
	void slotStatusChatty();
	void slotStatusAway();
	void slotStatusXA();
	void slotStatusDND();
	void slotStatusInvisible();

private:
	/**
	 * Initialize popup menu
	 */
	void initActions();

	/**
	 * Protocol identity (user ID) that this
	 * contact belongs to. Basically identifies
	 * the account into whose roster this contact
	 * belongs.
	 */
	QString mIdentityId;

	/**
	 * Protocol instance that this contact
	 * belongs to.
	 */
	JabberProtocol *protocol;

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
	 * This will simply cache all
	 * relevant data for this contact.
	 */
	Jabber::RosterItem rosterItem;

	/**
	 * List of available resources for the
	 * contact.
	 */
	QPtrList<JabberResource> resources;

	/**
	 * Current away reason and -type
	 */
	QString awayReason;
	JabberProtocol::Presence presence;

	/**
	 * Do we specifically send to a
	 * certain resource or do we use
	 * autoselection?
	 */
	bool resourceOverride, mEditingVCard;

	KActionCollection *actionCollection;

	KAction 	*actionRename,
			*actionSendAuth, *actionRequestAuth,
			*actionInfo, *actionStatusAway,
			*actionStatusOnline, *actionStatusChatty,
			*actionStatusXA, *actionStatusDND,
			*actionStatusInvisible;

	KSelectAction *actionSelectResource;
	KActionMenu *actionSetAvailability;
	KopeteMessageManager *m_manager;

	dlgJabberVCard *dlgVCard;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

