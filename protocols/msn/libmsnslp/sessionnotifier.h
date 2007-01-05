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

class SessionNotifier : public QObject
{
	Q_OBJECT

		Q_INT32 _type;

	public :
		/** @brief Creates a new instance of the SessionNotifier class. */
		SessionNotifier(QObject *parent);
		~SessionNotifier();

		void setType(const Q_INT32 type);
		const Q_INT32 type();

	signals:
		void dataReceived(const QByteArray& data);
		void endOfData(const Q_INT32 identifier);
		void messageAcknowledged(const Q_INT32 identifier);
		void messageReceived(const QByteArray& message, const Q_INT32 identifier, const Q_INT32 relatesTo);
		void transactionTimedout(const Q_INT32 identifier, const Q_INT32 relatesTo);

	private:
		void fireDataReceived(const QByteArray& data);
		void fireEndOfData(const Q_INT32 identifier);
		void fireMessageAcknowledged(const Q_INT32 identifier);
		void fireMessageReceived(const QByteArray& message, const Q_INT32 identifier, const Q_INT32 relatesTo);
		void fireTransactionTimedout(const Q_INT32 identifier, const Q_INT32 relatesTo);

	friend class Transport;

}; // SessionNotifier
}

#endif
