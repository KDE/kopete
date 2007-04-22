/*
    transaction.cpp - Peer to Peer Transaction class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "transaction.h"
#include <qregexp.h>
#include <qtimer.h>

namespace PeerToPeer
{

class Transaction::TransactionPrivate
{
	public:
		TransactionPrivate() : isLocal(false), transactionState(Transaction::Calling) {}

		bool isLocal;
		SlpRequest request;
		QTimer *timer;
		Transaction::TransactionState transactionState;
};

Transaction::Transaction(QObject *parent) : QObject(parent), d(new TransactionPrivate())
{
	d->timer = new QTimer(this);
	// Connect the signal/slot
	QObject::connect(d->timer, SIGNAL(timeout()), this,
	SLOT(onCheckTransactionTimeout()));
}

Transaction::Transaction(const SlpRequest& request, bool isLocal, QObject* parent) : QObject(parent), d(new TransactionPrivate())
{
	d->timer = new QTimer(this);
	// Connect the signal/slot
	QObject::connect(d->timer, SIGNAL(timeout()), this,
	SLOT(onCheckTransactionTimeout()));
	d->request = request;
	d->isLocal = isLocal;
}

Transaction::~Transaction()
{
	delete d;
}

void Transaction::begin() const
{
	const Q_INT32 timeSpan = 54 * 600;
	// Start the transaction timeout timer.
	d->timer->start(timeSpan);
}

void Transaction::confirm() const
{
	d->transactionState = Transaction::Confirmed;
	// Stop the transaction timeout timer.
	d->timer->stop();
	const Q_INT32 timeSpan = 82 * 600;
	// Start the transaction timeout timer 2.
	d->timer->start(timeSpan);
}

const QUuid Transaction::branch() const
{
	QUuid branch;
	QRegExp regex("branch=\\{([0-9A-Fa-f\\-]*)\\}");
	if (regex.search(d->request.headers()["Via"].toString().section(";", 1, 1), false) != -1)
	{
		branch = QUuid(regex.cap(1));
	}

	return branch;
}

void Transaction::end() const
{
	d->transactionState = Transaction::Completed;
	// Disconnect the signal/slot
	QObject::disconnect(d->timer, 0, this, 0);
	// Stop the transaction timeout timer.
	d->timer->stop();
	d->transactionState = Transaction::Terminated;
}

const bool Transaction::isLocal() const
{
	return d->isLocal;
}

const SlpRequest & Transaction::request() const
{
	return d->request;
}

SlpRequest & Transaction::request()
{
	return d->request;
}

void Transaction::setRequest(const SlpRequest& request)
{
	d->request = request;
}

const Transaction::TransactionState & Transaction::state() const
{
	return d->transactionState;
}

void Transaction::onCheckTransactionTimeout()
{
	// NOTE The transaction state transitions to the
	// confirmed state when the request associated with
	// the transaction is acknowledge by the callee.

	// Disconnect the signal/slot
	QObject::disconnect(d->timer, 0, this, 0);
	// Stop the transaction timeout timer.
	d->timer->stop();

	// If the transaction has not been confirmed, and
	// it is still in the calling state, or the transaction
	// was not completed, raise the transaction
	// timeout event.
	if ((d->transactionState == Transaction::Calling) || (d->transactionState < Transaction::Completed))
	{
		// Signal that a transaction timeout has occured.
		emit timeout();
	}
}

}

#include "transaction.moc"
