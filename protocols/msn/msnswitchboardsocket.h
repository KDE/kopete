/***************************************************************************
                          msnswitchboardsocket.h  -  description
                             -------------------
    begin                : Tue Nov 27 2001
    copyright            : (C) 2001 by Olaf Lueg
    email                : olueg@olsd.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MSNSWITCHBOARDSOCKET_H
#define MSNSWITCHBOARDSOCKET_H

#include <qobject.h>
#include <qstrlist.h>

#include <kstringhandler.h>

#include <msnsocket.h>

class KopeteMessage;
class MSNProtocol;

/**
 * @author Olaf Lueg
 */
class MSNSwitchBoardSocket : public MSNSocket
{
	Q_OBJECT

public:
	/**
	 * Contructor: id is the KopeteMessageMangager's id
	 */
	MSNSwitchBoardSocket( MSNProtocol *protocol );
	~MSNSwitchBoardSocket();

private:
	MSNProtocol *m_protocol;

	QString m_myHandle; // our handle

	// contains the handle of the last person that msg'ed us.
	// since we receive the actual message by readBlock(), we need
	// to remember what the handle was of the person sending us the message.
	QString m_msgHandle;
	
	QString m_ID;
	QString m_auth;
	QStringList m_chatMembers;

protected:
	/**
	 * Handle an MSN command response line.
	 */
	virtual void parseCommand( const QString &cmd, uint id,
		const QString &data );

	/**
	 * Handle exceptions that might occur during a chat.
	 */
	virtual void handleError( uint code, uint id );

	QString parseFontAttr( QString str, QString attr );


public:
	void connectToSwitchBoard( QString ID, QString address, QString auth );
	void setHandle( QString handle ) { m_myHandle = handle; }
	void setMsgHandle( QString handle ) { m_msgHandle = handle; }

	const QStringList &chatMembers() { return m_chatMembers; }

	void userLeftChat( QString handle );
	int sendMsg( const KopeteMessage &msg );

public slots:
	void slotCloseSession();
	void slotInviteContact(const QString &handle);

	/**
	 * Notify the server that the user is typing a message
	 */
	void sendTypingMsg( bool isTyping );

private slots:
	void slotOnlineStatusChanged( MSNSocket::OnlineStatus status );
	void slotSocketClosed(  );
	void slotReadMessage( const QString &msg );

signals:
	void msgReceived( const KopeteMessage &msg );
	void receivedTypingMsg( const QString &contactId, bool isTyping );
	void msgAcknowledgement(unsigned int, bool);
	/**
	 *  updateChatMember();
	 *  	if add=true, the contact join the chat, else, the contact leave.
	 */
	void updateChatMember(const QString &handle,const QString &plublicName, bool add  );
	
	void switchBoardClosed(  );
	void invitation(const QString& handle, const QString& msg);

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

