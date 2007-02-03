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
		enum Direction { Incoming, Outgoing };
		/** @brief Defines the states of a session during its lifecycle. */
		enum SessionState { Created, Established, Terminated, Canceled, Faulted };

	public :
		virtual ~Session();
		/** @brief Gets the direction of data flow for the communication. */
		const Direction direction() const;
		/** @brief Gets the unique identifier for the session. */
		const Q_UINT32 id() const;
		/** @brief Gets the current state of the session. */
		const SessionState state() const;

		/** @brief Accepts a session. */
		void accept();
		/** @brief Cancels a session. */
		void cancel();
		/** @brief Declines a session. */
		void decline();
		/** @brief Ends a session. */
		void end();
		/** @brief Handles a session invitation. */
		virtual void handleInvite(const Q_UINT32 appId, const QByteArray& context) = 0;
		/** @brief Starts a session. */
		void start();

	protected:
		/** @brief Creates a new instance of the Session class. */
		Session(const Q_UINT32 identifier, Direction direction, QObject *parent);

		void fault();
		/** @brief When overriden by a derived class, starts a session. */
		virtual void onStart() = 0;
		/** @brief When overriden by a derived class, ends a session. */
		virtual void onEnd() = 0;
		/** @brief When overriden by a derived class, faults a session. */
		virtual void onFaulted() = 0;

	signals:
		/** @brief Indicates that a session has been accepted by the user. */
		void accepted();
		/** @brief Indicates that a session has been cancelled by the user. */
		void cancelled();
		/** @brief Indicates that a session has been declined by the user. */
		void declined();
		/** @brief Indicates that a session has encountered a fault. */
		void faulted();
		/** @brief Indicates that a session has ended. */
		void ended();

		void sendData(const QByteArray& bytes);
		void sendMessage(const QByteArray& bytes);

	private:
		class SessionPrivate;
		SessionPrivate *d;

}; // Session
}

#endif
