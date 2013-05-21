 /*
  * jabbercontact.h  -  Base class for the Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2004 by Till Gerken <till@tantalo.net>
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

#ifndef JABBERBASECONTACT_H
#define JABBERBASECONTACT_H

#include "kopetecontact.h"
#include "xmpp.h"
#include "im.h"

class JabberProtocol;
class JabberAccount;
class JabberTransport;
namespace Kopete { class MetaContact; }
namespace XMPP { class VCard; }

class JabberBaseContact : public Kopete::Contact
{

Q_OBJECT
friend class JabberAccount;	/* Friends can touch each other's private parts. */

public:

	/**
	 * @param legacyId is the contactId of the contact if != Jid
	 */
	JabberBaseContact (const XMPP::RosterItem &rosterItem,
					   Kopete::Account *account, Kopete::MetaContact * mc, 
					   const QString &legacyId=QString());

	/********************************************************************
	 *
	 * Kopete::Contact reimplementation start
	 *
	 ********************************************************************/

	/**
	 * Return the protocol instance associated with this contact
	 */
	JabberProtocol *protocol ();

	/**
	 * Return the account instance associated with this contact
	 */
	JabberAccount *account () const { return m_account; };
	
	/**
	 * return the transport if any, or null
	 */
	JabberTransport *transport();
			
	/**
	 * Return if the contact is reachable (this is true if the account
	 * is online)
	 */
	virtual bool isReachable ();

	/**
	 * Create custom context menu items for the contact
	 * FIXME: implement manager version here?
	 */
	virtual QList<KAction*> *customContextMenuActions () = 0;
	using Kopete::Contact::customContextMenuActions;

	/**
	 * Serialize contact
	 */
	virtual void serialize (QMap < QString, QString > &serializedData, QMap < QString, QString > &addressBookData);

	/**
	 * Update contact if a roster item has been
	 * received for it. (used during login)
	 */
	void updateContact ( const XMPP::RosterItem &item );

	/**
	 * Deal with an incoming message for this contact.
	 */
	virtual void handleIncomingMessage ( const XMPP::Message &message ) = 0;

	/**
	 * Update the resource property of the
	 * contact, listing all available resources.
	 */
	void updateResourceList ();

	/**
	 * Return current full address.
	 * Uses bestResource() if no presubscribed
	 * address exists.
	 */
	QString fullAddress ();

	/**
	 * Set the dontSync flag for this contact.
	 * If this flag is set, calls to @ref sync will
	 * be ignored. This is required if the contact
	 * has been moved between groups on the server
	 * after we logged in and we try to update our
	 * local contact list. Since libkopete can only
	 * handle one group update at a time, moving
	 * between groups requires to operations which
	 * each in turn would cause a call to sync(),
	 * overwriting the change that is being carried
	 * out. (besides causing unnecessary traffic)
	 * This is avoided by setting the dontSync flag
	 * while synchronizing the local copy.
	 */
	void setDontSync ( bool flag );

	/**
	 * Return the status of the dontSync flag.
	 * See @ref setDontSync for a full description.
	 */
	bool dontSync ();
	
	/**
	 * return the roster item of the contact.
	 * to get the jid, use  rosterItem().jid().full()  don't use contactId as it is not the same with transport
	 */
	XMPP::RosterItem rosterItem() const { return mRosterItem; }
	
	/**
	 * Reads a vCard object and updates the contact's
	 * properties accordingly.
	 */
	void setPropertiesFromVCard ( const XMPP::VCard &vCard );

	/**
	 * Set contact's client sends delivery event flag.
	 */
	void setSendsDeliveredEvent( bool sends ) { mSendsDeliveredEvent = sends; }

	/**
	 * Return true if contact's client sends delivery event.
	 */
	bool sendsDeliveredEvent() const { return mSendsDeliveredEvent; }

	virtual bool isContactRequestingEvent( XMPP::MsgEvent /*event*/ ) { return false; }
	virtual bool isContactRequestingReceiptDelivery() { return false; }
	
	virtual QString lastReceivedMessageId () const { return QString(); }

public slots:

	/**
	 * Retrieve a vCard for the contact
	 */
	virtual void slotUserInfo ();
	
	
	/**
	 * Re-evaluate online status. Gets called
	 * whenever a resource is added, removed, or
	 * changed in the resource pool.
	 */
	void reevaluateStatus ();

protected:
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

	/**
	 * Flag if contact's client sends delivery event.
	 */
	bool mSendsDeliveredEvent;

private:
	bool mDontSync;
	JabberAccount *m_account;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:
