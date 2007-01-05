/*
    dialog.h - Peer to Peer Dialog class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__DIALOG_H
#define CLASS_P2P__DIALOG_H

#include <qobject.h>
#include <qvaluelist.h>
#include <quuid.h>

namespace PeerToPeer
{

class Transaction;

/**
 * @brief Represents a peer-to-peer relationship between two peers that persists for some time.
 *
 * A dialog enables the sequencing of messages between clients and the proper routing of requests
 * between both of them.  A dialog is identified at each client with a dialog ID which consists
 * of a Call ID value, a local tag and a remote tag.
 *
 * A dialog contains certain pieces of state needed for further message transmissions within the
 * dialog.  This state consists of the dialog ID, a local sequence number (used to order requests
 * from the client to its peer), a remote sequence number (used to order requests from its peer to the
 * client), a local URI and a remote URI.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class Dialog : public QObject
{
	Q_OBJECT

	public :
		/** @brief Represents the possible states of a dialog. */
		enum DialogState {Pending=0, Established=2, Terminating=4, Terminated=8};

	public :
		Dialog(const QUuid& identifier, QObject *parent);
		Dialog(Transaction *transaction, QObject* parent);
		~Dialog();

		void establish();
		const QString from();
		const QUuid identifier();
		Transaction* initialTransaction() const;
		void setState(const DialogState& state);
		const DialogState state();
		void terminate(const Q_UINT32 timeSpan=0);
		const QString to();
		const QValueList<Transaction*> & transactions() const;
		QValueList<Transaction*> & transactions();
		void setInitialTransaction(Transaction* transaction);
		const Q_UINT32 transactionId(bool nextTransactionId=false);

		int session;

	signals:
		void established();
		void terminated();

	private slots:
		void onTimeout();

	private:
		class DialogPrivate;
		DialogPrivate *d;

}; // Dialog
}

#endif
