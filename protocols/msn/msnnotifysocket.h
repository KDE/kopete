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
#include <kstringhandler.h>
#include <kextsock.h>
// qt
#include <qsocket.h>
// other
#include <time.h>

/**
 * @author Olaf Lueg
 */
class KMSNServiceSocket : public QObject
{
	Q_OBJECT

public:
	KMSNServiceSocket();
	~KMSNServiceSocket();

	/**
	 * Treat the service socket as singleton.
	 * WARNING: This is no longer true when Kopete supports multiple
	 *          identities per protocol. By then the KMSNService class should
	 *          either be phased out completely or another solution must be
	 *          found! Let's hope I make it in time to get the former result :)
	 * FIXME: Hide this again ASAP, this is dangerous! - Martijn
	 */
	static KMSNServiceSocket *kmsnServiceSocket();

	QString _publicName;
	QString buffer;
	
	QString readLine();
	bool canReadLine();
	QString readBlock(uint len);
	
protected:
	uint _serial;
	uint mailCount;
	QString _handle;
	QString _password;
	QString hashRes;
	bool _silent;
	time_t ID;
	time_t md5_tr;
	KExtendedSocket *socket;
	bool isConnected;
	KStringHandler kstr;
	
protected slots:
	void slotDataReceived();
	void slotSocketError(int error);
	void slotSocketClose();
	void slotSocketConnected();
	
protected:
	void sendProtocol();
	void newConnect( QString data);
	void sendServerPolicy();
	void sendInitialInfo();
	void sendResponseInfo();
	void sendSerial();
	void sendCVR();
	void sendFinalAuthentication(QString res);
	
	void parseCommand( QString str);
	
public:
	void connectToService( QString handle, QString password, uint serial, bool silent);
	void close();
	void kill();
	void closeService();
	
	void setStatus( int status );
	void addContact( const QString &handle, QString pulicName, uint group , int list );
	void removeContact( const QString &handle, uint group, int list );
	
	void addGroup( QString groupName );
	void removeGroup( uint group );
	void renameGroup(QString groupName, uint group);
	
	void changePublicName(QString publicName );
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
	
	void newPublicName(QString);
	void publicNameChanged( QString, QString );
	void newSerial( uint );
	void statusChanged( QString );

private:
	static KMSNServiceSocket *s_kmsnServiceSocket;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

