/***************************************************************************
                          imchatservice.h  -  description
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

#ifndef KMSNCHATSERVICE_H
#define KMSNCHATSERVICE_H

#include <qobject.h>
#include <qstrlist.h>

#include <kstringhandler.h>

class QSocket;
class KExtendedSocket;
class KopeteMessage;

/**
 * @author Olaf Lueg
 */
class KMSNChatService : public QObject
{
	Q_OBJECT

public:
	KMSNChatService();
	~KMSNChatService();
	KExtendedSocket *msgSocket;
	QString msgHandle;
	KStringHandler kstr;
	QSocket *fileSocket;
	int socketTimer;

protected:
	QString myHandle;
	QString buffer;

	// functions
	QString readLine();
	bool canReadLine();
	QString readBlock(uint len);
	void timerEvent(QTimerEvent *ev);
	QString parseFontAttr(QString str, QString attr);

public:
	void connectToSwitchBoard(QString ID, QString address, QString auth);
	void callUser();
	void setHandle(QString handle){myHandle = handle;}

	const QStringList &chatMembers() { return m_chatMembers; }

public slots:
	void slotDataReceived();
	void slotSendMsg( const KopeteMessage &msg );
	void slotSocketClosed();
	void slotCloseSession();
	void slotInviteContact(QString handle);
	void slotTypingMsg();

signals:
	void msgReceived( const KopeteMessage &msg );
	void startChat(KMSNChatService* switchoard);
	void userTypingMsg(QString);
	void msgAcknowledgement(bool);
	void userInChat(QString);
	void chatWith(QString,bool);
	void switchBoardIsActive(bool);
	void updateChatMember(QString,QString,bool);

private:
	uint m_id;

	/**
	 * Send an MSN command to the socket
	 */
	void sendCommand( const QCString &cmd, const QCString &args = "",
		bool addNewLine = true );

	QStringList m_chatMembers;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

