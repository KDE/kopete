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

	// Plugin reimplementation
	void init();
	bool unload();

	// KopeteProtocol reimplementation
	virtual QString protocolIcon() const;
	virtual AddContactPage *createAddContactWidget( QWidget *parent );
	virtual void Connect();
	virtual void Disconnect();
	virtual bool isConnected() const;
	virtual void setAway();
	virtual void setAvailable();
	virtual bool isAway() const;

	void addContact( const QString &userID ) const;
	void removeContact( const QString &userID ) const;
	void removeFromGroup( const QString &userID, const QString &group ) const;
	void moveContact( const MSNContact *c, const QString &newGroup ) const;
	void copyContact( const QString &userID, const QString &newGroup ) const;

	int contactStatus( const QString &handle ) const;
	QString publicName( const QString &handle ) const;

	QStringList groups() const;

	KMSNService* msnService() const;

public slots:
	void slotMessageDialogClosing( QString );
	void slotIncomingChat( KMSNChatService *, QString );

	void slotSyncContactList();

	void slotConnected();
	void slotDisconnected();

	// To go online we need to check if connected
	void slotGoOnline();
	void slotGoOffline();
	void slotGoAway();
	void slotIconRightClicked( const QPoint );

	void slotConnectedToMSN( bool c );
	void slotConnecting();

	void slotContactAdded( QString, QString, QString );
	void slotUserStateChange( QString, QString, int ) const;
	void slotStateChanged( uint ) const;
	void slotUserSetOffline( QString ) const;
	void slotInitContacts( QString, QString, QString );
	void slotNewUserFound( QString );

	// Someone tries to talk with us
	void slotNewUser( QString );
	// Ask user to auth the new contact
	void slotAuthenticate( QString );
	// Add a Contact
	void slotAddContact(QString) const;
	// Block a Contact
	void slotBlockContact(QString) const;

	// Group slots
	void slotGoURL( const QString ) const;

signals:
	void userStateChange( QString, QString, int );
	void protocolUnloading();
	void settingsChanged( void );

	// Propagated from the MSN Service, in order to hide it from external
	// classes:
	void updateContact( QString handle, uint status );
	void contactRemoved( QString handle, QString groupName );
	void connectedToService( bool connected );

private:
	void initIcons();
	void initActions();

	bool mIsConnected;

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
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

