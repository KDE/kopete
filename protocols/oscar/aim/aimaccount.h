/*
  AIMAccount - Oscar Protocol Account

  Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>

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

#ifndef AIMACCOUNT_H
#define AIMACCOUNT_H

#include <qdict.h>
#include <qstring.h>
#include <qwidget.h>

#include "oscarsocket.h"
#include "oscaraccount.h"

class KAction;

namespace Kopete { class Contact; }
namespace Kopete { class Group; }

class OscarChangeStatus;
class OscarContact;
class AIMContact;

class AIMAccount : public OscarAccount
{
	Q_OBJECT

	public:
		AIMAccount(Kopete::Protocol *parent, QString accountID, const char *name=0L);
		virtual ~AIMAccount();

		// Accessor method for the action menu
		virtual KActionMenu* actionMenu();

		// Called from AIMUserInfo
		void setUserProfile(const QString &profile);

		void setAway(bool away, const QString &awayReason);

		virtual void setStatus(const unsigned long status,
			const QString &awayMessage = QString::null);

		virtual void connect();

	public slots:
		void slotEditInfo();
		void slotGoOnline();

	protected slots:
		// called after XML is read in, cannot access pluginData in constructor
		virtual void loaded();

		// Called when we have been warned
		void slotGotWarning(int newlevel, QString warner);

		//void slotGotMyUserInfo(UserInfo &);
		void slotGoAway(const QString&);

	protected:
		void initSignals();
		/** Why are these here?
		void initActions();
		void initActionMenu();
		*/

		/**
		 * Implement virtual method from OscarAccount
		 * This allows OscarAccount to take care of adding new contacts
		 */
		OscarContact *createNewContact( const QString &contactId,
			const QString &displayName, Kopete::MetaContact *parentContact, bool isOnSSI = false );

	private:
		void connect(const unsigned long status, const QString &awMessage);

	private:
		//UserInfo mUserInfo;
		unsigned long mStatus;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
