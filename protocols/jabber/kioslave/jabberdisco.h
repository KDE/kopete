
/***************************************************************************
                   Jabber Service Discovery KIO Slave
                             -------------------
    begin                : Wed June 1 2005
    copyright            : (C) 2005 by Till Gerken <till@tantalo.net>

	   Kopete (C) 2001-2005 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERDISCO_H
#define JABBERDISCO_H

#include <qobject.h>
#include <qstring.h>

#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>
#include <jabberclient.h>

#define JABBER_DISCO_DEBUG		0

class JabberClient;

class JabberDiscoProtocol : public QObject, public KIO::SlaveBase
{

Q_OBJECT

public:
	JabberDiscoProtocol ( const QByteArray &pool_socket, const QByteArray &app_socket );
	virtual ~JabberDiscoProtocol ();

	virtual void setHost(const QString& host, quint16 port, const QString& user, const QString& pass);

	void openConnection ();
	void closeConnection ();

	void slave_status ();

	void get ( const KUrl &url );
	void listDir ( const KUrl &url );
	void mimetype ( const KUrl &url );

	void dispatchLoop ();

private slots:
	void slotClientDebugMessage ( const QString &msg );
	void slotHandleTLSWarning ( int validityResult );
	void slotClientError ( JabberClient::ErrorCode errorCode );
	void slotConnected ();
	void slotCSDisconnected ();
	void slotCSError ( int error );

	void slotQueryFinished ();

private:
	enum CommandType { Get, ListDir };

	QString m_host, m_user, m_password;
	int m_port;
	KUrl m_url;
	bool m_connected;

	CommandType m_command;

	JabberClient *m_jabberClient;

};

#endif
