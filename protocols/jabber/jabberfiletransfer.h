 /*
  * jabberfiletransfer.h
  *
  * Copyright (c) 2004 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#ifndef JABBERFILETRANSFER_H
#define JABBERFILETRANSFER_H

#include <qobject.h>
#include <filetransfer.h>

class QString;
class JabberAccount;
class KopeteTransfer;
class KopeteFileTransferInfo;
class JabberBaseContact;

class JabberFileTransfer : public QObject
{

Q_OBJECT

public:
	/**
	 * Constructor for an incoming transfer
	 */
	JabberFileTransfer ( JabberAccount *account, XMPP::FileTransfer *incomingTransfer );

	/**
	 * Constructor for an outgoing transfer
	 */
	JabberFileTransfer ( JabberAccount *account, JabberBaseContact *contact, const QString &file );

	~JabberFileTransfer ();

private slots:
	void slotIncomingTransferAccepted ( KopeteTransfer *transfer, const QString &fileName );
	void slotTransferRefused ( const KopeteFileTransferInfo &transfer );
	void slotTransferCanceled ();
	void slotTransferError ( int errorCode );

	void slotOutgoingConnected ();
	void slotOutgoingBytesWritten ( int nrWritten );

	void slotIncomingDataReady ( const QByteArray &data );

private:
	JabberAccount *mAccount;
	XMPP::FileTransfer *mXMPPTransfer;
	KopeteTransfer *mKopeteTransfer;
	QFile mLocalFile;
	int mTransferId;
	int mBytesTransferred;

};

#endif

// vim: set noet ts=4 sts=4 tw=4:
