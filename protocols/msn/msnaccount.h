/*
    msnaccount.h - Manages a single MSN identity

    Copyright (c) 2003 by Olivier Goffart       <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by The Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNIDENTITY_H
#define MSNIDENTITY_H

#include <qobject.h>

#include "kopeteaccount.h"

#include "msnprotocol.h"

class KAction;
class KActionMenu;

class MSNNotifySocket;
class MSNContact;

/**
 * MSNIdentity encapsulates everything that is identity-based, as opposed to
 * protocol based. This basically means sockets, current status, and account
 * info are stored in the identity, whereas the protocol is only the
 * 'manager' class that creates and manages identities.
 */
class MSNIdentity : public KopeteIdentity
{
	Q_OBJECT

public:
	MSNIdentity( MSNProtocol *parent, const QString &identityID, const char *name = 0L );
	~MSNIdentity();

	virtual void setAway( bool away );
	virtual KopeteContact* myself() const;

	/*
	 * return the menu for this identity
	 */
	virtual KActionMenu* actionMenu();

	//------ internal functions
	/**
	 * change the publicName to this new name
	 */
	void setPublicName( const QString &name );

	MSNNotifySocket *notifySocket();

	// FIXME: Make generic - Martijn
	void setOnlineStatus( const KopeteOnlineStatus &status );

public slots:
	virtual void connect() ;
	virtual void disconnect() ;

	/**
	 * Ask to the identity to create a new chat session
	 */
	void slotStartChatSession( QString handle );

protected:
	virtual bool addContactToMetaContact( const QString &contactId, const QString &displayName, KopeteMetaContact *parentContact );

protected slots:
	virtual void loaded();

private slots:
	// Actions related
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
	void slotChangePublicName();

//#if !defined NDEBUG //(Stupid moc which don't see when he don't need to slot this slot)
	/**
	 * Show simple debugging aid
	 */
	void slotDebugRawCommand();
//#endif

	// notifySocket related
	void slotStatusChanged( const KopeteOnlineStatus &status );
	void slotNotifySocketClosed( int state );
	void slotNotifySocketStatusChanged( MSNSocket::OnlineStatus status );
	void slotPublicNameChanged(QString publicName);
	void slotContactRemoved(QString handle, QString list, uint serial, uint group );
	void slotContactAdded(QString handle, QString publicName, QString list, uint serial, uint group );
	void slotContactListed( QString handle, QString publicName, QString group, QString list );
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
	 * Group was removed from the server
	 */
	void slotGroupRemoved( uint group );
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

	// ui related
	/**
	 * A kopetegroup is renamed, rename group on the server
	 */
	void slotKopeteGroupRenamed( KopeteGroup *g );

	/**
	 * A kopetegroup is removed, remove the group in the server
	 **/
	void slotKopeteGroupRemoved( KopeteGroup* );

	/********************/	
	/** add contact ui **/
	void slotBlockContact( QString passport ) ;
	void slotAddContact( const QString &userName );

private:
	KActionMenu *m_actionMenu;
	KAction *m_openInboxAction;
	int m_menuTitleId;

	MSNNotifySocket *m_notifySocket;
	MSNContact *m_myself;

	// status which will be using for connecting
	KopeteOnlineStatus m_connectstatus;

	QString m_msgHandle;

public: //FIXME: should be private
	QValueList< QPair<QString,QString> > tmp_addToNewGroup;

	void addGroup( const QString &groupName, const QString &contactToAdd = QString::null );

private:
	// server data
	QStringList m_allowList;
	QStringList m_blockList;

	QMap<unsigned int, KopeteGroup*> m_groupList;
	KopeteMetaContact *m_addWizard_metaContact;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

