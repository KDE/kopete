/*
  icqcontact.h  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Stefan Gehn
  Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#ifndef ICQCONTACT_H
#define ICQCONTACT_H

#include "oscarcontact.h"
#include "oscarsocket.h"

#include <qwidget.h>
#include "kopetecontact.h"
#include "kopetemessage.h"

class AIMBuddy;

struct UserInfo;
class KAction;
class KopeteMessageManager;
class KopeteOnlineStatus;
class ICQProtocol;
class ICQAccount;
class OscarAccount;
class ICQUserInfo; // user info dialog

class ICQGeneralUserInfo;
class ICQWorkUserInfo;

/**
 * Contact for ICQ over Oscar protocol
 * @author Stefan Gehn
 */
class ICQContact : public OscarContact
{
	Q_OBJECT
	// don't want to expose userinfo
	// for the dialog we make an exception to save a ton of var() {return mvar;}
	friend class ICQUserInfo;

	public:
		ICQContact(const QString name, const QString displayName,
					ICQAccount *account, KopeteMetaContact *parent);
		virtual ~ICQContact();

		/**
		* Returns a set of custom menu items for
		* the context menu
		*/
		virtual KActionCollection *customContextMenuActions();

		/* Return whether or not this contact is REACHABLE. */
		virtual bool isReachable();

		virtual void setStatus(const unsigned int newStatus);

		void requestUserInfo();

	public slots:
		virtual void slotUserInfo();

	signals:
		void updatedUserInfo();

	private:
		ICQProtocol *mProtocol;
		ICQUserInfo *infoDialog;

		ICQGeneralUserInfo generalInfo;
		ICQWorkUserInfo workInfo;

		int userinfoRequestSequence;
		int userinfoReplyCount;

	private slots:
		/** Called when the userinfo dialog is getting closed */
		void slotCloseUserInfoDialog();
		/** Called when a buddy has changed status */
		void slotContactChanged(UserInfo u);

		/** Called when a buddy is offgoing */
		void slotOffgoingBuddy(QString sn);
		/** Called when we want to send a message */
		void slotSendMsg(KopeteMessage&, KopeteMessageManager *);
		/** Called when an IM is received */
		void slotIMReceived(QString sender, QString msg, bool isAuto);

		void slotUpdGeneralInfo(const int seq, const ICQGeneralUserInfo &inf);
		void slotUpdWorkInfo(const int seq, const ICQWorkUserInfo &inf);
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
