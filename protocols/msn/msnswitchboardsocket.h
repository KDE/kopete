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

/**
 * @author Olaf Lueg
 */
class MSNSwitchBoardSocket : public MSNSocket
{
	Q_OBJECT

public:
	MSNSwitchBoardSocket();
	~MSNSwitchBoardSocket();

protected:
	QString m_myHandle; // our handle
	QString m_msgHandle; // the other side's handle
	QString m_ID;
	QString m_auth;

  QString m_filetransferName;

	// contains the handle of the last person that msg'ed us.
	// since we receive the actual message by readBlock(), we need
	// to remember what the handle was of the person sending us the message.
	//QString m_handle;

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
	void callUser();
	void setHandle( QString handle ) { m_myHandle = handle; }
	void setMsgHandle( QString handle ) { m_msgHandle = handle; }

	const QStringList &chatMembers() { return m_chatMembers; }

 	void userLeftChat( QString handle );

public slots:
	void slotReadMessage( const QString &msg );
	void slotSendMsg( const KopeteMessage &msg );
	void slotCloseSession();
	void slotInviteContact(QString handle);
	void slotTypingMsg();

private slots:
	void slotOnlineStatusChanged( MSNSocket::OnlineStatus status );
	void slotSocketClosed( int state );

signals:
	void msgReceived( const KopeteMessage &msg );
	void startChat(MSNSwitchBoardSocket* switchoard);
	void userTypingMsg(QString);
	void msgAcknowledgement(bool);
//	void userInChat(QString); //unused
//	void chatWith(QString,bool);  //unused
	void switchBoardIsActive(bool);  
  /**
   *  updateChatMember(handle, 'true' for add and 'false' for remove, public name );
   */
	void updateChatMember(QString,bool,QString);
	void switchBoardClosed( MSNSwitchBoardSocket* switchboard );

private:
	QStringList m_chatMembers;

  //Messages sent before the ending of the connection are queued
  QValueList<KopeteMessage> m_messagesQueue;

private: // Private methods
  /** No descriptions */
  void sendMessageQueue();
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

