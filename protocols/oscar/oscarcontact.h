/*
    oscarcontact.h  -  Oscar Protocol Plugin

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

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

#ifndef OSCARCONTACT_H
#define OSCARCONTACT_H

#include <qwidget.h>
#include "kopetetransfermanager.h"
#include "kopetecontact.h"
#include "kopetemessage.h"

/**
  * Contact for oscar protocol
  * @author Tom Linsky <twl6@po.cwru.edu>
  */

struct UserInfo;
class QTimer;
class OscarProtocol;
class KopeteMessageManager;
class OscarConnection;

class OscarContact : public KopeteContact
{
   Q_OBJECT
public:
	OscarContact(const QString name,
			OscarProtocol *protocol,
			KopeteMetaContact *parent);
	~OscarContact();
	/**
	 * Return the protocol specific serialized data
	 * that a plugin may want to store a contact list.
	 */
	virtual QString data(void) const;
	/** Returns the online status of the contact */
	virtual ContactStatus status(void) const;
	/** Returns the status icon of the contact */
	virtual QString statusIcon(void) const;
	/**
	 * Returns a set of custom menu items for
	 * the context menu
	 */
	virtual KActionCollection *customContextMenuActions(void);
	/* Return whether or not this contact is REACHABLE. */
	virtual bool isReachable(void);

	/**
	 * Returns the direct connection state of the contact
	 * true = a direct connection is established with the contact
	 * false = a direct connection is not established with the contact
	 */
	bool isDirectConnected() const { return mDirectlyConnected; };

	virtual KopeteMessageManager *manager( bool canCreate = false );

public slots:
	/** Method to delete a contact from the contact list */
	virtual void slotDeleteContact(void);
	/** Send a file */
	virtual void sendFile(const KURL &sourceURL, const QString &altFileName, const long unsigned int fileSize);

public: // Public attributes
	/** The name of the contact */
	QString mName;
	/** The status of the contact */
	int mStatus;
	/** List of contacts.. I don't want this to be here */
	QPtrList<KopeteContact> theContacts;
signals:
	void messageSuccess();

private: // Private members

	/** Initialzes the actions */
	void initActions(void);

	/**
	 * parses HTML AIM-Clients send to us and
	 * strips off most of it
	 */
	KopeteMessage parseAIMHTML ( QString m );
	/* used by above to strip off a tag*/
//	QStringList removeTag ( QString &message, QString tag );

private: // Private attributes
	KopeteMessageManager *mMsgManager;
	KAction* actionWarn;
	KAction* actionBlock;
	KAction* actionDirectConnect;
	KActionCollection* actionCollection;

	OscarProtocol *mProtocol;
	
	/**
	 * The time of the last autoresponse,
	 * used to determine when to send an
	 * autoresponse again.
	 */
	long mLastAutoResponseTime;
		
	/** The contact's idle time */
	int mIdle;
	/** Tells whether or not we have a direct connection with the contact */
	bool mDirectlyConnected;

private slots: // Private slots
	/** Called when a buddy changes */
	void slotUpdateBuddy(int buddyNum);
	/** Called when a buddy has changed status */
	void slotBuddyChanged(UserInfo u);
	/** Called when we get a minityping notification */
	void slotGotMiniType(QString screenName, int type);
	/**
	 * Called when we are notified by the chat window
	 * that this person is being typed to...
	 */
	void slotTyping(bool typing);
	/** Called when a buddy is offgoing */
	void slotOffgoingBuddy(QString sn);
	/** Called when user info is requested */
	void slotUserInfo(void);
	/** Called when we want to send a message */
	void slotSendMsg(KopeteMessage&, KopeteMessageManager *);
	/** Called when an IM is received */
	void slotIMReceived(QString sender, QString msg, bool isAuto);
	/** Called when nickname needs to be updated */
	void slotUpdateNickname(const QString);
	/** Warn the user */
	void slotWarn(void);
	/** Called when the status of the Kopete user(behind this computer)'s status has changed */
	void slotMainStatusChanged(int);
	/** Called when KMM is destroyed */
	void slotMessageManagerDestroyed();
	/** Called when we want to block the contact */
	void slotBlock(void);
	/** Called when we want to connect directly to this contact */
	void slotDirectConnect();
	/** Called when we become directly connected to the contact */
	void slotDirectIMReady(QString name);
	/** Called when the direct connection to name has been terminated */
	void slotDirectIMConnectionClosed(QString name);
	/** Called when someone wants to send us a file */
	void slotGotFileSendRequest(QString sn, QString message, QString filename, unsigned long filesize);
	/** Called when we deny a transfer */
	void slotTransferDenied(const KopeteFileTransferInfo &tr);
	/** Called when a pending transfer has been accepted */
	void slotTransferAccepted(KopeteTransfer *, const QString &fileName);
	/** Called when a file transfer begins */
	void slotTransferBegun(OscarConnection *con, const QString& file, const unsigned long size, const QString &recipient);
};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

