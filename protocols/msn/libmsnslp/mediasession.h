/*
    mediasession.h - Peer to Peer Media Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__MEDIASESSION_H
#define CLASS_P2P__MEDIASESSION_H

#include "session.h"

namespace PeerToPeer
{

/**
 * @brief Represents a media session.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class MediaSession : public Session
{
	Q_OBJECT

	public :
		/** @brief Creates a new instance of the MediaSession class. */
		MediaSession(const Q_UINT32 identifier, Session::Direction direction, QObject *parent);
		virtual ~MediaSession();

	protected:
		virtual void onBegin();
		virtual void onEnd();

}; // MediaSession
}

#endif
