/*
    msnaccount.h - Manages a single MSN account

    Copyright (c) 2003-2005 by Olivier Goffart       <ogoffart@ kde.org>
    Kopete    (c) 2003-2005 by The Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNACCOUNT_H
#define MSNACCOUNT_H

#include <qobject.h>

#include "kopetepasswordedaccount.h"

#include "msnprotocol.h"

class KAction;
class KActionMenu;

class MSNNotifySocket;
class MSNContact;

/**
 * MSNAccount encapsulates everything that is account-based, as opposed to
 * protocol based. This basically means sockets, current status, and account
 * info are stored in the account, whereas the protocol is only the
 * 'manager' class that creates and manages accounts.
 */
class MSNAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT

public:
	MSNAccount( MSNProtocol *parent, const QString &accountID, const char *name = 0L );

	virtual void setAway( bool away, const QString & );

	/*
	 * return the menu for this account
	 */
	virtual KActionMenu* actionMenu();

	//------ internal functions
	/**
	 * change the publicName to this new name
	 */
	void setPublicName( const QString &name );

	/**
	 * Returns the address of the MSN server
	 */
	QString serverName();

	/**
	 * Returns the address of the MSN server port
	 */
	uint serverPort();

	MSNNotifySocket *notifySocket();

	QString awayReason()
		{ return m_awayReason; }

	/**
	 * return true if we are able to send mail, or to open hotmail inbox
	 */
	bool isHotmail() const;


	/**
	 * return the <msnobj> tag of the display picture
	 */
	QString pictureObject();

	/**
	 * reset the <msnobj>.  This method should be called if the displayimage has changed
	 * If we are actualy connected, it will imediatly update the <msgobj> on the server, exepted
	 * if @param silent is set to true
	 */
	void resetPictureObject(bool silent=false);

public slots:
	virtual void connectWithPassword( const QString &password ) ;
	virtual void disconnect() ;
	virtual void setOnlineStatus( const Kopete::OnlineStatus &status , const QString &reason = QString::null);

	/**
	 * Ask to the account to create a new chat session
	 */
	void slotStartChatSession( const QString& handle );

protected:
	virtual bool createContact( const QString &contactId, Kopete::MetaContact *parentContact );


private slots:
	// Actions related
	void slotStartChat();
	void slotOpenInbox();
	void slotChangePublicName();

//#if !defined NDEBUG //(Stupid moc which don't see when he don't need to slot this slot)
	/**
	 * Show simple debugging aid
	 */
	void slotDebugRawCommand();
//#endif

	// notifySocket related
	void slotStatusChanged( const Kopete::OnlineStatus &status );
	void slotNotifySocketClosed();
	void slotNotifySocketStatusChanged( MSNSocket::OnlineStatus status );
	void slotPublicNameChanged(const QString& publicName);
	void slotContactRemoved(const QString& handle, const QString& list,  uint group );
	void slotContactAdded(const QString& handle, const QString& publicName, const QString& list,  uint group );
	void slotContactListed( const QString& handle, const QString& publicName, uint lists, const QString& group );
	void slotNewContactList();
	/**
	 * The group has successful renamed in the server
	 * groupName: is new new group name
	 */
	void slotGroupRenamed( const QString& groupName, uint group );
	/**
	 * A new group was created on the server (or received durring an LSG command)
	 */
	void slotGroupAdded( const QString& groupName, uint groupNumber );
	/**
	 * Group was removed from the server
	 */
	void slotGroupRemoved( uint group );
	/**
	 * Incoming RING command: connect to the Switchboard server and send
	 * the startChat signal
	 */
	void slotCreateChat( const QString& sessionID, const QString& address, const QString& auth,
		const QString& handle, const QString& publicName );
	/**
	 * Incoming XFR command: this is an result from
	 * slotStartChatSession(handle)
	 * connect to the switchboard server and sen startChat signal
	 */
	void slotCreateChat( const QString& address, const QString& auth);


	// ui related
	/**
	 * A kopetegroup is renamed, rename group on the server
	 */
	void slotKopeteGroupRenamed( Kopete::Group *g );

	/**
	 * A kopetegroup is removed, remove the group in the server
	 **/
	void slotKopeteGroupRemoved( Kopete::Group* );

	/**
	 * add contact ui
	 */
	void slotContactAddedNotifyDialogClosed( const QString &handle);

	/**
	 * When the dispatch server sends us the notification server to use.
	 */
	void createNotificationServer( const QString &host, uint port );


private:
	MSNNotifySocket *m_notifySocket;
	KAction *m_openInboxAction;
	KAction *m_startChatAction;
	KAction *m_changeDNAction;

	// status which will be using for connecting
	Kopete::OnlineStatus m_connectstatus;

	QStringList m_msgHandle;

	bool m_newContactList;


	/**
	 * Add the contact on the server in the given groups.
	 * this is a helper function called bu createContact and slotStatusChanged
	 */
	void addContactServerside(const QString &contactId, QPtrList<Kopete::Group> groupList);

public: //FIXME: should be private
	QMap<unsigned int, Kopete::Group*> m_groupList;

	void addGroup( const QString &groupName, const QString &contactToAdd = QString::null );

private:

	// server data
	QStringList m_allowList;
	QStringList m_blockList;
	QStringList m_reverseList;

	Kopete::MetaContact *m_addWizard_metaContact;
	QMap< QString, QStringList > tmp_addToNewGroup;

	QString m_awayReason;

	QString m_pictureObj; //a cache of the <msnobj>

	//this is the translation between old to new groups id when syncing from server.
	QMap<unsigned int, Kopete::Group*> m_oldGroupList;

	/**
	 * I need the password in createNotificationServer.
	 * but i can't ask it there with password() because a nested loop will provoque crash
	 * at this place.   so i'm forced to keep it here.
	 * I would like an API to request the password WITHOUT askling it.
	 */
	QString m_password;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

