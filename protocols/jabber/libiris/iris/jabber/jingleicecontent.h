/*
 * jinglecontent.cpp - Jingle content
 * Copyright (C) 2008 - Detlev Casanova <detlev.casanova@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef JINGLE_ICE_CONTENT_H
#define JINGLE_ICE_CONTENT_H

#include <QObject>

#include "im.h"
#include "jinglecontent.h"

class QHostAddress;
class QDomElement;
class QUdpSocket;

namespace XMPP
{
	/*
	 * This class contains all information about a particular content in a jingle session.
	 * It also has the socket that will be used for streaming.
	 */
	//This is the Ice-udp jingle content.
	
	class JingleSession;
	class IRIS_EXPORT JingleIceContent : public JingleContent
	{
		Q_OBJECT
	public:
		JingleIceContent(Mode mode = Unknown, JingleSession *parent = 0, Task *rootTask = 0);
		~JingleIceContent();
		
		/*
		 * Adds a candidate to this content. Doing so will add this content(s)
		 * to the transport when calling contentElement()
		 */
		virtual void addCandidate(const QDomElement&);

		/*
		 * Adds transport info (mostly a candidate). Doing so will try to
		 * connect to this candidate.
		 */
		virtual void addTransportInfo(const QDomElement&);

		/*
		 * Returns the transport type of this content.
		 */
		virtual QString transportNS() const;
		
		/*
		 * Set the parent JingleSession for this content.
		 * If you reimplement this class, you will have to call
		 * JingleContent::setSession() in your reimplementation
		 */
		virtual void setSession(JingleSession *sess);

		/*
		 * This is called to write RTP data on the established stream.
		 * TODO : what about RTCP ?
		 */
		virtual void writeDatagram(const QByteArray&);

		/* 
		 * Get all data available on the socket.
		 * Usually, this will be an RTP packet.
		 */
		virtual QByteArray readAll();

	private:
		class Private;
		Private *d;
	};
}

#endif
