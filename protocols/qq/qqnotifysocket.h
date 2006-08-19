/*
    qqnotifysocket.h - Notify Socket for the QQ Protocol
    forked from qqnotifysocket.h

    Copyright (c) 2006      by Hui Jin <blueangel.jin@gmail.com>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>
    Copyright (c) 2005      by Gregg Edghill          <gregg.edghill@gmail.com>

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

#ifndef QQNOTIFYSOCKET_H
#define QQNOTIFYSOCKET_H

#include "qqsocket.h"
#include "qqprotocol.h"
#include "libeva.h"


class QQAccount;
class KTempFile;


/**
 * @author Hui Jin
 * 
 * QQNotifySocket is inspried by MSNNotifySocket, the QQ incoming packets are
 * parsed here.
 */
class QQNotifySocket : public QQSocket
{
	Q_OBJECT

public:
	QQNotifySocket( QQAccount* account, const QString &password );
	~QQNotifySocket();

	virtual void disconnect();

	/**
	 * this should return a  Kopete::Account::DisconnectReason value
	 */
	int  disconnectReason() { return m_disconnectReason; }

	QString localIP() { return m_localIP; }

signals:
	void statusChanged( const Kopete::OnlineStatus &newStatus );
	void newContactList();
	void contactList( const Eva::ContactInfo& ci );
	void groupNames( const QStringList& ql );
	void contactInGroup(const int qqId, const char type, const int groupId );
	void contactStatusChanged( const Eva::ContactStatus& cs);
	void messageReceived( const Eva::MessageHeader&, const Eva::ByteArray& );

protected:
	/**
	 * Handle an QQ incoming packet.
	 */
	virtual void parsePacket( const QByteArray& data );

	/**
	 * Handle an QQ error condition.
	 * This reimplementation handles most of the other QQ error codes.
	 */
	virtual void handleError( uint code, uint id );

	/**
	 * This reimplementation sets up the negotiating with the server and
	 * suppresses the change of the status to online until the handshake
	 * is complete.
	 */
	virtual void doneConnect();

	// QQ operations
	void sendLoginTokenRequest(); 
	void sendLogin(); 
	void sendContactList( short pos );
	void sendChangeStatus( char status );
	void sendRequestTransferKey();
	void sendGoodbye() { return; }

public:
	void sendDownloadGroups( int pos = 0 );
	void sendGetGroupNames();
	void sendTextMessage( const uint toId, const QByteArray& message );

private:
	void doGetGroupNames( const Eva::ByteArray& text );
	void doGetCGTs( const Eva::ByteArray& text );
	void doGetContactStatuses( const Eva::ByteArray& text );
	void sendListOnlineContacts(uint pos = 0);
	void sendMsgReply( int sequence, const Eva::ByteArray& replyKey );
	void heartbeat();

private:
	QQAccount *m_account;
	/**
	 * stores the expected status
	 * would synchronize when ChangeStatus packet recieved.
	 */
	Kopete::OnlineStatus m_newstatus; 
	
	uint m_qqId;
	/** 
	 * stores the token requested from the server
	 */
	Eva::ByteArray m_token;
	/**
	 * Twice Md5 hashed password
	 */
	Eva::ByteArray m_passwordKey;

	/**
	 * sessionKey is used to encrypt the conversation
	 */
	Eva::ByteArray m_sessionKey;

	/**
	 * transferKey is the identification for the conversation
	 * transferToken is used to fetch the user picture 
	 */
	Eva::ByteArray m_transferKey;
	Eva::ByteArray m_transferToken;

	char m_loginMode;
	// FIXME: Do we need this ?
	QString m_password;

	int m_disconnectReason;


	/**
	 * Convert an entry of the Status enum back to a string
	 */
	QString statusToString( const Kopete::OnlineStatus &status ) const;


	//know the last handle used
	QString m_configFile;
	QString m_localIP;

	// heartbeat timer
	QTimer* m_heartbeat;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

