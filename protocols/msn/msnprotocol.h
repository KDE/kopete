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
#include <qmovie.h>
#include <qpixmap.h>
#include <qptrdict.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include "kopeteprotocol.h"
#include "kopetecontact.h"
#include "msnsocket.h"

class KAction;
class KActionMenu;

class MSNSwitchBoardSocket;
class MSNNotifySocket;
class KopeteContact;
class KopeteMetaContact;
class KopeteMessage;
class KopeteMessageManager;
class MSNContact;
class MSNIdentity;
class MSNPreferences;
class StatusBarIcon;

/**
 * @author duncan
 */
class MSNProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	MSNProtocol( QObject *parent, const char *name, const QStringList &args );
	~MSNProtocol();

	static MSNProtocol *protocol();

	typedef QMap<QString, MSNContact*> ContactList;

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

	/**
	 * Get group by number. The reverse method is used only internally and
	 * private.
	 * Returns QString::null if the group was not found
	 */
	QString groupName( uint number ) const;

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

	virtual bool serialize( KopeteMetaContact *metaContact,
		QStringList &strList ) const;
	virtual void deserialize( KopeteMetaContact *metaContact,
		const QStringList &strList );

	virtual QString protocolIcon() const;
	virtual AddContactPage *createAddContactWidget( QWidget *parent );
	virtual void Connect();
	virtual void Disconnect();
	virtual bool isConnected() const;
	virtual void setAway();
	virtual void setAvailable();
	virtual bool isAway() const;

	void addContact( const QString &userID , KopeteMetaContact *m=0L, const QString &group=QString::null);

	void addGroup( const QString &groupName , const QString& contactToAdd=QString::null );
	void renameGroup( const QString &oldGroup, const QString &newGroup );
	void removeGroup( const QString &groupName );

	KopeteContact *myself() const;

	/**
	 * Convert string-like status to Status enum
	 */
	static Status convertStatus( QString status );
	Status status() const;

	QStringList groups() const;

	QString publicName() const { return m_publicName; }
	/**
	 * change the publicName to this new name
	 */
	void setPublicName( const QString &name );


	ContactList& contacts() { return m_contacts; }

	MSNContact *contact( const QString &handle );

  void setStatus(Status);
  /** 
	 * Returns a set of action items for the chatWindows
	 */
  KActionCollection * customChatActions(KopeteMessageManager * );

	MSNNotifySocket *notifySocket() { return m_notifySocket; };

	/**
	 * Get group by name.
	 * Returns -1 if the group was not found.
	 */
	int groupNumber( const QString &groupName ) const;




signals:
//	void protocolUnloading();

public slots:
	/**
	 * Start a new chat session: the result is an XFR command, see above
	 */
	void slotStartChatSession( QString handle );

private slots:
	/**
	 * The publicName has successful changed
	 * This is an anwser from setMyPublicName
	 */
	void slotPublicNameChanged(QString publicName);

	// Add a Contact
	void slotAddContact( QString );
	// Block a Contact
	void slotBlockContact( QString passport ) const;

	void slotSyncContactList();

	void slotIconRightClicked( const QPoint& );

	void slotNotifySocketStatusChanged( MSNSocket::OnlineStatus );

	void slotGoOnline();
	void slotGoOffline();
	void slotGoAway();
	void slotGoBusy();
	void slotGoBeRightBack();
	void slotGoOnThePhone();
	void slotGoOutToLunch();
	void slotGoInvisible();

	void slotStartChat();

	void slotOpenInbox();

	/**
	 * The group has successful renamed
	 * groupName: is new new group name
	 * search the old groupName in the groupList with (uint group)
	 */
	void slotGroupRenamed( QString groupName, uint serial, uint group );
	/**
	 * A new group was created on the server
	 */
	void slotGroupAdded( QString groupName, uint serial, uint group );
	/**
	 * Group was removed from the list
	 */
	void slotGroupRemoved( uint serial, uint group );
	/**
	 * Group name received during an LSG ( 'list groups' ) command
	 */
	void slotGroupListed( QString groupName, uint group );

	/**
	 * Contact was removed from the list
	 */
	void slotContactRemoved(QString handle, QString list, uint serial,
		uint group );

	void slotContactAdded(QString handle, QString publicName, QString list,
		uint serial, uint group );

	void slotContactList(QString handle, QString publicName, QString group,
		QString list );
	void slotStatusChanged( QString status );

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

	/**
	 * Set our public name
	 * FIXME: This should be 'setPublicName( foo )' instead of course with
	 *        the GUI being defined elsewhere. Time to think about more
	 *        API fixes :(
	 */
	void slotChangePublicName();

	/**
	 * Show simple debugging aid
	 */
	void slotDebugRawCommand();

	/**
	 * The Notify socket might have closed unexpectedly...
	 */
	 void slotNotifySocketClosed( int state );

	/**
	 * An MSN contact got deleted. Clean up the necessary data
	 */
	void slotContactDestroyed( KopeteContact *c );

	void slotPreferencesSaved();
private:

	/**
	 * Add contact to contact list and maintain a list of currently active
	 * contacts. This way MSNContact doesn't need to do 'delete this' every
	 * time, which is very dangerous
	 */


	void initIcons();
	void initActions();

	bool mIsConnected;

	StatusBarIcon *statusBarIcon;

	MSNPreferences *mPrefs;

	QPixmap onlineIcon;
	QPixmap offlineIcon;
	QPixmap awayIcon;
	QPixmap naIcon;
	QMovie connectingIcon;

	// Actions we use
	KAction* actionGoOnline;
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
	KAction* actionUnload;
	int m_menuTitleId;

	ContactList m_contacts;

	QMap<uint, QString> m_groupList;

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

	MSNNotifySocket *m_notifySocket;
	MSNContact *m_myself;

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

