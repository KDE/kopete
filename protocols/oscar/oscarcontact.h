/*
  oscarcontact.h  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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
#include <qdatetime.h>
#include <kaction.h>
#include "kopetetransfermanager.h"
#include "kopetecontact.h"
#include "kopetemessage.h"

class AIMBuddy;

struct UserInfo;
class KAction;
class KopeteMessageManager;
class KopeteOnlineStatus;
class OscarConnection;
class OscarProtocol;
class OscarAccount;
class QTimer;

/**
 * Contact for oscar protocol
 * @author Tom Linsky <twl6@po.cwru.edu>
 * @author Chris TenHarmsel <tenharmsel@staticmethod.net>
 * @author Stefan Gehn <sgehn@gmx.net>
 */
class OscarContact : public KopeteContact
{
	Q_OBJECT

	public:
		OscarContact(const QString name, const QString displayName,
					KopeteAccount *account, KopeteMetaContact *parent);
		virtual ~OscarContact();

		/**
		* Returns the direct connection state of the contact
		* true = a direct connection is established with the contact
		* false = a direct connection is not established with the contact
		*/
		bool isDirectConnected() const { return mDirectlyConnected; };

		virtual void rename(const QString &);

		virtual KopeteMessageManager *manager( bool canCreate = false );

		const QString &contactname() { return mName; };
		OscarAccount *account() { return mAccount; };

		/*
		 * Convert between internal status representation
		 * and Oscar implementing protocols' own KOS
		 */
		virtual void setStatus(const unsigned int newStatus)=0;

		/*
		 * Returns the idle time
		 */
		const unsigned int idleTime() { return mIdle; }

		const unsigned long realIP() { return mRealIP; }
		const unsigned long localIP() { return mLocalIP; }
		const unsigned int  port() { return mPort; }
		const unsigned int  fwType() { return mFwType; }
		const unsigned int  tcpVersion() { return mTcpVersion; }
		const QDateTime signonTime() { return mSignonTime; }

	public slots:
		/** Method to delete a contact from the contact list */
		virtual void slotDeleteContact(void);

		/** Send a file */
		virtual void sendFile(const KURL &sourceURL, const QString &altFileName,
			const long unsigned int fileSize);

		/** Called when the metacontact owning this contact changes groups */
		virtual void syncGroups();

		/** Called when a buddy changes */
		void slotUpdateBuddy();

	protected:
		/** The account we're associated with */
		OscarAccount *mAccount;

		/** The name of the contact */
		QString mName;

		KActionCollection* actionCollection;

		KopeteMessageManager *mMsgManager;

		/**
		* The internal representation of our AIMBuddy
		* in the internal buddy list
		*/
		AIMBuddy *mListContact;

	private:
		/** Initializes signal connections */
		void initSignals();

		/** Initialzes the actions */
		void initActions();

	private:
		/*
		 * stuff filled from UserInfo
		 */
		unsigned int mIdle;
		unsigned long mRealIP;
		unsigned long mLocalIP;
		unsigned int mPort;
		unsigned int mFwType;
		unsigned int mTcpVersion;
		QDateTime mSignonTime;

		/** Tells whether or not we have a direct connection with the contact */
		bool mDirectlyConnected;

		int groupID; // TODO: move the server groupid to OscarContact!

	private slots:
		/** Called when a buddy has changed status */
//		void slotBuddyChanged(UserInfo u);
		/**
		* Called when we are notified by the chat window
		* that this person is being typed to...
		*/
//		void slotTyping(bool typing);
		/** Called when a buddy is offgoing */
		void slotOffgoingBuddy(QString sn);
		/** Called when we want to send a message */
//		void slotSendMsg(KopeteMessage&, KopeteMessageManager *);
		/** Called when an IM is received */
//		void slotIMReceived(QString sender, QString msg, bool isAuto);
		/** Called when nickname needs to be updated */
		void slotUpdateNickname(const QString);

		/**
		* Called when the status of the Kopete user(behind this computer)'s status has changed
		*/
		void slotMainStatusChanged(const unsigned int);

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
		void slotGotFileSendRequest(QString sn, QString message,
			QString filename, unsigned long filesize);
		/** Called when we deny a transfer */
		void slotTransferDenied(const KopeteFileTransferInfo &tr);
		/** Called when a pending transfer has been accepted */
		void slotTransferAccepted(KopeteTransfer *, const QString &fileName);
		/** Called when a file transfer begins */
		void slotTransferBegun(OscarConnection *con, const QString& file,
			const unsigned long size, const QString &recipient);
		/** Called when a contact from the Kopete contact list has been removed */
		void slotGroupRemoved( KopeteGroup * );

		void slotParseUserInfo(const UserInfo &);
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
