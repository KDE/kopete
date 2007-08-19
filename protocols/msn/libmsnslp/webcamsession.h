/*
    webcamsession.h - Peer to Peer Webcam Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__WEBCAMSESSION_H
#define CLASS_P2P__WEBCAMSESSION_H

#include "session.h"
#include <quuid.h>

namespace PeerToPeer
{

/**
 * @brief Represents a session used to start webcam activity.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 *
 * @example
 *
 * EUF-GUID: {1C9AA97E-9C05-4583-A3BD-908A196F1E92} or {4BD96FC0-AB17-4425-A14A-439185962DC8}
 * SessionID: 651402
 * AppID: 4
 * Context: ewBCADgAQgBFADcAMABEAEUALQBFADIAQwBBAC0ANAA0ADAAMAAtAEEARQAwADMALQA4ADgARgBGADgANQBCADkARgA0AEUAOAB9AA==
 *
 */
class WebcamSession : public Session
{
	Q_OBJECT
	Q_CLASSINFO("EUF-GUID-YOU", "1C9AA97E-9C05-4583-A3BD-908A196F1E92") // receiving
	Q_CLASSINFO("EUF-GUID-ME",  "4BD96FC0-AB17-4425-A14A-439185962DC8") // sending

	public :
		/** @brief Creates a new instance of the WebcamSession class. */
		WebcamSession(const Q_UINT32 id, DataTransferDirection direction, QObject *parent);
		virtual ~WebcamSession();
		/** @brief Gets the application id of the session. */
		const Q_UINT32 applicationId() const;
		virtual void handleInvite(const Q_UINT32 appId, const QByteArray& context);

	protected:
		virtual void onStart();
		virtual void onEnd();
		virtual void onFaulted();

	public slots:
		void onDataReceived(const QByteArray& data, bool lastChunk);
		void onReceive(const QByteArray& bytes, const Q_INT32 id, const Q_INT32 correlationId);
		void onSend(const Q_INT32 id);

	private:
		class WebcamSessionPrivate;
		WebcamSessionPrivate *d;

}; // WebcamSession
}

#endif
