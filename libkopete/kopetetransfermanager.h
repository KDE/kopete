/*
    kopetetransfermanager.h

    Copyright (c) 2002-2003 by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2002-2003 by Richard Smith <kopete@metafoo.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEFILETRANSFER_H
#define KOPETEFILETRANSFER_H

#include <qobject.h>
#include <qstring.h>
#include <qmap.h>
#include "kopete_export.h"

#include <kio/job.h>

namespace Kopete
{

class Transfer;
class Contact;

/**
 * @author Nick Betcher. <nbetcher@kde.org>
 */
class KOPETE_EXPORT FileTransferInfo
{
public:
	enum KopeteTransferDirection { Incoming, Outgoing };

	FileTransferInfo( Contact *, const QString&, const unsigned long size, const QString &, KopeteTransferDirection di, const unsigned int id, QString internalId=QString::null);
	~FileTransferInfo() {}
	unsigned int transferId() const { return mId; }
	const Contact* contact() const { return mContact; }
	QString file() const { return mFile; }
	QString recipient() const { return mRecipient; }
	unsigned long size() const { return mSize; }
	QString internalId() const { return m_intId; }
	KopeteTransferDirection direction() const { return mDirection; }

private:
	unsigned long mSize;
	QString mRecipient;
	unsigned int mId;
	Contact *mContact;
	QString mFile;
	QString m_intId;
	KopeteTransferDirection mDirection;
};

/**
 * Creates and manages kopete file transfers
 */
class KOPETE_EXPORT TransferManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * Retrieve the transfer manager instance
	 */
	static TransferManager* transferManager();
	virtual ~TransferManager() {};

	/**
	 * @brief Adds a file transfer to the Kopete::TransferManager
	 */
	Transfer *addTransfer( Contact *contact, const QString& file, const unsigned long size, const QString &recipient , FileTransferInfo::KopeteTransferDirection di);
	int askIncomingTransfer( Contact *contact, const QString& file, const unsigned long size, const QString& description=QString::null, QString internalId=QString::null);
	void removeTransfer( unsigned int id );

	/**
	 * @brief Ask the user which file to send when they click Send File.
	 *
	 * Possibly ask the user which file to send when they click Send File. Sends a signal indicating KURL to
	 * send when the local user accepts the transfer.
	 * @param file If valid, the user will not be prompted for a URL, and this one will be used instead.
	 *  If it refers to a remote file and mustBeLocal is true, the file will be transferred to the local
	 *  filesystem.
	 * @param localFile file name to display if file is a valid URL
	 * @param fileSize file size to send if file is a valid URL
	 * @param mustBeLocal If the protocol can only send files on the local filesystem, this flag
	 *  allows you to ensure the filename will be local.
	 * @param sendTo The object to send the signal to
	 * @param slot The slot to send the signal to. Signature: sendFile(const KURL &file)
	 */
	void sendFile( const KURL &file, const QString &localFile, unsigned long fileSize,
		bool mustBeLocal, QObject *sendTo, const char *slot );

signals:
	/** @brief Signals the transfer is done. */
	void done( Kopete::Transfer* );

	/** @brief Signals the transfer has been canceled. */
	void canceled( Kopete::Transfer* );

	/** @brief Signals the transfer has been accepted */
	void accepted(Kopete::Transfer*, const QString &fileName);

	/** @brief Signals the transfer has been rejected */
	void refused(const Kopete::FileTransferInfo& );

	/** @brief Send a file */
	void sendFile(const KURL &file, const QString &localFile, unsigned int fileSize);

private slots:
	void slotAccepted(const Kopete::FileTransferInfo&, const QString&);
	void slotComplete(KIO::Job*);

private:
	TransferManager( QObject *parent );
	static TransferManager *s_transferManager;

	int nextID;
	QMap<unsigned int, Transfer *> mTransfersMap;
};

/**
 * A KIO job for a kopete file transfer.
 * @author Richard Smith <kopete@metafoo.co.uk>
 */
class KOPETE_EXPORT Transfer : public KIO::Job
{
	Q_OBJECT

public:
	/**
	 * Constructor
	 */
	Transfer( const FileTransferInfo &, const QString &localFile, bool showProgressInfo = true);

	/**
	 * Constructor
	 */
	Transfer( const FileTransferInfo &, const Contact *toUser, bool showProgressInfo = true);

	/**
	 * Destructor
	 */
	~Transfer();

	/** @brief Get the info for this file transfer */
	const FileTransferInfo &info() const { return mInfo; }

	/**
	 * Retrieve a URL indicating where the file is being copied from.
	 * For display purposes only! There's no guarantee that this URL
	 * refers to a real file being transferred.
	 */
	KURL sourceURL();

	/**
	 * Retrieve a URL indicating where the file is being copied to.
	 * See @ref sourceURL
	 */
	KURL destinationURL();

public slots:

	/**
	 * @brief Set the file size processed so far
	 */
	void slotProcessed(unsigned int);

	/**
	 * @brief Indicate that the transfer is complete
	 */
	void slotComplete();

	/**
	 * @brief Inform the job that an error has occurred while transferring the file.
	 *
	 * @param error A member of the KIO::Error enumeration indicating what error occurred.
	 * @param errorText A string to aid understanding of the error, often the offending URL.
	 */
	void slotError( int error, const QString &errorText );

signals:
	/**
	 * @deprecated Use result() and check error() for ERR_USER_CANCELED
	 */
	void transferCanceled();

private:
	void init( const KURL &, bool );

	static KURL displayURL( const Contact *contact, const QString &file );

	FileTransferInfo mInfo;
	KURL mTarget;
	int mPercent;

private slots:
	void slotResultEmitted();
};

}

#endif
// vim: set noet ts=4 sts=4 sw=4:
