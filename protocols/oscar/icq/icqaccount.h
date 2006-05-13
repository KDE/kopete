/*
  icqaccount.h  -  ICQ Account Class Header

  Copyright (c) 2002 by Chris TenHarmsel            <tenharmsel@staticmethod.net>
  Copyright (c) 2004 by Richard Smith               <kde@metafoo.co.uk>
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

#ifndef ICQACCOUNT_H
#define ICQACCOUNT_H

#include "oscaraccount.h"
#include "oscarmyselfcontact.h"

#include "icqpresence.h"
#include "oscartypeclasses.h"

class KAction;
namespace Kopete { class AwayAction; }
class ICQProtocol;
class ICQAccount;
class OscarVisibilityDialog;

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
	ICQAccount( Kopete::Protocol *parent, QString accountID, const char *name = 0L );
	virtual ~ICQAccount();

	ICQProtocol *protocol();

	// Accessor method for the action menu
	virtual KActionMenu* actionMenu();

	/** Reimplementation from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus&, const QString& );

	virtual void setAway( bool away, const QString &awayReason );

	void connectWithPassword( const QString &password );

	void setUserProfile( const QString &profile );

protected:
	virtual OscarContact *createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const SSI& ssiItem );

	virtual QString sanitizedMessage( const QString& message );

protected slots:
	virtual void disconnected( DisconnectReason reason );


private:
	ICQ::Presence presence();

	void setInvisible( ICQ::Presence::Visibility );
	void setPresenceType( ICQ::Presence::Type, const QString &awayMessage = QString::null );
	void setPresenceTarget( const ICQ::Presence &presence, const QString &message = QString::null );

	//const unsigned long fullStatus( const unsigned long plainStatus );

private slots:
	void slotToggleInvisible();

	void slotSetVisiblility();
	void slotVisibilityDialogClosed();

	void slotGlobalIdentityChanged( const QString& key, const QVariant& value );

	void slotBuddyIconChanged();

private:
	bool mWebAware;
	bool mHideIP;
	QString mInitialStatusMessage;
	OscarVisibilityDialog* m_visibilityDialog;
};

#endif
//kate: indent-mode csands;
