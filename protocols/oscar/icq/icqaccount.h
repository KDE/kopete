/*
  icqaccount.h  -  ICQ Account Class Header

  Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>
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

#ifndef ICQACCOUNT_H
#define ICQACCOUNT_H

#include <qdict.h>
#include <qstring.h>
#include <qwidget.h>

#include <oscaraccount.h>
#include <oscarsocket.h>
#include <oscarsocket.icq.h>

class KActionMenu;
namespace Kopete { class Protocol; }

class OscarChangeStatus;
class ICQAccount : public OscarAccount
{
	Q_OBJECT

	public:
		ICQAccount(Kopete::Protocol *parent, QString accountID, const char *name=0L);
		~ICQAccount();

		KActionMenu* actionMenu();
		virtual void setAway(bool away, const QString &awayReason);

		virtual void setStatus(const unsigned long status,
			const QString &awayMessage = QString::null);

		void connectWithPassword(const QString &password);

		void setInvisible(bool);
		void reloadPluginData();

	public slots:
		void slotGoOnline();
		void slotGoAway( const QString & );
		void slotGoNA( const QString & );
		void slotGoOCC( const QString & );
		void slotGoFFC( const QString & );
		void slotGoDND( const QString & );
		void slotToggleInvisible();

	protected slots:
		// called after XML is read in, cannot access pluginData in constructor
		virtual void loaded();
		void slotSendSMS();

	protected:
		virtual OscarContact *createNewContact(const QString &contactId,
			const QString &displayName, Kopete::MetaContact *parentContact, bool isOnSSI = false);

	private:
		const unsigned long fullStatus(const unsigned long plainStatus);

	private:
		unsigned long mStatus; // icq status minux flags for web-aware, hide-ip and invisible
		bool mWebAware;
		bool mHideIP;
		bool mInvisible;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
