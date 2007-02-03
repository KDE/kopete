/*
    msnobjectsession.h - Peer to Peer Msn Object Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__MSNOBJECTSESSION_H
#define CLASS_P2P__MSNOBJECTSESSION_H

#include "session.h"
#include <quuid.h>

class KTempFile;

namespace PeerToPeer
{

/**
 * @brief Represents a session used to send or receive a msn object.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class MsnObjectSession : public Session
{
	Q_OBJECT

	public :
		static QUuid uuid();

	public :
		/** @brief Creates a new instance of the MsnObjectSession class. */
		MsnObjectSession(const Q_UINT32 identifier, Direction direction, QObject *parent);
		MsnObjectSession(const QString& s, const Q_UINT32 identifier, Direction direction, QObject *parent);
		virtual ~MsnObjectSession();

		virtual void handleInvite(const Q_UINT32 appId, const QByteArray& context);

	protected:
		virtual void onStart();
		virtual void onEnd();
		virtual void onFaulted();

	signals:
		void objectReceived(const QString& object, KTempFile *temporaryFile);

	public slots:
		void onDataReceived(const QByteArray& data, const Q_INT32 identifier, bool lastChunk);
		void onReceive(const QByteArray& bytes, const Q_INT32 identifier, const Q_INT32 relatesTo);
		void onSend(const Q_INT32 identifier);

	private:
		class MsnObjectSessionPrivate;
		MsnObjectSessionPrivate *d;

}; // MsnObjectSession
}

#endif
