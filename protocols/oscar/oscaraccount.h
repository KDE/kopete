/*
OscarAccount - Oscar Protocol Account

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

#ifndef OSCARACCOUNT_H
#define OSCARACCOUNT_H

#include <qwidget.h>
#include <qdict.h>
#include <qstring.h>
#include "oscarprotocol.h"
#include "aimbuddylist.h"

#include "kopeteaccount.h"

class KAction;

class KopeteContact;
class KopeteGroup;

class OscarAwayDialog;
class OscarProtocol;
class OscarContact;
class OscarSocket;

class AIMBuddyList;


class OscarAccount : public KopeteAccount
{
	Q_OBJECT

public:
	/** Default constructor */
	OscarAccount(KopeteProtocol *parent,
		const QString& accountID, 
		const char *name=0L);

	/** Default destructor */
	virtual ~OscarAccount();

	/** Connects this account */
	void connect();

	/** Disconnects this account  */
	void disconnect();

	/** Sets the account away */
	virtual void setAway( bool away );

	/** Accessor method for this account's contact */
	virtual KopeteContact* myself() const;

	/** Accessor method for our engine object */
	virtual OscarSocket* getEngine();

	/** Accessor method for the action menu */
	KActionMenu* actionMenu();
	
public slots:
	/** Slot for telling this account to go online */
	void slotGoOnline();
	/** Slot for telling this account to go offline */
	void slotGoOffline();
	/** Slot for telling this account to go away */
	void slotGoAway();
	/** Slot for editing our info */
	void slotEditInfo();
	/** Slot for showing the debug dialog */
	void slotShowDebugDialog();

protected slots:
	/** Called when we get disconnected */
	void slotDisconnected();
	
	/** Called when a group is added for this account */
	void slotGroupAdded(KopeteGroup* group);
	
	/** Called when a group is removed for this account */
	void slotGroupRemoved( const QString& groupName, uint groupNumber );
	
	/** 
	 * Called when a group is renamed for this account
	 * @param groupName The new group name
	 * @param groupNumber The group number, shouldn't change
	 */
	void slotGroupRenamed( const QString& groupName, uint groupNumber );
	
	/** Called when the contact list renames a group */
	void slotKopeteGroupRenamed( KopeteGroup *group );
	
	/** Called when the contact list removes a group */
	void slotKopeteGroupRemoved( KopeteGroup *group );
	
	/**
	 * Called when our status on the server has changed
	 * This is called when disconnected too
	 */
	void slotStatusChanged( const KopeteOnlineStatus &newStatus );

	/** 
	 * Called when we get a contact list from the server
	 * The parameter is a qmap with the contact names as keys
	 * the value is another map with keys "name", "group"
	 */
	void slotGotServerBuddyList( AIMBuddyList& buddyList );
	
	/** Called when we've received an IM */
	void slotGotIM( QString message, QString sender, bool isAuto );
	
	/** Called when we have been warned */
	void slotGotWarning(int newlevel, QString warner);
	
	/** Called when we get a request for a direct IM session with @sn */
	void slotGotDirectIMRequest(QString sn);
	
	/** Called when there is no activity for a certain amount of time  */
	void slotIdleTimeout();
	
	/** Called when there is mouse/keyboard activity */
	void slotIdleActivity();
	
	/** Called when preferences are saved */
	void slotPreferencesSaved();
	
	/** Displays an error dialog with the given text */
	void slotError(QString errmsg, int errorCode);
	
protected: // Methods
	/** Adds a contact to a meta contact */
	virtual bool addContactToMetaContact( const QString &contactId, 
										const QString &displayName, 
										KopeteMetaContact *parentContact );

	/** Initializes our actions */
	virtual void initActions();
	/** Initializes our action menu */
	virtual void initActionMenu();
	/** Initializes the engine */
	virtual void initEngine();
	
	/** Sets the user's profile */
	virtual void setUserProfile( QString profile );
	
protected: // Ivars
	/** Our protocol object */
	OscarProtocol *m_protocol;
	/** Flag for remembering the password */
	bool m_rememberPassword;
	/** Our Internal buddy list (from the server) */
	AIMBuddyList *m_internalBuddyList;
	/** Our contact */
	OscarContact *m_myself;
	/** Our OSCAR socket object */
	OscarSocket *m_engine;
	/** Our action menu */
	KActionMenu *m_actionMenu;

	/** Go online action */
	KAction *m_actionGoOnline;
	/** Go offline action */
	KAction *m_actionGoOffline;
	/** Go away action */
	KAction *m_actionGoAway;
	/** Edit user info */
	KAction *m_actionEditInfo;
	/** Show debug dialog action */
	KAction *m_actionShowDebug;
	/** Our away dialog */
	OscarAwayDialog *m_awayDialog;
	
	/** Random new buddy number for the engine */
	int m_randomNewBuddyNum;
	/** Random new group number for the engine */
	int m_randomNewGroupNum;
		
		
};
#endif
