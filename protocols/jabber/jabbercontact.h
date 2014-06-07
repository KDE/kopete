 /*
  * jabbercontact.cpp  -  Regular Kopete Jabber protocol contact
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

#ifndef JABBERCONTACT_H
#define JABBERCONTACT_H

#include "jabberbasecontact.h"
#include "xmpp_vcard.h"

#include "kopetechatsession.h" // needed for silly Kopete::ContactPtrList

#include <QList>

class JabberChatSession;
class QTimer;

class JabberContact : public JabberBaseContact
{

Q_OBJECT

public:

	JabberContact (const XMPP::RosterItem &rosterItem,
				   Kopete::Account *account, Kopete::MetaContact * mc, const QString &legacyId = QString());
	
	~JabberContact();

	/**
	 * Create custom context menu items for the contact
	 * FIXME: implement manager version here?
	 */
	QList<KAction*> *customContextMenuActions ();
	using JabberBaseContact::customContextMenuActions;

	/**
	 * Start a rename request.
	 */
	void rename ( const QString &newName );

	/**
	 * Deal with an incoming message for this contact.
	 */
	void handleIncomingMessage ( const XMPP::Message &message );

	/**
	 * Create a message manager for this contact.
	 * This variant is a pure single-contact version and
	 * not suitable for groupchat, as it only looks for
	 * managers with ourselves in the contact list.
	 */
	Kopete::ChatSession *manager ( Kopete::Contact::CanCreateFlags );
	

	virtual bool isContactRequestingEvent( XMPP::MsgEvent event );
	virtual bool isContactRequestingReceiptDelivery();

	virtual QString lastReceivedMessageId () const;

public slots:

	/**
	 * Remove this contact from the roster
	 */
	void deleteContact ();

	/**
	 * Sync Groups with server
	 * 
	 * operations are alctually performed in sloDelayedSync()
	 */
	void sync(unsigned int);

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

	/**
	 * Update the vCard on the server.
	 * @todo is that still used ?
	 */ 
	void slotSendVCard();

	/**
	 * Set contact photo.
	 * @param path Path to the photo.
	 */
	void setPhoto(const QString &photoPath);
	
	/**
	 * this will start a voice call to the contact
	 */
	//void voiceCall();
	 
	 /**
	  * This is the JabberContact level slot for starting a Jingle Video call.
	  * See later for params...
	  */
#ifdef JINGLE_SUPPORT
	void showSessionsGui();
	void startJingleSession();
	void startJingleAudioCall();
	void startJingleVideoCall();
#endif

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

	/**
	* Select a new resource for the contact
	*/
	void slotSelectResource ();

	void slotChatSessionDeleted ( QObject *sender );
	
	/**
	 * Check if cached vCard is recent.
	 * Triggered as soon as Kopete changes its online state.
	 */
	void slotCheckVCard ();

	/**
	 * Triggered from a timer, requests the vCard.
	 * Timer is initiated by @ref slotCheckVCard.
	 */
	void slotGetTimedVCard ();
	
	/**
	 * Passes vCard on to parsing function.
	 */
	void slotGotVCard ();

	/**
	 * Get information about last activity of the contact.
	 * Triggered as soon as Kopete goes online or the contact goes offline.
	 */
	void slotCheckLastActivity ( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & );

	/**
	 * Triggered from a timer, requests last activity information.
	 * Timer is initiated by @ref slotCheckLastActivity.
	 */
	void slotGetTimedLastActivity ();

	/**
	 * Updates activity information.
	 */
	void slotGotLastActivity ();

	/**
	 * Display a error message if the vCard sent was unsuccessful.
	 */
	void slotSentVCard();
	
	/**
	 * The service discovery on that contact is finished
	 */
	void slotDiscoFinished();
	
	/**
	 * actually perform operations of sync() with a delay.
	 * slot received by the syncTimer.
	 */
	void slotDelayedSync();

#ifdef LIBJINGLE_SUPPORT
	void makeLibjingleCallAction();
#endif

private:

	/**
	 * Create a message manager for this contact.
	 * This variant is a pure single-contact version and
	 * not suitable for groupchat, as it only looks for
	 * managers with ourselves in the contact list.
	 * Additionally to the version above, this one adds
	 * a resource constraint that has to be matched by
	 * the manager. If a new manager is created, the given
	 * resource is preselected.
	 */
	JabberChatSession *manager ( const QString &resource, Kopete::Contact::CanCreateFlags );

	/**
	 * Create a message manager for this contact.
	 * This version is suitable for groupchat as it
	 * looks for a message manager with a given
	 * list of contacts as members.
	 */
	JabberChatSession *manager ( Kopete::ContactPtrList chatMembers, Kopete::Contact::CanCreateFlags );

	/**
	 * Sends subscription messages.
	 */
	void sendSubscription (const QString& subType);

	/**
	 * Sends a presence packet to this contact
	 */
	void sendPresence ( const XMPP::Status status );

	/**
	 * This variable keeps a list of message managers.
	 * It is required to locate message managers by
	 * resource name, if one account is interacting
	 * with several resources of the same contact
	 * at the same time. Note that this does *not*
	 * apply to groupchats, so this variable
	 * only contains classes of type JabberChatSession.
	 * The casts in manager() and slotChatSessionDeleted()
	 * are thus legal.
	 */
	QList<JabberChatSession*> mManagers;

	/**
	 * Indicates whether the vCard is currently
	 * being updated or not.
	 */
	bool mVCardUpdateInProgress :1;

	bool mRequestComposingEvent :1;
	bool mRequestOfflineEvent :1;
	bool mRequestDisplayedEvent :1;
	bool mRequestDeliveredEvent :1;
	bool mRequestGoneEvent :1;
	/**
	 * tell if the disco#info has been done for this contact.
	 */
	bool mDiscoDone :1;

	bool mRequestReceiptDelivery :1;
	QString mLastReceivedMessageId;
	QTimer *m_syncTimer;

};

#endif
