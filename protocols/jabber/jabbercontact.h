/***************************************************************************
                          jabbercontact.h  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone, Till Gerken,
                           The Kopete Development Team
    email                : dstone@kde.org, till@tantalo.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
class KopeteHistoryDialog;
class KopeteMessage;
class KopeteMessageManager;
class KopeteMetaContact;

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
		 * Open a window for this contact (either chat or email)
		 */
		void execute();

		/**
		 * Return the identity ID
		 */
		virtual QString identityId() const
		{
			return mIdentityId;
		}

		/**
		 * Return the user ID
		 */
		QString userId() const
		{
			return rosterItem.jid().userHost();
		}

		/*
		 * Return the currently used resource for this contact
		 */
		QString resource() const
		{
			return activeResource->resource();
		}
		
		/*
		 * Return the group this contact resides in
		 */
		virtual QStringList groups() const
		{
			return rosterItem.groups();
		}

		/**
		 * Return the reason why we are away
		 */
		QString reason() const
		{
			return awayReason;
		}

		/**
		 * Return if we are reachable (defaults to
		 * true because we can send on- and offline
		 */
		virtual bool isReachable()
		{
			return true;
		}

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
		 * FIXME: old API?
		 */
		virtual QString id() const;
		virtual QString data() const;
		
		/*
		 * *** New API implementation ***
		 */
		
		/**
		 * Add contact to a group
		 */
		virtual void addToGroup( const QString &group );

		/**
		 * Remove contact from a group
		 */
		virtual void removeFromGroup( const QString &group );
		
		/**
		 * Move contact to a different group
		 */
		virtual void moveToGroup( const QString &from, const QString &to );

	public slots:

		/**
		 * Remove this contact from the roster
		 */
		virtual void slotDeleteContact();

		/*
		 * Handle incoming message
		 */
		void slotNewMessage(const Jabber::Message &message);
		
		/**
		 * Handle the removal of the log viewer
		 */
		void slotCloseHistoryDialog();

		/**
		 * View chat log
		 */
		void slotViewHistory();
		
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
		void slotGotVCard(Jabber::JT_VCard *);

		/**
		 * User wants to edit vCard
		 */
		//void slotEditVCard();

		/**
		 * User wants to save vCard on to server
		 */
		//void slotSaveVCard(QDomElement &);

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
		 * Open a chat window
		 */
		void slotChatUser();
		
		/**
		 * Open an email window
		 */
		void slotEmailUser();
	
		/**
		 * Edit a vCard for the contact.
		 */
		void slotEditVCard();

		/**
		 * Save this contact's vCard.
		 */
		void slotSaveVCard(QDomElement &);
		
		/**
		 * Append a message to the appropriate widgets
		 * and send it.
		 */
		void slotSendMsgKEW(const KopeteMessage &message);
		void slotSendMsgKCW(const KopeteMessage &message);

		/**
		 * Send type="subscribed" to contact
		 */
		void slotSendAuth();

		/**
		 * Send type="subscribe" to contact
		 */
		void slotRequestAuth();

		void slotStatusChat();
		void slotStatusAway();
		void slotStatusXA();
		void slotStatusDND();

		
	signals:
		void msgReceived(QString, QString, QString, QString, QFont, QColor);

	private:

		/**
		 * Factory for a Kopete Email Window
		 */
		KopeteMessageManager *msgManagerKEW();

		/**
		* Factory for a Kopete Chat Window
		*/
		KopeteMessageManager *msgManagerKCW();

		/**
		 * Initialize popup menu
		 */
		void initActions();

		/**
		 * Convert a KopeteMessage to a Jabber::Message
		 */
		void km2jm(const KopeteMessage &km, Jabber::Message &jm);
		
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

		KAction *actionMessage, *actionChat,
				*actionHistory, *actionRetrieveVCard,
				*actionRename,
	 			*actionSendAuth, *actionRequestAuth,
				*actionRemove, *actionInfo,
				*actionStatusAway, *actionStatusChat, *actionStatusXA, *actionStatusDND;
		
		KSelectAction *actionSelectResource;

		dlgJabberVCard *dlgVCard;

		KopeteHistoryDialog *historyDialog;
		KopeteMessageManager *mMsgManagerKCW;
		KopeteMessageManager *mMsgManagerKEW;

};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

