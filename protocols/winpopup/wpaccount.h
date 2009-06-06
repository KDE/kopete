/***************************************************************************
     wpaccount.h  -  Base class for the Kopete WP account
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WPACCOUNT_H
#define WPACCOUNT_H


// QT Includes
#include <qpixmap.h>

// KDE Includes

// Kopete Includes
#include "kopetemetacontact.h"
#include "kopeteaccount.h"
#include "kopeteonlinestatus.h"

// Local Includes
#include "wpcontact.h"
#include "wpaddcontact.h"

class KActionMenu;
class KAction;
class WPProtocol;

namespace Kopete
{
	class StatusMessage;
}
/**
 * The actual Account class used by Kopete.
 */
class WPAccount : public Kopete::Account
{
	Q_OBJECT

// Kopete::Account overloading
public:
	WPAccount(WPProtocol *parent, const QString& accountID);
	~WPAccount();

	virtual void fillActionMenu( KActionMenu *actionMenu );			// Per-protocol actions for the systray and the status bar
	virtual bool hasCustomStatusMenu() const;		//Has custom status menu
	virtual void setAway(bool status, const QString &);	// Set user away

public slots:
	virtual void connect(const Kopete::OnlineStatus &);						// Connect to server
	virtual void disconnect();					// Disconnect from server

	void goAvailable() { setAway(false, QString()); }		// Two convenience slots
	void goAway() { setAway(true, QString()); }			// for available and away

// Stuff used internally & by colleague classes
public:
	const QStringList getGroups();
	const QStringList getHosts(const QString &Group);

// Stuff used by WPContact
public:
	/**
	 * Returns whether or not the named host is online.
	 */
	bool checkHost(const QString &Name);

public slots:
	/**
	 * Dispatches said message to the destination.
	 */
	void slotSendMessage(const QString &Body, const QString &Destination);

	/**
	 * Called when a new message arrives with the message's data.
	 */
	void slotGotNewMessage(const QString &Body, const QDateTime &Arrival, const QString &From);

	/* Reimplemented from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus &status , const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                      const OnlineStatusOptions& options = None );
	void setStatusMessage(const Kopete::StatusMessage& statusMessage);

protected:
	virtual bool createContact(const QString &contactId, Kopete::MetaContact *parentContact);

private slots:
//	void updateAccountId();

private:
	WPProtocol *mProtocol;
	QString theAwayMessage;			// The message to give when the user is away
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
