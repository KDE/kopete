/*
    filetransfersession.h - Peer to Peer File Transfer Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__FILETRANSFERSESSION_H
#define CLASS_P2P__FILETRANSFERSESSION_H

#include "session.h"
#include <quuid.h>

class QFile;

namespace PeerToPeer
{

/**
 * @brief Represents a session used to send or receive files.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class FileTransferSession : public Session
{
	Q_OBJECT

	public :
		static QUuid uuid();

	public :
		/** @brief Creates a new instance of the FileTransferSession class. */
		FileTransferSession(const Q_UINT32 identifier, Direction direction, QObject *parent);
		virtual ~FileTransferSession();
		/** @brief Handles a file transfer session invitation. */
		virtual void handleInvite(const Q_UINT32 appId, const QByteArray& context);

		QFile* dataStore() const;
		void setDataStore(QFile *file) const;

	protected:
		virtual void onStart();
		virtual void onEnd();
		virtual void onFaulted();

	signals:
		/** @brief Indicates the current progress of a file download. */
		void dataReadProgress(const Q_UINT32 done, const Q_INT64 total);
		/** @brief Indicates the current progress of a file upload. */
		void dataSendProgress(const Q_UINT32 done, const Q_INT64 total);

	public slots:
		void onDataReceived(const QByteArray& data, const Q_INT32 identifier, bool lastChunk);
		void onSend(const Q_INT32 identifier);

	private:
		class FileTransferSessionPrivate;
		FileTransferSessionPrivate *d;

}; // FileTransferSession
}

#endif
