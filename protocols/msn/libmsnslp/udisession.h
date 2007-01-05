/*
    udisession.h - Peer to Peer User Display Icon Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__UDISESSION_H
#define CLASS_P2P__UDISESSION_H

#include "session.h"

namespace PeerToPeer
{

/**
 * @brief Represents a session for transfering a user display icon.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class UdiSession : public Session
{
	Q_OBJECT

	public :
		/** @brief Creates a new instance of the UdiSession class. */
		UdiSession(const Q_UINT32 id, Session::Direction direction, QObject *parent);
		/** @brief Finalizer. */
		virtual ~UdiSession();

		void onDataReceived(const QByteArray& data);
		void onEndOfData(const Q_INT32 identifier);
		void onMessageSent(const Q_INT32 identifier);
		void onMessageReceived(const QByteArray& message, const Q_INT32 identifier, const Q_INT32 relatesTo);

	protected:
		virtual void onStart();
		virtual void onEnd();

	signals:
		void transferComplete(const QString& path);

}; // UdiSession
}

#endif
