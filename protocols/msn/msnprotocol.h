/***************************************************************************
                    msnprotocol.h  -  Kopete's MSN Plugin
                             -------------------
    Copyright (c) 2002   by Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2002   by Martijn Klingens    <klingens@kde.org>
    Copyright (c) 2002   by Olivier Goffart     <ogoffart@tiscalinet.be>

    Copyright (c) 2002  by the Kopete developers  <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MSNPROTOCOL_H
#define MSNPROTOCOL_H

#include <qmap.h>
//#include <qptrdict.h>
#include <qstringlist.h>

#include "kopeteprotocol.h"
#include "msnsocket.h"

class KAction;
class KActionMenu;

class MSNContact;
class MSNIdentity;
class MSNPreferences;
class MSNNotifySocket;
class MSNSwitchBoardSocket;
class MSNMessageManager;
class KopeteMessageManager;
class KopeteMetaContact;
class KopeteContact;
class KopeteMessage;
class KopeteGroup;

/**
 * @author duncan
 */
class MSNProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	MSNProtocol( QObject *parent, const char *name, const QStringList &args );
	~MSNProtocol();

	/**
	 * SyncMode indicates whether settings differing between client and
	 * server should be propagated to keep them in sync.
	 * SyncToServer   - Ignore the server setting when sent. Instead, push
	 *                  the local setting to the server. Used when changing
	 *                  settings offline.
	 * SyncFromServer - Update locally stored settings with the value sent
	 *                  by the server. Used when connecting to the server if
	 *                  no offline changes are pending to force a sync.
	 * SyncBoth       - Changes are updated both ways. This is truly a
	 *                  'first come, first serve' scenario, which breaks if
	 *                  the 'old' value is sent by one peer before the other
	 *                  end is able to push the new value. An example of this
	 *                  is changing the MSN nickname offline - the server can
	 *                  only be updated after it has sent the old value to
	 *                  the client during connect, destroying the new setting.
	 *                  Once connected this is often the most useful setting.
	 * DontSync       - Do not sync values at all. This is used if settings
	 *                  are overridden locally, but should not be sent to the
	 *                  server, nor should the client update server-pushed
	 *                  values. This can be useful for e.g. contact lists.
	 */
	enum SyncMode
	{
		DontSync       = 0x00,
		SyncToServer   = 0x01,
		SyncFromServer = 0x02,
		SyncBoth       = 0x03
	};


	// Plugin reimplementation
	void init();
	bool unload();

	enum Status
	{
		NLN,    // Online
		BSY,    // Busy
		BRB,    // Be right back
		AWY,    // Away from computer
		PHN,    // On the phone
		LUN,    // Out to lunch
		FLN,    // Offline
		HDN,    // Invisible
		IDL,    // Idle
//		BLO,     // blocked (not used)
		UNK     // Unknown (Not in the contact list)
	};

	enum List
	{
		FL,    // forward
		AL,    // allow
		BL,    // blocked
		RL     // reverse
	};

	virtual void deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	virtual AddContactPage *createAddContactWidget( QWidget *parent );
	virtual EditIdentityWidget *createEditIdentityWidget(KopeteIdentity *identity, QWidget *parent);
	virtual KopeteIdentity *createNewIdentity(const QString &identityId);

	virtual bool isConnected() const;
	virtual void setAway();
	virtual void setAvailable();
	virtual bool isAway() const;

	bool addContactToMetaContact( const QString &contactId, const QString &displayName,
		KopeteMetaContact *parentContact );

	void addGroup( const QString &groupName,
		const QString &contactToAdd = QString::null );

	KopeteContact *myself() const;

	/**
	 * Convert string-like status to Status enum
	 */
	static Status convertStatus( QString status );
	Status status() const;

//	QStringList groups() const;

	QString publicName() const { return m_publicName; }


	/**
	 * Returns a set of action items for the chatWindows
	 */
	KActionCollection * customChatActions(KopeteMessageManager * );

	MSNNotifySocket *notifySocket();
	
	virtual KActionMenu* protocolActions();
	virtual const QString protocolIcon();

public slots:
	virtual void connect();
	virtual void disconnect();

	/**
	 * Start a new chat session: the result is an XFR command, see above
	 */
	void slotStartChatSession( QString handle );
	
		void slotNotifySocketStatusChanged( MSNSocket::OnlineStatus );

private slots:
	// Block a Contact
	void slotBlockContact( QString passport ) ;

	void slotSyncContactList();


	void slotAddContact( const QString &userName );
	/**
	 * A kopetegroup is renamed, just call renameGroup
	 **/
	void slotKopeteGroupRenamed(KopeteGroup *g);

	/**
	 * A kopetegroup is removed, remove the group in the server
	 **/
	void slotKopeteGroupRemoved( KopeteGroup* );

	/**
	 * The group has successful renamed in the server
	 * groupName: is new new group name
	 */
	void slotGroupRenamed( QString groupName, uint group );
	/**
	 * A new group was created on the server (or recieved durring an LSG command)
	 */
	void slotGroupAdded( QString groupName, uint groupNumber );
	/**
	 * Group was removed from the list
	 */
	void slotGroupRemoved( uint group );

	/**
	 * Contact was removed from the list
	 */
	void slotContactRemoved(QString handle, QString list, uint serial,
		uint group );

	void slotContactAdded(QString handle, QString publicName, QString list,
		uint serial, uint group );

	void slotContactListed( QString handle, QString publicName, QString group, QString list );

	/**
	 * Incoming RING command: connect to the Switchboard server and send
	 * the startChat signal
	 */
	void slotCreateChat( QString sessionID, QString address, QString auth,
		QString handle, QString publicName );

	/**
	 * Incoming XFR command: this is an result from
	 * slotStartChatSession(handle)
	 * connect to the switchboard server and sen startChat signal
	 */
	void slotCreateChat( QString address, QString auth);

	void slotPreferencesSaved();

private:


	bool mIsConnected;

	MSNPreferences *mPrefs;

	// Actions we use
	/*KAction* actionGoOnline;
	KAction* actionGoOffline;
	KAction* actionGoAway;
	KAction* actionGoBusy;
	KAction* actionGoBeRightBack;
	KAction* actionGoOnThePhone;
	KAction* actionGoOutToLunch;
	KAction* actionGoInvisible;
	KAction* m_renameAction;
	KAction* m_startChatAction;
	KAction* m_openInboxAction;

	KActionMenu *m_debugMenu;
	KAction *m_debugRawCommand;

	KActionMenu *actionStatusMenu;
	KAction* actionConnect;
	KAction* actionDisconnect;
	KAction* actionPrefs;
	KAction* actionUnload;*/
	int m_menuTitleId;

	QMap<unsigned int, KopeteGroup*> m_groupList;

	static MSNProtocol *s_protocol;
	Status m_status;
	Status m_connectstatus;
	QString m_msnId;
	QString m_password;
	QString m_publicName;
	SyncMode m_publicNameSyncMode;
	bool m_publicNameSyncNeeded;
	QString m_msgHandle;
	KopeteMetaContact *m_addWizard_metaContact;

	MSNIdentity *m_identity;
//	QPtrDict<MSNSwitchBoardSocket> m_switchBoardSockets;

	/**
	 * Mapping of meta contacts to MSN contacts
	 * I removed this: I think it is not needed.
	 * that can provoque problem with several MSN contact in the same protocol
	 */
	//QPtrDict<MSNContact> m_metaContacts;

	QStringList m_allowList;
	QStringList m_blockList;

	QValueList< QPair<QString,QString> > tmp_addToNewGroup;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

