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
#include <kopetetransfermanager.h>

class QFile;
namespace Kopete { class Contact; }

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
	Q_CLASSINFO("EUF-GUID", "5D3E02AB-6190-11D3-BBBB-00C04F795683")
	Q_CLASSINFO("APP-ID",	"2")

	public :
		/** @brief Creates a new instance of the FileTransferSession class. */
		FileTransferSession(const Q_UINT32 id, Direction direction, Kopete::Contact *contact, QObject *parent);
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
		void sendFile(QFile *file);

	public slots:
		void onDataReceived(const QByteArray& data, bool lastChunk);
		void onSend(const Q_INT32 identifier);

	private slots:
		void sessionAccepted(Kopete::Transfer *transfer, const QString& file);
		void sessionDeclined(const Kopete::FileTransferInfo& info);

	private:
		class FileTransferSessionPrivate;
		FileTransferSessionPrivate *d;

}; // FileTransferSession
}

#endif
