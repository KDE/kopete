/*
  icqcontact.h  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Stefan Gehn  <metz AT gehn.net>
  Copyright (c) 2003 by Olivier Goffart
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

#include <qwidget.h>
#include "kopetecontact.h"
#include "kopetemessage.h"

struct UserInfo;
class KAction;
class KToggleAction;
namespace Kopete { class MessageManager; }
namespace Kopete { class OnlineStatus; }
class ICQProtocol;
class ICQAccount;
class OscarAccount;
class ICQUserInfo; // user info dialog
class ICQReadAway;

class ICQGeneralUserInfo;
class ICQWorkUserInfo;

/*
 * Contact for ICQ over Oscar protocol
 * @author Stefan Gehn
 */
class ICQContact : public OscarContact
{
	Q_OBJECT
	// don't want to expose userinfo
	// for the dialog we make an exception to save a ton of var() {return mvar;}
	friend class ICQProtocol;

	public:
		ICQContact(const QString name, const QString displayName,
			ICQAccount *account, Kopete::MetaContact *parent);

		virtual ~ICQContact();

		/*
		 * Returns a set of custom menu items for
		 * the context menu
		 */
		virtual QPtrList<KAction> *customContextMenuActions();

		/* Return whether or not this contact is REACHABLE. */
		virtual bool isReachable();

		virtual void setStatus(const unsigned int newStatus);

		/** Request full user info */
		void requestUserInfo();

		/** Request short user info */
		void requestShortInfo();

		/*
		 * Do NOT use this for anything but the ICQAccount::myself() contact!
		 * This avoids using OscarContact::rename() which triggers a renaming on
		 * the server side contactlist as well
		 */
		void setOwnDisplayName(const QString &);

		/*
		 * Reimplemented because invisible contacts have a
		 * small auto-modifying status
		 */
		void setOnlineStatus(const Kopete::OnlineStatus&);

		virtual const QString awayMessage();
		virtual void setAwayMessage(const QString &message);

	public slots:
		virtual void slotUserInfo();

	signals:
		void updatedUserInfo();
		void userInfoRequestFailed();

	private:
		void incUserInfoCounter();

	private:
		ICQProtocol *mProtocol;
		ICQUserInfo *infoDialog;
		ICQReadAway *awayMessageDialog;

		ICQGeneralUserInfo generalInfo;
		ICQWorkUserInfo workInfo;
		ICQMoreUserInfo moreInfo;
		QString aboutInfo;
		ICQMailList emailInfo;
		ICQInfoItemList interestInfo;
		ICQInfoItemList currentBackground;
		ICQInfoItemList pastBackground;
		ICQSearchResult shortInfo;

		KAction *actionReadAwayMessage;
		KAction *actionRequestAuth;
		KAction *actionSendAuth;
		KToggleAction *actionIgnore;
		KToggleAction *actionVisibleTo;

		int userinfoRequestSequence;
		unsigned int userinfoReplyCount;
		bool mInvisible;

	private slots:
		void slotCloseUserInfoDialog();
		void slotCloseAwayMessageDialog();

		/*
		 * Called when a buddy has changed status
		 */
		void slotContactChanged(const UserInfo &u);

		/*
		 * Called when a buddy is offgoing
		 */
		void slotOffgoingBuddy(QString sn);

		/*
		 * Called when we want to send a message
		 */
		void slotSendMsg(Kopete::Message&, Kopete::MessageManager *);

		void slotUpdGeneralInfo(const int, const ICQGeneralUserInfo &);
		void slotUpdWorkInfo(const int, const ICQWorkUserInfo &);
		void slotUpdMoreUserInfo(const int, const ICQMoreUserInfo &);
		void slotUpdAboutUserInfo(const int, const QString &);
		void slotUpdEmailUserInfo(const int, const ICQMailList &);
		void slotUpdShortInfo(const int, const ICQSearchResult &);

		/*
		* Store the interest user info for this contact and see if we have
		* received all the info we support.
		*/
		void slotUpdInterestUserInfo(const int, const ICQInfoItemList& );

		/*
		* Store the background user info for this contact and see if we have
		* received all the info we support
		*/
		void slotUpdBackgroundUserInfo( const int seq, const ICQInfoItemList &curr, const ICQInfoItemList &past );

		void slotReadAwayMessage();
		void slotSnacFailed(WORD snacID);
		void slotIgnore();
		void slotVisibleTo();
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
