/*
    transaction.h - Peer to Peer Transaction class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__TRANSACTION_H
#define CLASS_P2P__TRANSACTION_H

#include <qobject.h>
#include <quuid.h>
#include "slprequest.h"

namespace PeerToPeer
{

/**
 * @brief Represents a series of messages exchanged between a caller and a callee.
 *
 * A transaction consists of one request sent by the caller and any responses
 * to that request sent from the callee back to the caller.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class Transaction : public QObject
{
	Q_OBJECT

		/** @brief Represents the states of a transaction. */
		enum TransactionState {Calling=0, Confirmed=2, Terminated=4};

	public :
		explicit Transaction(QObject *parent=0l);
		Transaction(const SlpRequest& request, bool isLocal=true, QObject* parent=0l);
		~Transaction();

		void begin() const;
		void confirm() const;
		void end() const;
		const QUuid identifier() const;
		const bool isLocal() const;
		const SlpRequest & request() const;
		void setRequest(const SlpRequest& request);
		const TransactionState & state() const;

	signals:
		void timeout();

	private slots:
		void onCheckTransactionTimeout();

	private:
		class TransactionPrivate;
		TransactionPrivate *d;
};
}

#endif
