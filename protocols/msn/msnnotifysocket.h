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


#ifndef MSNNOTIFYSOCKET_H
#define MSNNOTIFYSOCKET_H

#include "msnauthsocket.h"
#include "msnprotocol.h"

class MSNDispatchSocket;

/**
 * @author Olaf Lueg
 */
class MSNNotifySocket : public MSNAuthSocket
{
	Q_OBJECT

public:
	MSNNotifySocket( const QString &msnId );
	~MSNNotifySocket();

	void connect( const QString &password );
	virtual void disconnect();

	void setStatus( int status );
	void addContact( const QString &handle, QString pulicName, uint group , int list );
	void removeContact( const QString &handle, uint group, int list );

	void addGroup( QString groupName );
	void removeGroup( uint group );
	void renameGroup(QString groupName, uint group);

	void changePublicName( const QString &publicName );

	void createChatSession();

signals:
//	void newMail(QString, uint);
	void contactList(QString, QString, QString, QString);
	void contactList(QString, QString, uint);
	void contactStatusChanged( const QString &msnId, const QString &publicName,
		MSNProtocol::Status status );
	void contactStatus(QString, QString, QString );
	void contactAdded(QString, QString, QString, uint, uint);
	void contactRemoved(QString, QString, uint, uint);

	void groupName(QString, uint);
	void groupAdded( QString, uint, uint);
	void groupRenamed( QString, uint, uint);
	void groupRemoved( uint, uint);

	void invitedToChat(QString, QString, QString, QString, QString );
	void startChat( QString, QString );

	void publicNameChanged( QString, QString );
	void statusChanged( QString );

	void recievedInfo(QString, QString , QString);

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
	
	void slotDispatchFailed();
	void slotWriteHotmailTmpFile();

private:
	unsigned int mailCount;
	QString m_password;
	
	int m_newstatus;

	/**
	 * Convert an entry of the Status enum back to a string
	 */
	QString statusToString( int status ) const;

	MSNDispatchSocket *m_dispatchSocket;

	//for hotmail inbox opening
	QString m_MSPAuth;
	QString m_kv;
	QString m_sid;
	QString m_HotmailTmpFile;
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

