/***************************************************************************
                          jabbercontact.h  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : dstone@kde.org
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


#include "kopetecontact.h"


class KAction;
class KListAction;
class KPopupMenu;
class KSelectAction;

class dlgJabberRename;
class dlgJabberVCard;
class JabMessage;
class JabResource;
class JabberProtocol;
class JabberResource;
class Jid;
class JT_VCard;
class KopeteHistoryDialog;
class KopeteMessage;
class KopeteMessageManager;
class KopeteMetaContact;

class JabberContact : public KopeteContact
{
	Q_OBJECT
  
	public:

		JabberContact( QString userid, QString name, QString group,
				JabberProtocol *protocol, KopeteMetaContact *mc, QString identity);
				
		~JabberContact();

		/**
		 * Initialize contact
		 */
		void initContact(QString &userID, QString &name, QString &group);


		// Reimplementations of the (uninteresting)
		// members in KopeteContact
		
		/**
		 * Return contact's status
		 */
		virtual ContactStatus status() const;
		
		/**
		 * Return contact's status in textual form
		 */
		QString statusText() const;

		/**
		 * Return contact's status as icon name
		 */
		QString statusIcon() const;
	
		/**
		 * Return importance of contact.
		 * The importance is used for sorting and determined based
		 * on the contact's current status. See ICQ for reference values.
		 */
		int importance() const;

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
		
		/*
		 * Return the currently used resource for this contact
		 */
		QString resource() const
		{
			return mResource;
		}
		
		/*
		 * Return the group this contact resides in
		 */
		QString group() const
		{
			return mGroup;
		}
		
		/*
		 * Return this contact's user ID
		 */
		QString userID() const
		{
			return mUserID;
		}
		
		/*
		 * Returns if the contact is residing
		 * in a "virtual" local group or if it
		 * is in a group on the server
		 */
		bool localGroup()
		{
			return hasLocalGroup;
		}

		/*
		 * Show context menu for the contact
		 */
		virtual void showContextMenu(const QPoint&, const QString&);

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
		 * FIXME: Jabber does not support multiple copies of the same contact!
		 *        This will need emulation via the serialization API
		 */
		virtual void addToGroup( const QString &group );
		
		virtual void removeFromGroup( const QString &group );
		
		/**
		 * Move contact to a different group
		 */
		virtual void moveToGroup( const QString &from, const QString &to );

	public slots:

		/*
		 * Handle incoming message
		 */
		void slotNewMessage(const JabMessage &);
		
		/**
		 * Handle the removal of the log viewer
		 */
		void slotCloseHistoryDialog();

		/**
		 * View chat log
		 */
		void slotViewHistory();
		
		/*
		 * Send a message using the chat window
		 */
		void slotSendMsgKCW(const KopeteMessage&);

		/**
		 * Send a message using the email window
		 */
		void slotSendMsgKEW(const KopeteMessage&);
		
		/**
		 * Slots called when a certain resource
		 * appears or disappears for the contact
		 */
		void slotResourceAvailable(const Jid &, const JabResource &);
		
		/**
		 * Remove a resource from the contact
		 */
		void slotResourceUnavailable(const Jid &);

		/**
		* Select a new resource for the contact
		*/
		void slotSelectResource();
		
		/*
		 * Remove the user from the current group
		 * (will leave the group field empty)
		 */
		void slotRemoveFromGroup();

		/**
		 * vCard received from server for this contact
		 */
		void slotGotVCard(JT_VCard *);
		
		/**
		 * Update contact to a new status
		 */
		void slotUpdateContact(QString, int, QString);

	private slots:
		
		/**
		 * Delete this contact instance
		 */
		void slotDeleteMySelf(bool);

		/**
		 * Remove this contact from the roster
		 */
		void slotRemoveThisUser();
		
		/**
		 * Display a rename dialog
		 */
		void slotRenameContact();

		/**
		 * Catch the rename dialog's results
		 */
		void slotDoRenameContact();

		/**
		 * Move the contact to a different group
		 */
		void slotMoveThisUser();

		/**
		 * Open a chat window
		 */
		void slotChatThisUser();
		
		/**
		 * Open an email window
		 */
		void slotEmailUser();
	
		/**
		 * Retrieve a vCard for the contact
		 */
		void slotSnarfVCard();
		
		/**
		 * Set a new nickname for the contact
		 */
		void slotUpdateNickname(const QString);
		
		void slotSendAuth();
		
		void slotRerequestAuth();

	signals:
		void msgRecieved(QString, QString, QString, QString, QFont, QColor);

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
		
		JabberProtocol *mProtocol;
		JabberResource *activeResource;

		KopeteMetaContact *parentMetaContact;
		
		QPtrList<JabberResource> resources;
		QPtrList<KopeteContact> theContacts;

		bool hasLocalName, hasLocalGroup, hasResource;
		QString mUserID, mResource, mGroup, mReason;
		int mStatus;

		KPopupMenu *popup;
		KAction *actionMessage, *actionChat,
				*actionHistory, *actionSnarfVCard,
				*actionRename,
	 			*actionSendAuth, *actionRerequestAuth,
				*actionRemoveFromGroup, *actionRemove, *actionInfo;
		KListAction *actionContactMove;
		KSelectAction *actionSelectResource;

		dlgJabberRename *dlgRename;
		dlgJabberVCard *dlgVCard;
		KopeteMessageManager *mMsgManagerKCW, *mMsgManagerKEW;
		
		/**
		 * Singleton for returning a chat window
		 */
		KopeteMessageManager *msgManagerKCW();
		
		/**
		 * Singleton for returning an email window
		 */
		KopeteMessageManager *msgManagerKEW();

		KopeteHistoryDialog *historyDialog;

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

