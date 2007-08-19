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
		/** @brief Creates a new instance of the SessionNotifier class. */
		SessionNotifier(const Q_UINT32 session, const Q_UINT32 applicationId, QObject *parent=0l);
		~SessionNotifier();

		const Q_UINT32 applicationId() const;
		const Q_UINT32 session() const;

	signals:
		void dataReceived(const QByteArray& data, bool lastChunk);
		void messageAcknowledged(const Q_INT32 id);
		void messageReceived(const QByteArray& message, const Q_INT32 id, const Q_INT32 correlationId);
		void dataSendProgress(const Q_UINT32 progress);

	private:
		void fireDataReceived(const QByteArray& data, bool lastChunk);
		void fireDataSendProgress(const Q_UINT32 progress);
		void fireMessageAcknowledged(const Q_INT32 id);
		void fireMessageReceived(const QByteArray& message, const Q_INT32 id, const Q_INT32 correlationId);

	private:
		class SessionNotifierPrivate;
		SessionNotifierPrivate *d;

	friend class Transport;

}; // SessionNotifier
}

#endif
