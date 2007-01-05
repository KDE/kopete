/*
    session.h - Peer to Peer Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__SESSION_H
#define CLASS_P2P__SESSION_H

#include <qobject.h>
#include <qstring.h>

class QFile;

namespace PeerToPeer
{

/**
 * @brief Represents a shared context between peer endpoints that exchanges
 * data by providing a unique identifier for point-to-point communication.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class Session : public QObject
{
	Q_OBJECT

	public :
		/** @brief Indicates which side of the communication the session is implemented on. */
		enum Direction { None, Incoming, Outgoing };
		/** @brief Represents the possible states of a session. */
		enum SessionState { Created, Established, Terminated, Canceled, Faulted };

	public :
		/** @brief Creates a new instance of the Session class. */
		Session(const Q_UINT32 identifier, Direction direction, QObject *parent);
		virtual ~Session();

		Direction direction() const;
		/** @brief Gets the identifier of a session. */
		const Q_UINT32 id();
		void setDataStore(QFile *file);
		void setTransport(const Q_UINT32 transportId);
		const SessionState state() const;
		const Q_UINT32 transport();
		QFile* dataStore();

		void end();
		void start();

	public slots:
		virtual void onDataReceived(const QByteArray& data);
		virtual void onEndOfData(const Q_INT32 identifier);
		virtual void onMessageSent(const Q_INT32 identifier);
		virtual void onMessageReceived(const QByteArray& message, const Q_INT32 identifier, const Q_INT32 relatesTo);

	signals:
		void sendAcknowledge(const Q_INT32 identifier);
		void sendMessage(const QByteArray& message);
		void readyWrite(const QByteArray& dataBuffer);
		void terminate();

	protected:
		void setState(const SessionState state) const;
		virtual void onStart() = 0;
		virtual void onEnd() = 0;

	private:
		class SessionPrivate;
		SessionPrivate *d;

}; // Session
}

#endif
