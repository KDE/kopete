/***************************************************************************
                          imservicesocket.h  -  description
                             -------------------
    begin                : Mon Nov 12 2001
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


#ifndef KMSNSERVICESOCKET_H
#define KMSNSERVICESOCKET_H

#include <qobject.h>

// kde include
#include <kextsock.h>
// qt
#include <qsocket.h>
// other
#include <time.h>

class MSNDispatchSocket;

/**
 * @author Olaf Lueg
 */
class KMSNServiceSocket : public QObject
{
	Q_OBJECT

public:
	KMSNServiceSocket();
	~KMSNServiceSocket();

	QString _publicName;
	QString buffer;
	QString readBlock(uint len);

protected:
	uint _serial;
	uint mailCount;
	QString _handle;
	QString _password;
	bool _silent;
	KExtendedSocket *socket;
	bool isConnected;

protected slots:
	void slotDataReceived();
	void slotSocketError(int error);

	/**
	 * Check if new lines of data are available and process the first line
	 */
	void slotReadLine();

	/**
	 * The actual connect code, shared between the initial connect and a
	 * server-initiated redirect to another server
	 */
	void slotReceivedServer( const QString &server, uint port );

protected:
	void sendProtocol();
	void sendCVR();

	void parseCommand( QString str);

public:
	void connectToMSN( const QString &handle, const QString &password,
		uint serial, bool silent );
	void close();
	void closeService();

	void setStatus( int status );
	void addContact( const QString &handle, QString pulicName, uint group , int list );
	void removeContact( const QString &handle, uint group, int list );

	void addGroup( QString groupName );
	void removeGroup( uint group );
	void renameGroup(QString groupName, uint group);

	void changePublicName( const QString &publicName );
	QString getPublicName() { return _publicName;}

	void createChatSession();
signals:
	void newMail(QString, uint);
	void contactList(QString, QString, QString, QString);
	void contactList(QString, QString, uint);
	void contactStatusChanged( QString, QString, QString );
	void contactStatus(QString, QString, QString );
	void contactAdded(QString, QString, QString, uint, uint);
	void contactRemoved(QString, QString, uint, uint);
	void contactOffline(QString);

	void groupName(QString, uint);
	void groupAdded( QString, uint, uint);
	void groupRenamed( QString, uint, uint);
	void groupRemoved( uint, uint);

	void sessionClosed( QString);
	void connected( bool);

	void invitedToChat(QString, QString, QString, QString, QString );
	void startChat( QString, QString );

	void publicNameChanged( QString, QString );
	void newSerial( uint );
	void statusChanged( QString );

private:
	/**
	 * Send an MSN command to the socket
	 */
	void sendCommand( const QString &cmd, const QString &args = QString::null,
		bool addNewLine = true );

	/**
	 * The id of the message sent to the MSN server. This ID will increment
	 * for each subsequent message sent or received.
	 */
	uint m_id;

	/**
	 * Convert an entry of the Status enum back to a string
	 */
	QString statusToString( int status ) const;

	void closeSocket();

	MSNDispatchSocket *m_dispatchSocket;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

