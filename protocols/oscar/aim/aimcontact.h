/*
  oscarcontact.h  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Will Stephenson
  Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#ifndef AIMCONTACT_H
#define AIMCONTACT_H

#include "oscarcontact.h"

class AIMAccount;
class AIMProtocol;
class KopeteMessageManager;
class AIMUserInfoDialog;

class AIMContact : public OscarContact
{
	Q_OBJECT

	public:
		AIMContact(const QString , const QString , AIMAccount *, KopeteMetaContact *);
		virtual ~AIMContact();

		bool isReachable();
		QPtrList<KAction> *customContextMenuActions();

		/**
		 * Reimplemented because AIM handles start/stop of typing
		 */
		KopeteMessageManager* manager(bool canCreate = false);

		virtual void setStatus(const unsigned int newStatus);

		const QString &userProfile() { return mUserProfile; }

		/*
		 * Only usable for the myself() contact
		 */
		void setOwnProfile(const QString &profile);

		virtual const QString awayMessage();
		virtual void setAwayMessage(const QString &message);

	protected:
		/**
		 * parses HTML AIM-Clients send to us and
		 * strips off most of it
		 */
		KopeteMessage parseAIMHTML(const QString &m);

		AIMProtocol* mProtocol;

		/**
		* The time of the last autoresponse,
		* used to determine when to send an
		* autoresponse again.
		*/
		long mLastAutoResponseTime;

	signals:
		void updatedProfile();

	private slots:
		/**
		 * Called when we get a minityping notification
		 */
		void slotGotMiniType(const QString &screenName, int type);
		void slotTyping(bool typing);

		/**
		 * Called when a buddy has changed status
		 */
		void slotContactChanged(const UserInfo &);

		/**
		 * Called when a buddy is going offline
		 */
		void slotOffgoingBuddy(QString sn);

		/**
		 * Called when we want to send a message
		 */
		void slotSendMsg(KopeteMessage&, KopeteMessageManager *);

		/**
		 * Called when the user requests a contact's user info
		 */
		void slotUserInfo();

		/**
		 * Warn the user
		 */
		void slotWarn();

		void slotGotProfile(const UserInfo &user, const QString &profile, const QString &away);

		void slotCloseUserInfoDialog();

	private:
		QString mUserProfile;
		AIMUserInfoDialog *infoDialog;
		KAction *actionRequestAuth;
		KAction *actionSendAuth;
		KAction *actionWarn;
		KAction *actionBlock;
};
#endif
