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

class QFile;

namespace PeerToPeer
{

/**
 * @brief Represents a session used to transfer a file.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class FileTransferSession : public Session
{
	Q_OBJECT

	public :
		/** @brief Creates a new instance of the  File Session class. */
		FileTransferSession(const Q_UINT32 identifier, Session::Direction direction, QObject *parent);
		virtual ~FileTransferSession();

	signals:
		void transferComplete(const QString& path);

	protected:
		virtual void onBegin();
		virtual void onEnd();

}; // FileTransferSession
}

#endif
