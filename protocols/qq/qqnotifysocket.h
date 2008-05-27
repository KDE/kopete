/*
    qqnotifysocket.h - Notify Socket for the QQ Protocol
    forked from msnnotifysocket.h

    Copyright (c) 2006      by Hui Jin <blueangel.jin@gmail.com>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>
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
#include "libeva.h"


class QQAccount;
class QTimer;

/**
 * @author Hui Jin
 * 
 * QQNotifySocket is forked from MSNNotifySocket, it parse the incoming QQ packets,
 * emit signals and drive the state machine.
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

	void sendGetGroupNames()
	{ sendPacket( Eva::groupNames( m_qqId, m_id++, m_sessionKey ) ); }

	inline void sendDownloadGroups( int next = 0 ) 
	{ sendPacket( Eva::downloadGroups( m_qqId, m_id++, m_sessionKey, next ) ); }

	void sendTextMessage( const uint toId, const QByteArray& message );

		

signals:
	void statusChanged( const Kopete::OnlineStatus &newStatus );
	void newContactList();
	void contactList( const Eva::ContactInfo& ci );
	void groupNames( const QStringList& ql );
	void contactInGroup(const int qqId, const char type, const int groupId );
	void contactStatusChanged( const Eva::ContactStatus& cs);
	void messageReceived( const Eva::MessageHeader&, const Eva::ByteArray& );
	void contactDetailReceived( const QString& , const QMap<const char*, QByteArray>& ) ;

protected:
	/**
	 * Handle an QQ incoming packet.
	 */
	virtual void handleIncomingPacket( const QByteArray& rawData );

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
	void sendGoodbye() { return; }

private:
	inline void sendPacket( const Eva::ByteArray& packet )
	{ QQSocket::sendPacket( QByteArray(packet.c_str(), packet.size() ) ); }

	void groupNames( const Eva::ByteArray& text );
	void groupInfos( const Eva::ByteArray& text );
	void doGetContactStatuses( const Eva::ByteArray& text );
	void sendListOnlineContacts(uint pos = 0);
	void sendMsgReply( int sequence, const Eva::ByteArray& replyKey );
	void heartbeat();
	void contactDetail(Eva::uint qqId);

private:
	QQAccount *m_account;

	/** 
	 * The QQ ID associated with m_account for the sake of convenience.
	 */
	uint m_qqId;

	/**
	 * stores the expected status
	 * would synchronize when ChangeStatus packet received.
	 */
	Kopete::OnlineStatus m_newstatus; 
	
	/** 
	 * stores the login token requested from the server
	 */
	Eva::ByteArray m_token;

	/**
	 * Twice Md5 hashed password
	 */
	Eva::ByteArray m_passwordKey;

	/**
	 * sessionKey is used to encrypt/decrypt the conversation
	 */
	Eva::ByteArray m_sessionKey;

	/**
	 * transferKey is the identification for the conversation
	 * transferToken is used to fetch the user picture 
	 */
	Eva::ByteArray m_transferKey;
	// FIXME: Rename me!
	Eva::ByteArray m_transferToken;

	char m_loginMode;
	int m_disconnectReason;

	// heartbeat timer
	QTimer* m_heartbeat;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

