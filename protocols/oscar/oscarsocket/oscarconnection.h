/*
    oscarconnection.h  -  Implementation of an oscar connection

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARCONNECTION_H
#define OSCARCONNECTION_H

#define USE_KEXTSOCK 1

#ifdef USE_KEXTSOCK
#include <kextsock.h>
#else
#include <qbufferedsocket.h>
#endif

#include <qobject.h>

/**
 * Implementation of a base oscar connection.
 * No login functions, just basic direct Oscar connection functionality.
 *
 * @author Tom Linsky
 */
class OscarConnection : public QObject
{
	Q_OBJECT

	public:
		/** Enum for connection type */
		enum ConnectionType
		{
			DirectIM=0, Server, SendFile, Redirect
		};

		/** Enum for typing notifications */
		enum TypingNotify
		{
			TypingFinished=0, TextTyped, TypingBegun
		};

		enum ConnectionStatus
		{
			Disconnected=0, Connecting, Connected
		};

		OscarConnection(const QString &connName, ConnectionType type,
			const QByteArray &cookie, QObject *parent=0, const char *name=0);
		virtual ~OscarConnection();

		inline QString connectionName() const { return mConnName; };

		/**
		 * @return Type of this connection
		 */
		inline OscarConnection::ConnectionType connectionType() const { return mConnType; };

		/**
		 * @return Currently logged in user's screen-name/UIN
		 */
		const QString &getSN() const;

		ConnectionStatus socketStatus() const;

		/**
		 * Sets the currently logged in users screen name
		 */
		void setSN(const QString &newSN);

		/** Gets the message cookie */
		inline const QByteArray &cookie() const { return mCookie; };

		void connectTo(const QString &host, const QString &port);

		/**
		 * Sends the direct IM message to buddy
		 */
		virtual void sendIM(const QString &message, bool isAuto);

		/**
		 * Sends a typing notification to the connection
		 * @param notifyType Type of notify to send
		 */
		virtual void sendTypingNotify(TypingNotify notifyType);

		/**
		 * Sends request to the client telling he/she that we want to send a file
		 */
		virtual void sendFileSendRequest();


	signals:
		/**
		 * Emitted when an IM comes in
		 */
		//void gotIM(QString, QString, bool);

		/**
		 * Emitted when an OSCAR protocol error occurs
		 */
		void protocolError(QString, int);

		/**
		 * Emitted when we get a minityping notifications
		 * First param is the screen name, second is the type
		 */
		void gotMiniTypeNotification(const QString &, OscarConnection::TypingNotify);

		/**
		 * Emitted when we are ready to send commands!
		 */
		void socketConnected(const QString &name);

		/**
		 * Emitted when the connection is closed
		 * @p name name of this connection
		 */
		void socketClosed(const QString &name);

		/**
		 * Emitted when a lowlevel socket error occured
		 * @p name name of this connection
		 */
		void socketError(const QString &name, int);



	protected slots:
		/**
		 * Called when there is data to be read.
		 * If you want your connection to be able to receive data, you
		 * should override this
		 */
		virtual void slotRead();


	private slots:
		/**
		 * Called when the socket has established a connection
		 */
		void slotSocketConnected();

		/**
		 * Called when the socket is closed
		 */
		void slotSocketClosed();

		/**
		 * Called when a socket error has occured
		 */
		void slotSocketError(int);


	private:
		/** The ICBM cookie used to authenticate */
		QByteArray mCookie;

		/**
		 * The name of the connection
		 */
		QString mConnName;

		/**
		 * The connection type
		 */
		ConnectionType mConnType;

		/**
		 * The users screen-name/UIN
		 */
		QString mSN;

	protected:
		/**
		 * The encapsulated socket
		 */
		#ifdef USE_KEXTSOCK
		KExtendedSocket *mSocket;
		#else
		QBufferedSocket *mSocket;
		#endif
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
