/***************************************************************************
                          msnprotocol.h  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
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

	static const MSNProtocol *protocol();

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
		BLO     // blocked
	};

	enum List
	{
		FL,    // forward
		AL,    // allow
		BL,    // blocked
		RL     // reverse
	};

	// KopeteProtocol reimplementation
	virtual KopeteContact* createContact( KopeteMetaContact *parent, const QString &serializedData );
	virtual QString protocolIcon() const;
	virtual AddContactPage *createAddContactWidget( QWidget *parent );
	virtual void Connect();
	virtual void Disconnect();
	virtual bool isConnected() const;
	virtual void setAway();
	virtual void setAvailable();
	virtual bool isAway() const;

	void addContact( const QString &userID );
	void removeContact( const MSNContact *c ) const;
	void removeFromGroup( const MSNContact *c, const QString &group ) const;
	void moveContact( const MSNContact *c, const QString &oldGroup,
		const QString &newGroup ) const;
	void copyContact( const MSNContact *c, const QString &newGroup ) const;

	void addGroup( const QString &groupName );
	void renameGroup( const QString &oldGroup, const QString &newGroup );
	void removeGroup( const QString &groupName );

	KopeteContact *myself() const;

	/**
	 * Convert string-like status to Status enum
	 * FIXME: should be made private again when possible
	 */
	static Status convertStatus( QString status );
	Status status() const;

	QStringList groups() const;

	void setSilent( bool s ) { m_silent = s; }
	bool isSilent() { return m_silent; }

	QString msnId() const { return m_msnId; }

	QString publicName() const { return m_publicName; }
	/**
	 * change the publicName to this new name
	 */
	void setPublicName( const QString &name );

	/**
	 * Allow this blocked contact
	 */
	void contactUnBlock( QString handle ) const;

	const ContactList& contacts() const { return m_contacts; }

public slots:
	void slotSyncContactList();

	// To go online we need to check if connected
	void slotGoOnline();
	void slotGoOffline();
	void slotGoAway();
	void slotIconRightClicked( const QPoint );

	void slotOnlineStatusChanged( MSNSocket::OnlineStatus );

	void slotStateChanged( QString status );

	/**
	 * The publicName has successful changed
	 * This is an anwser from setMyPublicName
	 */
	void slotPublicNameChanged(QString handle, QString publicName);

	// Add a Contact
	void slotAddContact( QString );
	// Block a Contact
	void slotBlockContact(QString) const;

signals:
	void protocolUnloading();
	void settingsChanged( void );

private slots:
	/**
	 * We received a message
	 */
	void slotMessageReceived( const KopeteMessage &msg );
	void slotMessageSent( const KopeteMessage& msg );

	/**
	 * Open the chat window for a specific user
	 */
    void slotExecute( QString handle );

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
	void slotContactRemoved(QString handle, QString list, uint serial, uint group );
	void slotContactStatus( QString handle, QString publicName, QString status );
	void slotContactAdded(QString handle, QString publicName, QString list, uint serial, uint group );

	void slotContactList(QString handle, QString publicName, QString group, QString list );
	void slotContactStatusChanged( const QString &msnId,
		const QString &publicName, MSNProtocol::Status status );
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
	 * Start a new chat session: the result is an XFR command, see above
     * FIXME: The chat session needs an update
	 */
	void slotStartChatSession( QString handle );

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
	 * The SwitchBoard socket was closed, so we remove it from the list.
	 */
	 void slotSwitchBoardClosed( MSNSwitchBoardSocket *switchboard );

private:
	/**
	 * Get group by name.
	 * Returns -1 if the group was not found.
	 */
	int groupNumber( const QString &groupName ) const;

	/**
	 * Add contact to contact list and maintain a list of currently active
	 * contacts. This way MSNContact doesn't need to do 'delete this' every
	 * time, which is very dangerous
	 */
	void addToContactList( MSNContact *c, const QString &group );

	void initIcons();
	void initActions();

	bool mIsConnected;

	StatusBarIcon *statusBarIcon;

	QPixmap onlineIcon;
	QPixmap offlineIcon;
	QPixmap awayIcon;
	QPixmap naIcon;
	QMovie connectingIcon;

	// Actions we use
	KAction* actionGoOnline;
	KAction* actionGoOffline;
	KAction* actionGoAway;
	KAction* m_renameAction;

	KActionMenu *m_debugMenu;
	KAction *m_debugRawCommand;

	KActionMenu *actionStatusMenu;
	KAction* actionConnect;
	KAction* actionDisconnect;
	KAction* actionPrefs;
	KAction* actionUnload;
	int m_menuTitleId;

	MSNPreferences *mPrefs;

	ContactList m_contacts;

	QMap<uint, QString> m_groupList;

	static const MSNProtocol *s_protocol;
	Status m_status;
	uint m_serial;
	bool m_silent;
	QString m_msnId;
	QString m_password;
	QString m_publicName;
	SyncMode m_publicNameSyncMode;
	bool m_publicNameSyncNeeded;
	QString m_msgHandle;

	MSNNotifySocket *m_notifySocket;
	KopeteContact *m_myself;

	MSNIdentity *m_identity;
	QPtrDict<MSNSwitchBoardSocket> m_switchBoardSockets;

	QStringList m_allowList;
	QStringList m_blockList;
};

#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

