/*
    msnnotifysocket.h - Notify Socket for the MSN Protocol

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    Portions taken from
    KMerlin   (c) 2001      by Olaf Lueg              <olueg@olsd.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNNOTIFYSOCKET_H
#define MSNNOTIFYSOCKET_H

#include "msnauthsocket.h"
#include "msnprotocol.h"

class MSNDispatchSocket;
class MSNAccount;

/**
 * @author Olaf Lueg
 */
class MSNNotifySocket : public MSNAuthSocket
{
	Q_OBJECT

public:
	MSNNotifySocket( MSNAccount* account, const QString &msnId );
	~MSNNotifySocket();

	void connect( const QString &password );
	virtual void disconnect();

	void setStatus( const KopeteOnlineStatus &status );
	void addContact( const QString &handle, const QString& pulicName, uint group , int list );
	void removeContact( const QString &handle, uint group, int list );

	void addGroup( const QString& groupName );
	void removeGroup( uint group );
	void renameGroup(const QString& groupName, uint group);

	void changePublicName( const QString &publicName , const QString &handle=QString::null );

	void createChatSession();

public slots:
	void slotOpenInbox();

signals:
	void newContactList();
	void contactList(const QString&, const QString&, const QString&, const QString&);
//	void contactList(const QString&, const QString&, uint);
	void contactStatus(const QString&, const QString&, const QString& );
	void contactAdded(const QString&, const QString&, const QString&, uint);
	void contactRemoved(const QString&, const QString&, uint);

	void groupListed(const QString&, uint group);
	void groupAdded( const QString&, uint group);
	void groupRenamed( const QString&, uint group);
	void groupRemoved( uint );

	void invitedToChat(const QString&, const QString&, const QString&, const QString&, const QString& );
	void startChat( const QString&, const QString& );

	void publicNameChanged( const QString& );
	void statusChanged( const KopeteOnlineStatus &newStatus );
	
	void hotmailSeted(bool) ;

protected:
	/**
	 * Handle an MSN command response line.
	 */
	virtual void parseCommand( const QString &cmd, uint id,
		const QString &data );

	/**
	 * Handle an MSN error condition.
	 * This reimplementation handles most of the other MSN error codes.
	 * FIXME: It *should* do this, but currently implements only a very
	 * limited subset...
	 */
	virtual void handleError( uint code, uint id );

private slots:
	void slotReceivedServer( const QString &server, uint port );

	/**
	 * We received a message from the server, which is sent as raw data,
	 * instead of cr/lf line-based text.
	 */
	void slotReadMessage( const QString &msg );

	void slotDispatchClosed();

	/**
	 * Send a keepalive to the server to avoid idle connections to cause
	 * MSN closing the connection
	 */
	void slotSendKeepAlive();

	/**
	 * Reset the keepalive time after the socket has sent a command.
	 */
	void slotResetKeepAlive();

private:
	/**
	 * Convert the MSN status strings to a KopeteOnlineStatus
	 */
	KopeteOnlineStatus convertOnlineStatus( const QString &statusString );

	MSNAccount *m_account;

	unsigned int mailCount;
	QString m_password;

	KopeteOnlineStatus m_newstatus;

	/**
	 * Convert an entry of the Status enum back to a string
	 */
	QString statusToString( const KopeteOnlineStatus &status ) const;

	MSNDispatchSocket *m_dispatchSocket;
	bool dispatchOK;

	//know the last handle used
	QString m_tmpLastHandle;

	//for hotmail inbox opening
	bool m_isHotmailAccount;
	QString m_MSPAuth;
	QString m_kv;
	QString m_sid;
	QString m_hotmailRequest;

	QTimer *m_keepaliveTimer;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

