/*
    dialog.cpp - Peer to Peer Dialog class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "dialog.h"
#include "transaction.h"
#include <qregexp.h>
#include <qtimer.h>
#include <stdlib.h>

namespace PeerToPeer
{

class Dialog::DialogPrivate
{
	public:
		DialogPrivate() : state(Dialog::Pending)
		{
			transactionId = 10 * (rand() % 0xC38C + 10);
		}

		QUuid identifier;
		Dialog::DialogState state;
		Q_UINT32 transactionId;
		QValueList<Transaction*> transactions;

}; // DialogPrivate

Dialog::Dialog(const QUuid& identifier, QObject* parent) : QObject(parent), d(new DialogPrivate())
{
	d->identifier = identifier;
}

Dialog::Dialog(Transaction *transaction, QObject* parent) : QObject(parent), d(new DialogPrivate())
{
	QRegExp regex("\\{([0-9A-Fa-f\\-]*)\\}");
	const QString headerValue = transaction->request().headers()["Call-ID"].toString();
	if (regex.search(headerValue, false) != -1)
	{
		d->identifier = QUuid(regex.cap(1));
	}
	d->transactions.prepend(transaction);
}

Dialog::~Dialog()
{
	QValueList<Transaction*>::Iterator i = d->transactions.begin();
	while(i != d->transactions.end())
	{
		// Get the current transaction.
		Transaction *transaction = *i;
		// Remove the transaction from the list.
		d->transactions.remove(i);
		// Delete the transaction.
		delete transaction;
		transaction = 0l;

		++i;
	}

	delete d;
	d = 0l;
}

void Dialog::establish()
{
	// Set the dialog state to established
	d->state = Dialog::Established;
	// Signal that the dialog has been established.
	emit established();
}

const QString Dialog::from()
{
	Transaction *transaction = *d->transactions.at(0);
	return transaction->isLocal() ? transaction->request().from() : transaction->request().to();
}

const QUuid Dialog::identifier()
{
	return d->identifier;
}

Transaction* Dialog::initialTransaction() const
{
	return *d->transactions.at(0);
}

void Dialog::setState(const DialogState& state)
{
	d->state = state;
}

const Dialog::DialogState Dialog::state()
{
	return d->state;
}

void Dialog::terminate(const Q_UINT32 timeSpan)
{
	if (timeSpan != 0)
	{
		// Set the dialog state to terminating and wait an
		// interval 'timeSpan' before terminating the dialog.
		d->state = Dialog::Terminating;

		QTimer::singleShot(timeSpan, this, SLOT(onTimeout()));
	}
	else
	{
		// Otherwise, set the dialog state to terminated.
		d->state = Dialog::Terminated;
		// Signal that the dialog has terminated.
		emit terminated();
	}
}

const QString Dialog::to()
{
	Transaction *transaction = *d->transactions.at(0);
	return transaction->isLocal() ? transaction->request().to() : transaction->request().from();
}

const QValueList<Transaction*> & Dialog::transactions() const
{
	return d->transactions;
}

QValueList<Transaction*> & Dialog::transactions()
{
	return d->transactions;
}

void Dialog::setInitialTransaction(Transaction* transaction)
{
	d->transactions.prepend(transaction);
}

const Q_UINT32 Dialog::transactionId(bool nextTransactionId)
{
	return (nextTransactionId ? ++d->transactionId : d->transactionId);
}

void Dialog::onTimeout()
{
	// Set the dialog state to terminated.
	d->state = Dialog::Terminated;
	// Signal that the dialog has terminated.
	emit terminated();
}

}

#include "dialog.moc"
