/*
  icqaccount.h  -  ICQ Account Class Header

  Copyright (c) 2002 by Chris TenHarmsel            <tenharmsel@staticmethod.net>
  Copyright (c) 2004 by Richard Smith               <kde@metafoo.co.uk>
  Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>

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

#include "oscaraccount.h"
#include "oscarmyselfcontact.h"

#include "oscartypeclasses.h"
#include "oscarpresence.h"

class KAction;
class KToggleAction;

namespace Kopete { class StatusMessage; }
namespace Xtraz { class Status; }
class ICQProtocol;
class ICQAccount;
class ICQUserInfoWidget;

class ICQMyselfContact : public OscarMyselfContact
{
Q_OBJECT
public:
	ICQMyselfContact( ICQAccount *acct );
	void userInfoUpdated();

public slots:
	void receivedShortInfo( const QString& );
	void fetchShortInfo();
};


class ICQAccount : public OscarAccount
{
Q_OBJECT

public:
	ICQAccount( Kopete::Protocol *parent, QString accountID );
	virtual ~ICQAccount();

	ICQProtocol *protocol();

	// fill the menu for this account
	virtual void fillActionMenu( KActionMenu *actionMenu );

	/** Reimplementation from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus&, const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                      const OnlineStatusOptions& options = None );
	void setStatusMessage( const Kopete::StatusMessage& );

	void connectWithPassword( const QString &password );

	void setUserProfile( const QString &profile );

protected:
	virtual OscarContact *createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const OContact& ssiItem );

protected slots:
	virtual void loginActions();
	virtual void disconnected( DisconnectReason reason );


private:
	Oscar::Presence presence();

	void setPresenceFlags( Oscar::Presence::Flags flags, const Kopete::StatusMessage &reason = Kopete::StatusMessage() );

	//const unsigned long fullStatus( const unsigned long plainStatus );

private slots:
	void setPresenceTarget( const Oscar::Presence &presence, const Kopete::StatusMessage &reason = Kopete::StatusMessage() );
	void setPresenceXStatus( const Xtraz::Status &xStatus );

	void slotToggleInvisible();

	void slotUserInfo();
	void storeUserInfoDialog();
	void closeUserInfoDialog();

	void userReadsStatusMessage( const QString& contact );

	void setXtrazStatus();
	void editXtrazStatuses();

	/** We have received an auth request */
	void slotGotAuthRequest( const QString& contact, const QString& reason );

	void addedInfoEventActionActivated( uint actionId );

private:
	bool mWebAware;
	bool mHideIP;
	Kopete::StatusMessage mInitialStatusMessage;
	ICQUserInfoWidget* mInfoWidget;

	KAction* mEditInfoAction;
	KToggleAction* mActionInvisible;
};

#endif
//kate: indent-mode csands;
