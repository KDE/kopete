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

#include "buffer.h"
#include <qobject.h>
#include <kextsock.h>

/**Implementation of a base oscar connection.
  *No login functions, just basic direct Oscar connection functionality.
  *@author Tom Linsky
  */

class OscarConnection : public QObject
{
	Q_OBJECT

	public:
		/** Enum for connection type */
		enum ConnectionType
		{
			DirectIM, Server, SendFile, Redirect
		};

		/** Enum for typing notifications */
		enum TypingNotify
		{
			TypingFinished, TextTyped, TypingBegun
		};

		OscarConnection(const QString &sn, const QString &connName, ConnectionType type,
			const QByteArray &cookie, QObject *parent=0, const char *name=0);
		virtual ~OscarConnection();

		inline QString connectionName() const { return mConnName; };
		/** Returns the type of this connection */
		inline int connectionType() const { return mConnType; };
		/** Gets the currently logged in user's screen name */
		inline QString getSN() const { return mSN; };
		/** Sets the currently logged in user's screen name */
		void setSN(const QString &newSN);

		/** Gets the current socket **/
		KExtendedSocket* socket() const { return mSocket; };

		/** Gets the message cookie */
		inline const QByteArray &cookie() const { return mCookie; };

		// virtual functions to be overloaded by child classes

		/*
		 * Sends the direct IM message to buddy
		 */
		virtual void sendIM(const QString &message, bool isAuto);

		/* Sends a typing notification to the server
		 * @param notifyType Type of notify to send
		 */
		virtual void sendTypingNotify(TypingNotify notifyType);
		/** Sends request to the client telling he/she that we want to send a file */
		virtual void sendFileSendRequest();

	signals:
		/*
		 * Emitted when an IM comes in
		 */
//		void gotIM(QString, QString, bool);

		/*
		 * called when an AIM protocol error occurs
		 */
		void protocolError(QString, int);

		/**
		* Emitted when we get a minityping notifications
		* First param is the screen name, second is the type
		* 0: Finished
		* 1: Typed
		* 2: Begun (is typing)
		*/
		void gotMiniTypeNotification(const QString &, int);
		/*
		 * Emitted when we are ready to send commands!
		 */
		void connectionReady(QString name);
		/*
		 * Emitted when the connection is closed
		 */
		void connectionClosed(QString name);
		/*
		 * Emitted when a file transfer is complete
		 */
		void transferComplete(QString name);
		/*
		 * Emitted when data is received.. parameter is percentage of transfer complete
		 */
		void percentComplete( unsigned int percent );
		/*
		 * Emitted when a file transfer begins
		 */
		void transferBegun(OscarConnection *con, const QString& file, const unsigned long size, const QString &recipient);

	protected slots:
		/* Called when there is data to be read.
		 * If you want your connection to be able to receive data, you
		 * should override this
		 * No need to connect the signal in derived classes, just override this slot
		 */
		virtual void slotRead();

	private slots:
		/*
		 * Called when we have established a connection
		 */
		void slotConnected();
		/*
		 * Called when the connection is closed
		 */
		void slotConnectionClosed();
		/*
		 * Called when a socket error occurs
		 */
		void slotError(int);

	private:
		/** The ICBM cookie used to authenticate */
		QByteArray mCookie;
		/** The name of the connection */
		QString mConnName;
		/** The connection type */
		ConnectionType mConnType;
		/** The ip of the host we will connect to */
		QString mHost;
		/** The Port we will connect to on the peer machine */
		int mPort;
		/** The user's screen name */
		QString mSN;
		/** The socket */
		KExtendedSocket *mSocket;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
