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
#include <qfile.h>
#include <filetransfer.h>

class QString;
class JabberAccount;
namespace Kopete { class Transfer; }
namespace Kopete { class FileTransferInfo; }
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
	void slotIncomingTransferAccepted ( Kopete::Transfer *transfer, const QString &fileName );
	void slotTransferRefused ( const Kopete::FileTransferInfo &transfer );
	void slotTransferResult ();
	void slotTransferError ( int errorCode );

	void slotOutgoingConnected ();
	void slotOutgoingBytesWritten ( qint64 nrWritten );

	void slotIncomingDataReady ( const QByteArray &data );

	void slotThumbnailReceived ();
	void askIncomingTransfer ( const QByteArray &thumbnail = QByteArray() );

private:
	void initializeVariables ();

	JabberAccount *mAccount;
	JabberBaseContact *mContact;
	XMPP::FileTransfer *mXMPPTransfer;
	Kopete::Transfer *mKopeteTransfer;
	QFile mLocalFile;
	int mTransferId;
	qint64 mBytesTransferred;
	qint64 mBytesToTransfer;

};

#endif

// vim: set noet ts=4 sts=4 tw=4:
