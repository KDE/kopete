/*
    sessionnotifier.h - Peer to Peer Session Notifier

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__SESSIONNOTIFIER_H
#define CLASS_P2P__SESSIONNOTIFIER_H

#include <qobject.h>

namespace PeerToPeer
{

/**
 * @brief Provides support for monitoring activity on a session.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class SessionNotifier : public QObject
{
	Q_OBJECT

	public :
		enum Type { Normal=0, Object=1, FileTransfer=2, Webcam=4 };

	public :
		/** @brief Creates a new instance of the SessionNotifier class. */
		SessionNotifier(const Q_UINT32 session, const Type type, QObject *parent=0l);
		~SessionNotifier();

		const Q_UINT32 session() const;
		void setType(const Type type);
		const Type type() const;

	signals:
		void dataReceived(const QByteArray& data, const Q_INT32 identifier, bool lastChunk);
		void messageAcknowledged(const Q_INT32 identifier);
		void messageReceived(const QByteArray& message, const Q_INT32 identifier, const Q_INT32 relatesTo);

	private:
		void fireDataReceived(const QByteArray& data, const Q_INT32 identifier, bool lastChunk);
		void fireMessageAcknowledged(const Q_INT32 identifier);
		void fireMessageReceived(const QByteArray& message, const Q_INT32 identifier, const Q_INT32 relatesTo);

	private:
		class SessionNotifierPrivate;
		SessionNotifierPrivate *d;

	friend class Transport;

}; // SessionNotifier
}

#endif
