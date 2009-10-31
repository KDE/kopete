/*
    kopetetransfermanager.h

    Copyright (c) 2002-2003 by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002-2003 by Richard Smith          <kopete@metafoo.co.uk>
    Copyright (c) 2008      by Roman Jarosz           <kedgedev@centrum.cz>

    Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETETRANSFERMANAGER_H
#define KOPETETRANSFERMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QMap>

#include "kopete_export.h"

#include <kio/job.h>

namespace Kopete
{

class Transfer;
class Contact;
class Message;
class ChatSession;

/**
 * @author Nick Betcher. <nbetcher@kde.org>
 */
class KOPETE_EXPORT FileTransferInfo
{
public:
	enum KopeteTransferDirection { Incoming, Outgoing };

	FileTransferInfo();
	FileTransferInfo( Contact *, const QStringList&, const unsigned long size, const QString &, KopeteTransferDirection di, const unsigned int id, QString internalId=QString(), const QPixmap &preview=QPixmap(), bool saveToDirectory = false );
	~FileTransferInfo() {}

	bool isValid() const { return (mContact && mId > 0); }
	unsigned int transferId() const { return mId; }
	Contact* contact() const { return mContact; }
	QString file() const { return mFiles.value( 0 ); }
	QStringList files() const { return mFiles; }
	QString recipient() const { return mRecipient; }
	unsigned long size() const { return mSize; }
	QString internalId() const { return m_intId; }
	KopeteTransferDirection direction() const { return mDirection; }
	QPixmap preview() const { return mPreview; }
	bool saveToDirectory() const { return mSaveToDirectory; }

private:
	unsigned long mSize;
	QString mRecipient;
	unsigned int mId;
	QPointer<Contact> mContact;
	QStringList mFiles;
	QString m_intId;
	KopeteTransferDirection mDirection;
	QPixmap mPreview;
	bool mSaveToDirectory;
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
	virtual ~TransferManager() {}

	/**
	 * @brief Adds a file transfer to the Kopete::TransferManager
	 */
	Transfer *addTransfer( Contact *contact, const QString& file, const unsigned long size, const QString &recipient, FileTransferInfo::KopeteTransferDirection di);

	/**
	 * @brief Adds a file transfer to the Kopete::TransferManager
	 * Same as above just can have more files.
	 */
	Transfer *addTransfer( Contact *contact, const QStringList& files, const unsigned long size, const QString &recipient, FileTransferInfo::KopeteTransferDirection di);
	
	/**
	 * @brief Adds incoming file transfer request to the Kopete::TransferManager.
	 **/
	unsigned int askIncomingTransfer( Contact *contact, const QString& file, const unsigned long size, const QString& description=QString(), QString internalId=QString(), const QPixmap &preview=QPixmap() );

	/**
	 * @brief Adds incoming file transfer request to the Kopete::TransferManager.
	 * Same as above just can have more files.
	 * @note If file list has more than one file then the directory dialog is shown
	 *       instead of file dialog and saveToDirectory in FileTransferInfo will be set to true.
	 * @note If the file names are unknown add empty file names to the string list.
	 **/
	unsigned int askIncomingTransfer( Contact *contact, const QStringList& files, const unsigned long size, const QString& description=QString(), QString internalId=QString(), const QPixmap &preview=QPixmap() );
	
	/**
	 * @brief Shows save file dialog and accepts/rejects incoming file transfer request.
	 **/
	void saveIncomingTransfer( unsigned int id );

	/**
	 * @brief Cancels incoming file transfer request.
	 **/
	void cancelIncomingTransfer( unsigned int id );

	/**
	 * @brief Ask the user which file to send when they click Send File.
	 *
	 * Possibly ask the user which file to send when they click Send File. Sends a signal indicating KUrl to
	 * send when the local user accepts the transfer.
	 * @param file If valid, the user will not be prompted for a URL, and this one will be used instead.
	 *  If it refers to a remote file and mustBeLocal is true, the file will be transferred to the local
	 *  filesystem.
	 * @param localFile file name to display if file is a valid URL
	 * @param fileSize file size to send if file is a valid URL
	 * @param mustBeLocal If the protocol can only send files on the local filesystem, this flag
	 *  allows you to ensure the filename will be local.
	 * @param sendTo The object to send the signal to
	 * @param slot The slot to send the signal to. Signature: sendFile(const KUrl &file)
	 */
	void sendFile( const KUrl &file, const QString &localFile, unsigned long fileSize,
		bool mustBeLocal, QObject *sendTo, const char *slot );

signals:
	/** @brief Signals the transfer is done. */
	void done( Kopete::Transfer* );

	/** @brief Signals the transfer has been canceled. */
	void canceled( Kopete::Transfer* );

	/** @brief Signals the transfer has been accepted 
	 *  @note If saveToDirectory in FileTransferInfo is true then fileName is a directory.
	 */
	void accepted(Kopete::Transfer*, const QString &fileName);

	/** @brief Signals the transfer has been rejected */
	void refused(const Kopete::FileTransferInfo& );

	/** @brief Signals the incoming transfer has been rejected or accepted */
	void askIncomingDone( unsigned int id );

	/** @brief Send a file */
	void sendFile(const KUrl &file, const QString &localFile, unsigned int fileSize);

private slots:
	void slotComplete(KJob*);

private:
	TransferManager( QObject *parent );

	void removeTransfer( unsigned int id );

	KUrl getSaveFile( const KUrl& startDir ) const;
	KUrl getSaveDir( const KUrl& startDir ) const;

	QMap<unsigned int, Transfer *> mTransfersMap;
	QMap<unsigned int, FileTransferInfo> mTransferRequestInfoMap;
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
	explicit Transfer( const FileTransferInfo &, bool showProgressInfo = true);

	/**
	 * Destructor
	 */
	~Transfer();

	/** @brief Get the info for this file transfer */
	const FileTransferInfo &info() const;

	/**
	 * Retrieve a URL indicating where the file is being copied from.
	 * For display purposes only! There's no guarantee that this URL
	 * refers to a real file being transferred.
	 */
	KUrl sourceURL();

	/**
	 * Retrieve a URL indicating where the file is being copied to.
	 * See @ref sourceURL
	 */
	KUrl destinationURL();
protected:
	void emitCopying(const KUrl &src, const KUrl &dest);

	virtual void timerEvent ( QTimerEvent * event );

public slots:
	/**
	 * @brief Set the source and destination file names of currently processed file.
	 */
	void slotNextFile( const QString &sourceFile, const QString &destinationFile );

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

	/** transfer was cancelled (but not by our user) */
	void slotCancelled();
	/** something interesting happened; not an error /
	void slotInfo( int type, const QString &text ); */

	/** display a message in the chatwindow if it exists */
	bool showMessage( QString text ) const;

signals:
	/**
	 * @deprecated Use result() and check error() for ERR_USER_CANCELED
	 */
	void transferCanceled();

private:
	void init( const KUrl &, bool );

	static KUrl displayURL( const Contact *contact, const QString &file );

	bool showHtmlMessage( QString text ) const;
	QString fileForMessage() const;

	void stopTransferRateTimer();
	Kopete::ChatSession* chatSession() const;

	class Private;
	Private* d;
private slots:
	void slotResultEmitted();
	void slotContactDestroyed();
};

}

#endif // KOPETETRANSFERMANAGER_H
// vim: set noet ts=4 sts=4 sw=4:
