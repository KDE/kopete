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
#include <qptrlist.h>

#include "kopeteprotocol.h"

class QLabel;

class KAction;
class KActionMenu;
class KDialogBase;
class KPopupMenu;
class KSimpleConfig;
class KURLLabel;

class KMSNChatService;
class KMSNService;
class KMSNServiceSocket;
class MSNContact;
class MSNMessageDialog;
class MSNPreferences;
class StatusBarIcon;

/**
 * @author duncan
 */
class MSNProtocol : public QObject, public KopeteProtocol
{
	Q_OBJECT

public:
	MSNProtocol();
	~MSNProtocol();

	static const MSNProtocol *protocol();


	/**
	 * Get group by number and vice versa.
	 * Returns -1 resp QString::null if the search term was not found
	 *
	 * FIXME: Probably make private when KMSNService is completely ported
	 */
	int groupNumber( const QString &groupName ) const;
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

	/**
	 * Convert string-like status to Status enum
	 * FIXME: should be made private again when possible
	 */
	Status convertStatus( QString status ) const;
	Status status() const;

	QString publicName( const QString &handle ) const;

	QStringList groups() const;
	QStringList groupContacts( const QString &group ) const;

	KMSNService* msnService() const;

	void setSilent( bool s ) { m_silent = s; }
	bool isSilent() { return m_silent; }

	QString msnId() const { return m_msnId; }

	QString publicName() const { return m_publicName; }
	/**
	 * change the publicName to this new name
	 */
	void setPublicName( const QString &name );

public slots:
	void slotMessageDialogClosing( QString );
	void slotIncomingChat( KMSNChatService *, QString );

	void slotSyncContactList();

	// To go online we need to check if connected
	void slotGoOnline();
	void slotGoOffline();
	void slotGoAway();
	void slotIconRightClicked( const QPoint );

	void slotConnectedToMSN( bool c );

	void slotUserStateChange( QString, QString, int ) const;
	void slotStateChanged( QString status );
	void slotUserSetOffline( QString ) const;
	void slotInitContacts( QString, QString, QString );
	void slotNewUserFound( QString );

	/**
	 * The publicName has successful changed
	 * This is an anwser from setMyPublicName
	 */
	void slotPublicNameChanged(QString handle, QString publicName);

	// Someone tries to talk with us
	void slotNewUser( QString );
	// Add a Contact
	void slotAddContact( QString );
	// Block a Contact
	void slotBlockContact(QString) const;

	// Group slots
	void slotGoURL( const QString ) const;

signals:
	void userStateChange( QString, QString, int );
	void protocolUnloading();
	void settingsChanged( void );

private slots:
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
	 * MSN has send the current publicName
	 */
	void slotPublicNameReceived(QString publicName);

	/**
	 * Contact was removed from the list
	 */
	void slotContactRemoved(QString handle, QString list, uint serial, uint group );
	void slotContactStatus( QString handle, QString publicName, QString status );
	void slotContactAdded(QString handle, QString publicName, QString list, uint serial, uint group );

	void slotContactList(QString handle, QString publicName, QString group, QString list );
	void slotContactStatusChanged( QString handle, QString publicName, QString status );
	void slotStatusChanged( QString status );

private:
	/**
	 * Add contact to contact list and maintain a list of currently active
	 * contacts. This way MSNContact doesn't need to do 'delete this' every
	 * time, which is very dangerous
	 */
	void addToContactList( MSNContact *c, const QString &group );

	void initIcons();
	void initActions();

	bool mIsConnected;

	/**
	 * Return the service socket. Use this and be safe. Use the static
	 * KMSNServiceSocket::kmsnServiceSocket() directly and prepare to be
	 * shot - Martijn
	 */
	KMSNServiceSocket *serviceSocket() const;

	// The MSN Engine
	KMSNService *m_msnService;

	StatusBarIcon *statusBarIcon;

	QPixmap onlineIcon;
	QPixmap offlineIcon;
	QPixmap awayIcon;
	QPixmap naIcon;
	QMovie connectingIcon;

	// The main msn popup
	KPopupMenu *popup;

	// Actions we use
	KAction* actionGoOnline;
	KAction* actionGoOffline;
	KAction* actionGoAway;
	//KSelectAction* actionStatus;

	KActionMenu *actionStatusMenu;
	KAction* actionConnect;
	KAction* actionDisconnect;
	KAction* actionPrefs;
	KAction* actionUnload;

	QPtrList<MSNMessageDialog> mChatWindows;

	// Files to store contacts locally
	KSimpleConfig *mContactsFile;
	KSimpleConfig *mGroupsFile;

	MSNPreferences *mPrefs;
	KDialogBase *mEmptyConfig;
	KURLLabel *mPassportURL;
	QLabel *mEmptyMsg;
//	QList<MSNContactStruct> contactList;
//	QList<MSNGroupStruct> groupList;

	QMap<QString, MSNContact*> m_contacts;
	
	QMap<uint, QString> m_groupList;

	static const MSNProtocol *s_protocol;
	Status m_status;
	uint m_serial;
	bool m_silent;
	QString m_msnId;
	QString m_password;
	QString m_publicName;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

