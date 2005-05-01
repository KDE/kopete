/*
    msnnotifysocket.h - Notify Socket for the MSN Protocol

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

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

#include "msnsocket.h"
#include "msnprotocol.h"

class MSNDispatchSocket;
class MSNAccount;
class KTempFile;

#include <kio/job.h>


/**
 * @author Olaf Lueg
 * @author Olivier Goffart
 */
class MSNNotifySocket : public MSNSocket
{
	Q_OBJECT

public:
	MSNNotifySocket( MSNAccount* account, const QString &msnId, const QString &password );
	~MSNNotifySocket();

	virtual void disconnect();

	void setStatus( const Kopete::OnlineStatus &status );
	void addContact( const QString &handle, const QString& pulicName, uint group , int list );
	void removeContact( const QString &handle, uint group, int list );

	void addGroup( const QString& groupName );
	void removeGroup( uint group );
	void renameGroup(const QString& groupName, uint group);

	void changePublicName(  QString publicName , const QString &handle=QString::null );

	void changePhoneNumber( const QString &key, const QString &data );

	void createChatSession();

	void sendMail(const QString &email);

	/**
	 * this should return a  Kopete::Account::DisconnectReason value
	 */
	int  disconnectReason() { return m_disconnectReason; }

public slots:
	void slotOpenInbox();

signals:
	void newContactList();
	void contactList(const QString& handle, const QString& displayname, uint lists, const QString& groups);
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
	void statusChanged( const Kopete::OnlineStatus &newStatus );

	void hotmailSeted(bool) ;


	/**
	 * When the dispatch server sends us the notification server to use, this
	 * signal is emitted. After this the socket is automatically closed.
	 */
	void receivedNotificationServer( const QString &host, uint port );


protected:
	/**
	 * Handle an MSN command response line.
	 */
	virtual void parseCommand( const QString &cmd, uint id,
		const QString &data );

	/**
	 * Handle an MSN error condition.
	 * This reimplementation handles most of the other MSN error codes.
	 */
	virtual void handleError( uint code, uint id );

	/**
	 * This reimplementation sets up the negotiating with the server and
	 * suppresses the change of the status to online until the handshake
	 * is complete.
	 */
	virtual void doneConnect();


private slots:
	/**
	 * We received a message from the server, which is sent as raw data,
	 * instead of cr/lf line-based text.
	 */
	void slotReadMessage( const QString &msg );

	/**
	 * Send a keepalive to the server to avoid idle connections to cause
	 * MSN closing the connection
	 */
	void slotSendKeepAlive();


	void slotAuthJobDataReceived ( KIO::Job *, const QByteArray &data);
	void slotAuthJobDone ( KIO::Job *);


private:
	/**
	 * Convert the MSN status strings to a Kopete::OnlineStatus
	 */
	Kopete::OnlineStatus convertOnlineStatus( const QString &statusString );

	MSNAccount *m_account;
	QString m_password;

	unsigned int mailCount;

	Kopete::OnlineStatus m_newstatus;

	/**
	 * Convert an entry of the Status enum back to a string
	 */
	QString statusToString( const Kopete::OnlineStatus &status ) const;

	//know the last handle used
	QString m_tmpLastHandle;
	QMap <unsigned int,QString> m_tmpHandles;

	//for hotmail inbox opening
	bool m_isHotmailAccount;
	QString m_MSPAuth;
	QString m_kv;
	QString m_sid;
	QString m_loginTime;
	QString m_authData;

	QTimer *m_keepaliveTimer;

	bool m_ping;

	int m_disconnectReason;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

